
mc = atmega8 
program = voltmeter
TARGET = $(program).elf
SOURCES = $(program).c
CC = avr-gcc

COMMON = -mmcu=$(mc)

CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -O0
CFLAGS += -Wp,-M,-MP,-MT,$(*F).o,-MF,dep/$(@F).d 

ASMFLAGS = $(COMMON)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

LDFLAGS = $(COMMON)
LDFLAGS += 

HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0

OBJECTS = $(program).o lcd.o wh1602l-ygk-ct.o

all: checkname $(TARGET) $(program).hex $(program).eep $(program).lss

checkname:
ifndef program
	$(error 'program' is not set. Use 'make program=<name of program>')
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#$(OBJECTS): $(SOURCES) 
#	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@
	$(CC) $(INCLUDES) $(CFLAGS) -S $(SOURCES) -o$(program).S

.PHONY: clean
clean:
	-rm -rf *.o *.lss *.S *.elf dep/ *.hex *.eep

-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

flash: $(program).hex
	stty 9600 ignbrk -brkint -icrnl -imaxbel -opost -isig -icanon -iexten -echo noflsh </dev/ttyS0
	avrdude -b 9660 -p $(MCU) -c nikolaew -P /dev/ttyS0 -v -U flash:w:$(program).hex:i

