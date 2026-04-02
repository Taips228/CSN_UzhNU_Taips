#include "project.h" 
#include <string.h>
 
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
 
/* Значення кнопок — const, ніколи не змінюється */
static const uint8_t key_values[4][3] = { 
    {1,  2,  3}, 
    {4,  5,  6}, 
    {7,  8,  9}, 
    {10, 0, 11}, 
};

/* Стан пінів після зчитування (0 = натиснута, 1 = відпущена) */
static uint8_t key_state[4][3];

#define PASSWORD_LENGTH 3
static const uint8_t PASSWORD[PASSWORD_LENGTH] = {1, 2, 3};
static uint8_t password_buffer[PASSWORD_LENGTH];
static uint8_t password_index = 0;
static uint8_t password_locked = 1;
 
static void initMatrix() 
{ 
    for(int i = 0; i < 3; i++) 
        COLUMN_x_SetDriveMode[i](COLUMN_0_DM_DIG_HIZ); 

    memset(key_state, 1, sizeof(key_state));
} 
 
static void readMatrix() 
{ 
    for(int col = 0; col < 3; col++) 
    { 
        COLUMN_x_SetDriveMode[col](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[col](0);

        for(int row = 0; row < 4; row++) 
            key_state[row][col] = ROW_x_Read[row]();

        COLUMN_x_SetDriveMode[col](COLUMN_0_DM_DIG_HIZ); 
    } 
}

static void checkPassword()
{
    uint8_t correct = 1;
    for(int i = 0; i < PASSWORD_LENGTH; i++)
    {
        if(password_buffer[i] != PASSWORD[i])
        {
            correct = 0;
            break;
        }
    }
    
    if(correct)
    {
        SW_Tx_UART_PutString("Access allowed");
        SW_Tx_UART_PutCRLF();
        password_locked = 0;
        LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        CyDelay(1000);
    }
    else
    {
        SW_Tx_UART_PutString("Access denied");
        SW_Tx_UART_PutCRLF();
        password_locked = 1;
    }
    
    password_index = 0;
    memset(password_buffer, 0, PASSWORD_LENGTH);
}

static void setLEDColor(uint8_t v)
{
    switch(v)
    {
        case 1: case 7:  LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); break;
        case 2: case 8:  LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); break;
        case 3: case 9:  LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); break;
        case 4: case 10: LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); break;
        case 5: case 0:  LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); break;
        case 6: case 11: LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); break;
        default:         LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); break;
    }
}

static void printButtonName(uint8_t v)
{
    char buf[20];
    if(v == 10)      SW_Tx_UART_PutString("Button * pressed");
    else if(v == 11) SW_Tx_UART_PutString("Button # pressed");
    else
    {
        SW_Tx_UART_PutString("Button ");
        buf[0] = '0' + v;
        buf[1] = '\0';
        SW_Tx_UART_PutString(buf);
        SW_Tx_UART_PutString(" pressed");
    }
    SW_Tx_UART_PutCRLF();
}
  
int main(void) 
{ 
    CyGlobalIntEnable;
  
    SW_Tx_UART_Start(); 
    SW_Tx_UART_PutCRLF(); 
    SW_Tx_UART_PutString("Software Transmit UART"); 
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Enter password to unlock");
    SW_Tx_UART_PutCRLF();
 
    initMatrix();
    
    uint8_t last_state    = 255;
    uint8_t button_pressed = 0;
    
    LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); /* Білий = заблоковано */
    
    for(;;) 
    {      
        readMatrix();
        button_pressed = 0;
        
        for(int row = 0; row < 4; row++)
        {
            for(int col = 0; col < 3; col++)
            {
                if(key_state[row][col] == 0) /* Кнопка натиснута */
                {
                    uint8_t button_value = key_values[row][col]; /* ← головне виправлення */
                    
                    if(last_state != button_value)
                    {
                        last_state = button_value;
                        printButtonName(button_value);
                        
                        if(password_locked)
                        {
                            password_buffer[password_index++] = button_value;
                            if(password_index >= PASSWORD_LENGTH)
                                checkPassword();
                        }
                        else
                        {
                            setLEDColor(button_value);
                        }
                    }
                    button_pressed = 1;
                    break;
                }
            }
            if(button_pressed) break;
        }
        
        if(!button_pressed && last_state != 255)
        {
            last_state = 255;
            if(password_locked)
            {
                LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); /* Білий */
            }
            else
            {
                LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); /* Вимкнено */
            }
        }
        
        CyDelay(50); 
    }
}
