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
    CyGlobalIntEnable;

    SW_Tx_UART_Start();
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Software Transmit UART");
    SW_Tx_UART_PutCRLF();
    SW_Tx_UART_PutString("Taps Dmytro");
    SW_Tx_UART_PutCRLF();

    for(;;)
    {
        if (Button_Read() == 0) // кнопка натиснута
        {
            SW_Tx_UART_PutString("CYAN / Button pressed");
            SW_Tx_UART_PutCRLF();

            setColor(1,0,0); // Cyan
        }
        else
        {
            SW_Tx_UART_PutString("RED / Button released");
            SW_Tx_UART_PutCRLF();

            setColor(0,1,1); // Red
        }

        CyDelay(500);
    }
}