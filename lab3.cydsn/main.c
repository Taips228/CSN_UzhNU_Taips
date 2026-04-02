/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"

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

/* [ROW][COLUMN] */
static uint8_t keys[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 0, 11},
};

/* matrix initialization function */
static void initMatrix()
{
    for(int column_index = 0; column_index < 3; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
    }
}

/* keys matrix read function */
static void readMatrix()
{
    /* define the length of a row and column */
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]);
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);
    
    /* column: iterate the columns */
    for(int column_index = 0; column_index < column_counter; column_index++)
    {
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG);
        COLUMN_x_Write[column_index](0);
        
        /* row: interate throught the rows */
        for(int row_index = 0; row_index < row_counter; row_index++)
        {
            keys[row_index][column_index] = ROW_x_Read[row_index]();
        }
        
        /* disable the column */
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ);
    }
}

/* matrix print function */
void printMatrix()
{
    SW_Tx_UART_PutCRLF();
    for (int row_index = 0; row_index < 4; row_index++)
    {
        for(int column_index = 0; column_index < 3; column_index++)
        {
            SW_Tx_UART_PutHexInt(keys[row_index][column_index]);
            SW_Tx_UART_PutString(" ");
        }
        SW_Tx_UART_PutCRLF();
    }
    SW_Tx_UART_PutCRLF();
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Software Transmit UART");
    SW_Tx_UART_PutCRLF();
    
    initMatrix();
    uint8_t last_state = 99;
    
    for(;;)
    {
        /* Button == 0 -> Button pressed
            Button == 1 -> Button released
            LED_x_Write(0); -> Turn ON
            LED_x_Write(1); -> Turn OFF
        */
        readMatrix();
        
        /* ЗАВДАННЯ 9: Виводимо стан матриці у термінал */
        printMatrix();
        
        /* невелика затримка, щоб зручно було читати дані у терміналі */
        CyDelay(200);
        
        /* ------------------ КНОПКА 1 ------------------ */
        if(keys[0][0] == 0 && last_state != 1)
        {
            last_state = 1;
            SW_Tx_UART_PutString("Button 1 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[0][0] == 1 && last_state == 1)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 1 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }
        
        /* ------------------ КНОПКА 2 ------------------ */
        if(keys[0][1] == 0 && last_state != 2)
        {
            last_state = 2;
            SW_Tx_UART_PutString("Button 2 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[0][1] == 1 && last_state == 2)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 2 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 3 ------------------ */
        if(keys[0][2] == 0 && last_state != 3)
        {
            last_state = 3;
            SW_Tx_UART_PutString("Button 3 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[0][2] == 1 && last_state == 3)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 3 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 4 ------------------ */
        if(keys[1][0] == 0 && last_state != 4)
        {
            last_state = 4;
            SW_Tx_UART_PutString("Button 4 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[1][0] == 1 && last_state == 4)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 4 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 5 ------------------ */
        if(keys[1][1] == 0 && last_state != 5)
        {
            last_state = 5;
            SW_Tx_UART_PutString("Button 5 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[1][1] == 1 && last_state == 5)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 5 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 6 ------------------ */
        if(keys[1][2] == 0 && last_state != 6)
        {
            last_state = 6;
            SW_Tx_UART_PutString("Button 6 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[1][2] == 1 && last_state == 6)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 6 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 7 ------------------ */
        if(keys[2][0] == 0 && last_state != 7)
        {
            last_state = 7;
            SW_Tx_UART_PutString("Button 7 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[2][0] == 1 && last_state == 7)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 7 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 8 ------------------ */
        if(keys[2][1] == 0 && last_state != 8)
        {
            last_state = 8;
            SW_Tx_UART_PutString("Button 8 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[2][1] == 1 && last_state == 8)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 8 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 9 ------------------ */
        if(keys[2][2] == 0 && last_state != 9)
        {
            last_state = 9;
            SW_Tx_UART_PutString("Button 9 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[2][2] == 1 && last_state == 9)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 9 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА * ------------------ */
        if(keys[3][0] == 0 && last_state != 10)
        {
            last_state = 10;
            SW_Tx_UART_PutString("Button * pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[3][0] == 1 && last_state == 10)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button * released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА 0 ------------------ */
        if(keys[3][1] == 0 && last_state != 0)
        {
            last_state = 0;
            SW_Tx_UART_PutString("Button 0 pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[3][1] == 1 && last_state == 0)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button 0 released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }

        /* ------------------ КНОПКА # ------------------ */
        if(keys[3][2] == 0 && last_state != 11)
        {
            last_state = 11;
            SW_Tx_UART_PutString("Button # pressed");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(0); LED_G_Write(0); LED_B_Write(0);
        }
        else if (keys[3][2] == 1 && last_state == 11)
        {
            last_state = 99;
            SW_Tx_UART_PutString("Button # released");
            SW_Tx_UART_PutCRLF();
            LED_R_Write(1); LED_G_Write(1); LED_B_Write(1);
        }
    }
}


/* [] END OF FILE */
