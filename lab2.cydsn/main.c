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
 /* Initialization */
    
void setColor(uint8 r, uint8 g, uint8 b)
{
    LED_R_Write(r);
    LED_G_Write(g);
    LED_B_Write(b);
}

int main(void)
{
    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Software Transmit UART");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Taips Dmytro");
    SW_Tx_UART_PutCRLF();
    CyGlobalIntEnable;

    for(;;)
    {
        if (Button_Read() == 0)  // кнопка натиснута
        {
            SW_Tx_UART_PutString("BLUE/Button pressed");
            SW_Tx_UART_PutCRLF();
            // Blue
            setColor(1, 1, 0);
        }
        else
        {
            SW_Tx_UART_PutString("YELLOW/Button released");
            SW_Tx_UART_PutCRLF();
            // Yellow
            setColor(0, 0, 1);
        }

        CyDelay(500);
        setColor(1, 1, 1); // вимкнути (Black)
        CyDelay(500);
    }
}



/* [] END OF FILE */
