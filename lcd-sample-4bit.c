#include <avr/io.h>
#define F_CPU 12000000UL
#include <util/delay.h>
#include "wh1602l-ygk-ct.h"

/*
 * PD2(4) ---> RS (4)
 * PD3(5) ---> EN (6)
 * PD4(6) ---> DB4(11)
 * PD5(11)---> DB5(12)
 * PD6(12)---> DB7(13)
 * PD7(13)---> DB7(14)
 */


#define en_bit PD3
#define en_port PORTD
#define en_dir DDRD

#define rs_bit PD2
#define rs_port PORTD
#define rs_dir DDRD

#define data_port PORTD
#define data_dir DDRD

#define data_bits (_BV(PD4)|_BV(PD5)|_BV(PD6)|_BV(PD7))

#define clear_bit(reg, bit) reg &= (~(_BV(bit)))
#define set_bit(reg, bit)   reg |= (_BV(bit))

void sleep1ms() {
    _delay_ms(1);
}

void sleep20ms() {
    _delay_ms(20);
}

void sleep1s() {
    _delay_ms(1000);
}

void sleep2s() {
    _delay_ms(2000);
}

void en_strobe() {
    set_bit(en_port, en_bit);
    sleep1ms();
    clear_bit(en_port, en_bit);
    sleep1ms();
}

void lcd_upper_4bit_command(unsigned char command) {
    clear_bit(rs_port, rs_bit);
    data_port = (command & 0xf0) | (data_port & 0x0f);
    en_strobe();
}

void lcd_command(unsigned char command) {
    clear_bit(rs_port, rs_bit);
    data_port = (command & 0xf0) | (data_port & 0x0f);
    en_strobe();
    data_port = ((command<<4) & 0xf0) | (data_port & 0x0f);
    en_strobe();
}

void lcd_data(char byte) {
    set_bit(rs_port, rs_bit);
    data_port = (byte & 0xf0) | (data_port & 0x0f);
    en_strobe();
    data_port = ((byte<<4) & 0xf0) | (data_port & 0x0f);
    en_strobe();
}

void lcd_init() {
    sleep20ms();
    lcd_upper_4bit_command(lcd_funcset_8bit_2lines_5x8dots);
    sleep20ms();
    lcd_upper_4bit_command(lcd_funcset_8bit_2lines_5x8dots);
    sleep20ms();
    lcd_upper_4bit_command(lcd_funcset_8bit_2lines_5x8dots);
    sleep20ms();
    // switch to 4bit mode
    lcd_upper_4bit_command(lcd_funcset_4bit_2lines_5x8dots);
    sleep20ms();
    // execute commands in 4bit mode
    lcd_command(lcd_funcset_4bit_2lines_5x8dots);
    sleep20ms();
    lcd_command(lcd_display_off);
    sleep20ms();
    lcd_command(lcd_display_clear);
    sleep20ms();
    lcd_command(lcd_entry_mode_cursor_right);
    sleep20ms();
    lcd_command(lcd_display_on);
    sleep20ms();
}

int main(void) {
    data_dir |= (_BV(en_bit) | _BV(rs_bit) | data_bits) ;

    lcd_init();

    char one[16] = {
        cyr_v,
        cyr_a,
        cyr_s,
        space,
        cyr_p,
        cyr_r,
        cyr_i,
        cyr_v,
        cyr_e,
        cyr_t,
        cyr_s,
        cyr_t,
        cyr_v,
        cyr_u,
        cyr_e,
        cyr_t};
        
    char two[16] = {
        space,
        space,
        cyr_a,
        cyr_v,
        cyr_t,
        cyr_o,
        cyr_o,
        cyr_t,
        cyr_v,
        cyr_e,
        cyr_t,
        cyr_ch,
        cyr_i,
        cyr_k,
        space,
        space
        };
    while(1) {
        lcd_command(lcd_goto_upper_line);
        for (unsigned char i=0; i<16;i++) {
            lcd_data(one[i]);
        }
        sleep1s();
        lcd_command(lcd_goto_lower_line);
        for (unsigned char i=0; i<16;i++) {
            lcd_data(two[i]);
        }
        sleep2s();
        lcd_command(lcd_display_clear);
        sleep1s();
    }

    return 0;
}
