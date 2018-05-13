/*
       a   
    +-----+
   f|     |b
    |     |
    +     +
   e|     |c
    |     |
    +-----+
       d
*/

uint8_t snake[11] PROGMEM = {
   /* hgfedcba   */
    0b00000001,
    0b00000011,
    0b00000010,
    0b00000110,
    0b00000100,
    0b00001100,
    0b00001000,
    0b00011000,
    0b00010000,
    0b00110000,
    0b00100001
};

uint8_t snake_position;

void update_snake() {
    if (snake_position++ >= 11) {
        snake_position = 0;
    }
    snake_port = snake[snake_position];
}

