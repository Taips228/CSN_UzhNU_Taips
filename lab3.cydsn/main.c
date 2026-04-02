#include "project.h" 
#include <string.h>
 
/* arrays of pointers */ 
/* function of drive mode configuration */ 
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = { 
    COLUMN_0_SetDriveMode, 
    COLUMN_1_SetDriveMode, 
    COLUMN_2_SetDriveMode 
};   
/* column write function */     
static void (*COLUMN_x_Write[3])(uint8_t value) = { 
    COLUMN_0_Write, 
    COLUMN_1_Write, 
    COLUMN_2_Write 
}; 
/* read row function */    
static uint8 (*ROW_x_Read[4])() = { 
    ROW_0_Read, 
    ROW_1_Read, 
    ROW_2_Read, 
    ROW_3_Read 
}; 
 
/* [ROW][COLUMN] — значення кнопок (не змінюється) */
static const uint8_t key_values[4][3] = { 
    {1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9}, 
    {10, 0, 11}, 
};

/* [ROW][COLUMN] — стан пінів після зчитування (0 = натиснута, 1 = відпущена) */
static uint8_t key_state[4][3];

/* Password settings */
#define PASSWORD_LENGTH 3
static const uint8_t PASSWORD[PASSWORD_LENGTH] = {1, 2, 3};
static uint8_t password_buffer[PASSWORD_LENGTH];
static uint8_t password_index = 0;
static uint8_t password_locked = 1; /* 1 = заблоковано, 0 = розблоковано */
 
/* matrix initialization function */ 
static void initMatrix() 
{ 
    for(int column_index = 0; column_index < 3; column_index++) 
    { 
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    }

    /* Очищення масиву стану */
    memset(key_state, 1, sizeof(key_state));
} 
 
/* keys matrix read function — пише в key_state, не чіпає key_values */ 
static void readMatrix() 
{ 
    uint8_t row_counter    = sizeof(ROW_x_Read)    / sizeof(ROW_x_Read[0]); 
    uint8_t column_counter = sizeof(COLUMN_x_Write) / sizeof(COLUMN_x_Write[0]);     

    for(int column_index = 0; column_index < column_counter; column_index++) 
    { 
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[column_index](0);

        for(int row_index = 0; row_index < row_counter; row_index++) 
        { 
            key_state[row_index][column_index] = ROW_x_Read[row_index](); 
        } 

        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    } 
}

/* Function to check password */
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
        /* Білий світлодіод при розблокуванні */
        LED_R_Write(0);
        LED_G_Write(0);
        LED_B_Write(0);
        CyDelay(1000);
    }
    else
    {
        SW_Tx_UART_PutString("Access denied");
        SW_Tx_UART_PutCRLF();
        password_locked = 1;
    }
    
    /* Скидання буфера і лічильника */
    password_index = 0;
    memset(password_buffer, 0, PASSWORD_LENGTH);
}

/* Function to set LED color based on button */
static void setLEDColor(uint8_t button_value)
{
    switch(button_value)
    {
        case 1:  /* Red */
        case 7:
            LED_R_Write(0);
            LED_G_Write(1);
            LED_B_Write(1);
            break;
            
        case 2:  /* Green */
        case 8:
            LED_R_Write(1);
            LED_G_Write(0);
            LED_B_Write(1);
            break;
            
        case 3:  /* Blue */
        case 9:
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(0);
            break;
            
        case 4:  /* Yellow (Red + Green) */
        case 10: /* * */
            LED_R_Write(0);
            LED_G_Write(0);
            LED_B_Write(1);
            break;
            
        case 5:  /* Purple (Red + Blue) */
        case 0:
            LED_R_Write(0);
            LED_G_Write(1);
            LED_B_Write(0);
            break;
            
        case 6:  /* Cyan (Green + Blue) */
        case 11: /* # */
            LED_R_Write(1);
            LED_G_Write(0);
            LED_B_Write(0);
            break;
            
        default:
            LED_R_Write(1);
            LED_G_Write(1);
            LED_B_Write(1);
            break;
    }
}

/* Function to print button name */
static void printButtonName(uint8_t button_value)
{
    switch(button_value)
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
    
    uint8_t last_state    = 255; /* 255 = жодна кнопка не натиснута */
    uint8_t button_pressed = 0;
    
    /* Початковий стан — білий світлодіод (система заблокована) */
    LED_R_Write(0);
    LED_G_Write(0);
    LED_B_Write(0);
    
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
                    /* Беремо значення з незмінного масиву key_values */
                    uint8_t button_value = key_values[row][col];
                    
                    if(last_state != button_value)
                    {
                        last_state = button_value;
                        printButtonName(button_value);
                        
                        if(password_locked)
                        {
                            password_buffer[password_index] = button_value;
                            password_index++;
                            
                            if(password_index >= PASSWORD_LENGTH)
                            {
                                checkPassword();
                            }
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
        
        /* Якщо жодна кнопка не натиснута */
        if(!button_pressed)
        {
            if(last_state != 255)
            {
                last_state = 255;
                
                if(password_locked)
                {
                    /* Білий — система заблокована */
                    LED_R_Write(0);
                    LED_G_Write(0);
                    LED_B_Write(0);
                }
                else
                {
                    /* Вимкнути — система розблокована, кнопка відпущена */
                    LED_R_Write(1);
                    LED_G_Write(1);
                    LED_B_Write(1);
                }
            }
        }
        
        CyDelay(50); 
    }
}
