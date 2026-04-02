#include "project.h" 
#include <string.h>
 
/* arrays of pointers */ 
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
 
/* [ROW][COLUMN] — значення кнопок (НЕ змінюється) */
static const uint8_t key_values[4][3] = { 
    {1,  2,  3}, 
    {4,  5,  6}, 
    {7,  8,  9}, 
    {10, 0, 11}, 
};

/* [ROW][COLUMN] — сирий стан пінів (0=натиснута, 1=відпущена) */
static uint8_t key_state[4][3];

/* Password settings */
#define PASSWORD_LENGTH 3
static const uint8_t PASSWORD[PASSWORD_LENGTH] = {1, 2, 3};
static uint8_t password_buffer[PASSWORD_LENGTH];
static uint8_t password_index  = 0;
static uint8_t password_locked = 1;
 
/* matrix initialization */
static void initMatrix() 
{ 
    for(int i = 0; i < 3; i++) 
        COLUMN_x_SetDriveMode[i](COLUMN_0_DM_DIG_HIZ); 

    memset(key_state, 1, sizeof(key_state));
} 
 
/* read matrix — пише в key_state, key_values не чіпає */
static void readMatrix() 
{ 
    uint8_t rows = sizeof(ROW_x_Read)    / sizeof(ROW_x_Read[0]); 
    uint8_t cols = sizeof(COLUMN_x_Write) / sizeof(COLUMN_x_Write[0]);     

    for(int c = 0; c < cols; c++) 
    { 
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[c](0);
        for(int r = 0; r < rows; r++) 
            key_state[r][c] = ROW_x_Read[r](); 
        COLUMN_x_SetDriveMode[c](COLUMN_0_DM_DIG_HIZ); 
    } 
}

/* check entered password */
static void checkPassword()
{
    uint8_t correct = 1;
    for(int i = 0; i < PASSWORD_LENGTH; i++)
        if(password_buffer[i] != PASSWORD[i]) { correct = 0; break; }
    
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

/* set LED color */
static void setLEDColor(uint8_t v)
{
    switch(v)
    {
        case 1: case 7:                          // Red
            LED_R_Write(0); LED_G_Write(1); LED_B_Write(1); break;
        case 2: case 8:                          // Green
            LED_R_Write(1); LED_G_Write(0); LED_B_Write(1); break;
        case 3: case 9:                          // Blue
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(0); break;
        case 4: case 10:                         // Yellow
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(1); break;
        case 5: case 0:                          // Purple
            LED_R_Write(0); LED_G_Write(1); LED_B_Write(0); break;
        case 6: case 11:                         // Cyan
            LED_R_Write(1); LED_G_Write(0); LED_B_Write(0); break;
        default:
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); break;
    }
}

/* print button name to UART */
static void printButtonName(uint8_t v)
{
    switch(v)
    {
        case 0:  SW_Tx_UART_PutString("Button 0 pressed"); break;
        case 1:  SW_Tx_UART_PutString("Button 1 pressed"); break;
        case 2:  SW_Tx_UART_PutString("Button 2 pressed"); break;
        case 3:  SW_Tx_UART_PutString("Button 3 pressed"); break;
        case 4:  SW_Tx_UART_PutString("Button 4 pressed"); break;
        case 5:  SW_Tx_UART_PutString("Button 5 pressed"); break;
        case 6:  SW_Tx_UART_PutString("Button 6 pressed"); break;
        case 7:  SW_Tx_UART_PutString("Button 7 pressed"); break;
        case 8:  SW_Tx_UART_PutString("Button 8 pressed"); break;
        case 9:  SW_Tx_UART_PutString("Button 9 pressed"); break;
        case 10: SW_Tx_UART_PutString("Button * pressed"); break;
        case 11: SW_Tx_UART_PutString("Button # pressed"); break;
        default: break;
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
    
    uint8_t last_state     = 255;
    uint8_t button_pressed = 0;
    
    /* Початково — білий (заблоковано) */
    LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
    
    for(;;) 
    {      
        readMatrix();
        button_pressed = 0;
        
        for(int row = 0; row < 4; row++)
        {
            for(int col = 0; col < 3; col++)
            {
                if(key_state[row][col] == 0) /* натиснута */
                {
                    uint8_t button_value = key_values[row][col]; /* беремо з незмінного масиву */
                    
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
                LED_R_Write(0); LED_G_Write(0); LED_B_Write(0); /* білий */
            }
            else
            {
                LED_R_Write(1); LED_G_Write(1); LED_B_Write(1); /* вимкнено */
            }
        }
        
        CyDelay(50); 
    }
}
