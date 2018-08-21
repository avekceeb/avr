#include "wh1602l-ygk-ct.h"

void wh1602_print(char *buffer, char line)
{
    unsigned char i;
    if (0 == buffer) {
        // clearing both lines
        lcd_command(lcd_display_clear);
        return;
    }
    lcd_command(line);
    for (i=0;i<16;i++) {
        if (0 == buffer[i]) {
            return;
        }
        lcd_data(buffer[i]);
    }
}

