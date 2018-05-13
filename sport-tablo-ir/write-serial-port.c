#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include "sport-tablo.h"

struct Message {
    uint8_t start1;
    uint8_t start2;
    uint8_t cmd;
    uint8_t del1;
    uint8_t h1;
    uint8_t h2;
    uint8_t del2;
    uint8_t g1;
    uint8_t g2;
    uint8_t del3;
    uint8_t per;
};
    

struct Message message;

void update_2_digit_bcd(uint8_t value, uint8_t* hi, uint8_t* lo) {
    if (value > 99) {
        return;
    }
    *lo = value % 10;
    *hi = (value - *lo) / 10;
}

void parse_two_digits(char * opt, uint8_t *f, uint8_t *s) {
    int i;
    printf ("option `%s'\n", opt);
    int digits = sscanf(opt, "%ud", &i);
    if (1 == digits) {
        update_2_digit_bcd(i, s, f);
    }
}

void parse_one_digit(char * opt, uint8_t *f) {
    int i;
    printf ("option `%s'\n", opt);
    int digits = sscanf(opt, "%ud", &i);
    if (1 != digits && i < 9) {
        *f = i;
    }
}


int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}


int main(int argc, char **argv) {
    char *portname = "/dev/ttyS0";
    int fd;
    int wlen;
    int i;

    char * p = (uint8_t*)&message;

    int c;
    uint8_t g_first = 0x30;
    uint8_t g_second = 0x30;
    uint8_t h_first = 0x30;
    uint8_t h_second = 0x30;
    uint8_t period = 0x01;

    uint8_t command = COMMAND_BAD;

    int digits;
    int dryrun = 0;
    
    while (1) {
        static struct option long_options[] = {
          {"dryrun",  no_argument,       0, 'd'},
          {"start",   no_argument,       0, 's'},
          {"stop",    no_argument,       0, 'x'},
          {"clock",   no_argument,       0, 'c'},
          {"resume",  no_argument,       0, 'r'},
          {"hosts",   required_argument, 0, 'h'},
          {"period",  required_argument, 0, 'p'},
          {"guests",  required_argument, 0, 'g'},
          {0, 0, 0, 0}
    };

    int option_index = 0;
    c = getopt_long (argc, argv, "dsxcrg:h:p:",
                   long_options, &option_index);
    if (c == -1)
        break;
    switch (c) {
        case 'd':
            dryrun = 1;
            portname = "out.bin";
            break;
        case 'g':
            parse_two_digits(optarg, &g_first, &g_second);
            command = COMMAND_SCORE;
            break;
        case 'h':
            parse_two_digits(optarg, &h_first, &h_second);
            command = COMMAND_SCORE;
            break;
        case 'p':
            parse_one_digit(optarg, &period);
            if (COMMAND_BAD == command) {
                command = COMMAND_PERIOD;
            }
            break;
        case 's':
            command = COMMAND_TIMER_START_FORWARD;
            break;
        case 'x':
            command = COMMAND_TIMER_STOP;
            break;
        case 'c':
            command = COMMAND_TIMER_CLOCK;
            break;
        default:
          abort ();
        }
    }

    fd = open(portname, (dryrun) ? (O_RDWR|O_CREAT|O_SYNC):(O_RDWR|O_NOCTTY|O_SYNC));
   	// S_IRWXU | S_IRWXG |  S_IRWXO
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    if (!dryrun) { 
        /*baudrate 2400 !!!, 8 bits, no parity, 1 stop bit */
        set_interface_attribs(fd, B2400);
        //set_mincount(fd, 0);                /* set to pure timed read */
    }

    int size = 9;
    message.start1 = START_BYTE_1;
    message.start2 = START_BYTE_2;
    message.cmd = command;
    message.del1 = 0;
    message.h1 = 0;
    message.h2 = 0;
    message.del2 = 0;
    message.g1 = 0;
    message.g2 = 0;
    message.del3 = 0;
    message.per = 0;

    switch (command) {
        case COMMAND_SCORE:
            message.g1 = (uint8_t)g_first;
            message.g2 = (uint8_t)g_second;
            message.h1 = (uint8_t)h_first;
            message.h2 = (uint8_t)h_second;
            message.del1 = 0x20;
            message.del2 = 0x20;
            message.per = (uint8_t)period;
            size = 11;
            break;
        case COMMAND_PERIOD:
            message.g2 = period;
            break;
        case COMMAND_TIMER_START_FORWARD:
        case COMMAND_TIMER_CLOCK:
        case COMMAND_TIMER_STOP:
            break;
        default:
            abort();
    }
   
    for (i=0; i<size; i++) {
        wlen = write(fd, p, 1);
        if (wlen != 1) {
            printf("Error from write: %d, %d\n", wlen, errno);
        }
        usleep(10);
        p++;
    }
    if (!dryrun) { 
        tcdrain(fd); /* delay for output */
    }

}
