#include "project.h"

int main(void)
{
    CyGlobalIntEnable;

    for(;;)
    {
        if(Button_Read() == 0)   // кнопка натиснута (часто active LOW)
        {
            LED_R_Write(1);     // увімкнути LED
        }
        else
        {
            LED_R_Write(0);     // вимкнути LED
        }
    }
}