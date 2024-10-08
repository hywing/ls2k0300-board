#include <stdio.h>
#include "lcd.h"
#include "lcd_init.h"

int main()
{
    int j = 0;

    LCD_Init();

    LCD_Fill(0, 0, 240, 240, BLACK);

    while (1)
    {
        for(int i = 0; i < 240; i += 10) {
            Draw_Circle(120, 120, i, 400 * j);
        }

        if(j++ > 65536 / 400) {
            j = 0;
        }

        usleep(1000000);
    }

    closeLCD();
    return 0;
}