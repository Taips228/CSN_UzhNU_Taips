#include "project.h" 
#include <string.h>
 
/* Arrays of function pointers */
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = { 
    COLUMN_0_SetDriveMode, 
    COLUMN_1_SetDriveMode, 
    COLUMN_2_SetDriveMode 
};
static void (*COLUMN_x_Write[3])(uint8_t value) = { 
    COLUMN_0_Write, 
    COLUMN_1_Write, 
    COLUMN_2_Write 
};
static uint8 (*ROW_x_Read[4])() = { 
    ROW_0_Read, 
    ROW_1_Read, 
    ROW_2_Read, 
    ROW_3_Read 
};

/* Key values — never overwritten */
static const uint8_t key_values[4][3] = { 
    {1,  2,  3 }, 
    {4,  5,  6 }, 
    {7,  8,  9 }, 
    {10, 0,  11}, /* 10 = *, 11 = # */
};

/* Raw pin state after readMatrix() */
static uint8_t key_state[4][3];

/* Password settings */
#define PASSWORD_LENGTH 3
static const uint8_t PASSWORD[PASSWORD_LENGTH] = {1, 2, 3};
static uint8_t password_buffer[PASSWORD_LENGTH];
static uint8_t password_index  = 0;
static uint8_t password_locked = 1;  /* 1 = locked, 0 = unlocked */
static uint8_t entry_mode      = 0;  /* 1 = collecting digits */

/* ---------------------------------------------------------- */
static void initMatrix(void)
{ 
    for(int c = 0; c < 3; c++)
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ);
    memset(key_state, 1, sizeof(key_state));
}

static void readMatrix(void)
{ 
    for(int c = 0; c < 3; c++)
    { 
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[c](0);
        for(int r = 0; r < 4; r++)
            key_state[r][c] = ROW_x_Read[r]();
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ);
    }
}

/* ---------------------------------------------------------- */
static void ledWhite(void) { LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); }
static void ledOff(void)   { LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); }

static void setLEDColor(uint8_t v)
{
    switch(v)
    {
        case 1: case 7:  LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); break; /* Red    */
        case 2: case 8:  LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); break; /* Green  */
        case 3: case 9:  LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); break; /* Blue   */
        case 4: case 10: LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); break; /* Yellow */
        case 5: case 0:  LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); break; /* Purple */
        case 6: case 11: LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); break; /* Cyan   */
        default: ledOff(); break;
    }
}

static void printButtonName(uint8_t v)
{
    const char *names[] = {"0","1","2","3","4","5","6","7","8","9","*","#"};
    SW_Tx_UART_PutString("Button ");
    if(v <= 11) SW_Tx_UART_PutString(names[v]);
    SW_Tx_UART_PutString(" pressed");
    SW_Tx_UART_PutCRLF();
}

/* ---------------------------------------------------------- */
static void resetPasswordEntry(void)
{
    password_index = 0;
    entry_mode     = 0;
    memset(password_buffer, 0, PASSWORD_LENGTH);
}

static void checkPassword(void)
{
    uint8_t ok = 1;
    for(int i = 0; i < PASSWORD_LENGTH; i++)
        if(password_buffer[i] != PASSWORD[i]) { ok = 0; break; }

    if(ok)
    {
        SW_Tx_UART_PutString("Access allowed");
        SW_Tx_UART_PutCRLF();
        password_locked = 0;
        ledWhite();
        CyDelay(500);
    }
    else
    {
        SW_Tx_UART_PutString("Access denied");
        SW_Tx_UART_PutCRLF();
        SW_Tx_UART_PutString("Press * to try again");
        SW_Tx_UART_PutCRLF();
        password_locked = 1;
        /* Flash red */
        LED_R_Write(0); LED_G_Write(1); LED_B_Write(1);
        CyDelay(500);
        ledWhite();
    }

    resetPasswordEntry();
}

/* ---------------------------------------------------------- */
int main(void) 
{ 
    CyGlobalIntEnable;
  
    SW_Tx_UART_Start(); 
    SW_Tx_UART_PutCRLF(); 
    SW_Tx_UART_PutString("Software Transmit UART"); 
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Press * to enter password");
    SW_Tx_UART_PutCRLF();

    initMatrix();

    uint8_t last_state     = 255;
    uint8_t button_pressed = 0;

    ledWhite(); /* Locked = white LED */
    
    for(;;) 
    {      
        readMatrix();
        button_pressed = 0;
        
        for(int row = 0; row < 4; row++)
        {
            for(int col = 0; col < 3; col++)
            {
                if(key_state[row][col] == 0) /* Button pressed (active low) */
                {
                    uint8_t v = key_values[row][col];
                    
                    if(last_state != v) /* New press only, ignore held button */
                    {
                        last_state = v;
                        printButtonName(v);

                        /* ========== LOCKED ========== */
                        if(password_locked)
                        {
                            if(v == 10) /* * = begin password entry */
                            {
                                resetPasswordEntry();
                                entry_mode = 1;
                                SW_Tx_UART_PutString("Enter 3 digits, then #");
                                SW_Tx_UART_PutCRLF();
                            }
                            else if(entry_mode)
                            {
                                if(v == 11) /* # = confirm */
                                {
                                    if(password_index == PASSWORD_LENGTH)
                                        checkPassword();
                                    else
                                    {
                                        SW_Tx_UART_PutString("Need 3 digits first");
                                        SW_Tx_UART_PutCRLF();
                                        resetPasswordEntry();
                                    }
                                }
                                else if(password_index < PASSWORD_LENGTH)
                                {
                                    password_buffer[password_index] = v;
                                    password_index++;
                                    /* Show progress: 1/3, 2/3, 3/3 */
                                    char buf[2] = {'0' + password_index, '\0'};
                                    SW_Tx_UART_PutString("Digit ");
                                    SW_Tx_UART_PutString(buf);
                                    SW_Tx_UART_PutString("/3 accepted");
                                    SW_Tx_UART_PutCRLF();
                                }
                            }
                            /* If not entry_mode and not *, ignore digit presses */
                        }
                        /* ========== UNLOCKED ========== */
                        else
                        {
                            if(v == 10) /* * = lock again */
                            {
                                password_locked = 1;
                                resetPasswordEntry();
                                SW_Tx_UART_PutString("Locked. Press * to unlock.");
                                SW_Tx_UART_PutCRLF();
                                ledWhite();
                            }
                            else
                            {
                                setLEDColor(v);
                            }
                        }
                    }
                    button_pressed = 1;
                    break;
                }
            }
            if(button_pressed) break;
        }
        
        /* No button pressed */
        if(!button_pressed && last_state != 255)
        {
            last_state = 255;
            if(!password_locked)
                ledOff();
        }
        
        CyDelay(50); 
    }
}
