.PHONY: clean

istty = 1
#mc = atmega8
mc = attiny2313

programmer = nikolaew
serialport = /dev/ttyS0

CC = avr-gcc
COMMON = -mmcu=$(mc)
CFLAGS = $(COMMON)
CFLAGS += -Wall -Os
LDFLAGS = $(COMMON)
HEX_FLASH_FLAGS = -R .eeprom
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0

TARGETS = tablo.hex write-serial-port

ifeq ($(istty),1)
	_Y:=\\033[93m
	_R:=\\033[31m
	_G:=\\033[92m
	_N:=\\033[m
endif

define green
	printf "$(_G)%s$(_N)\n" "$(1)"
endef

define buildflash
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS) $(1).elf $(1).hex
	avr-objdump -h -S $(1).elf > $(1).lss
	$(CC) $(INCLUDES) $(CFLAGS) -S $(1).c -o $(1).S
	avr-objcopy -I ihex $(1).hex -O binary $(1).bin
	@$(call green,`stat -c '%n : %s bytes' $(1).bin`)
endef

mainobjects = tablo.o ir-nec.o

all: $(TARGETS)

write-serial-port:
	gcc write-serial-port.c -o $@

tablo.hex: $(mainobjects)
	$(CC) $(LDFLAGS) $^ $(LIBDIRS) $(LIBS) -o tablo.elf
	$(call buildflash,tablo)


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	-rm -rf write-serial-port $(mainobjects) *.lss *.S *.elf dep/ *.hex *.eep *.bin

-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)


flash: tablo.hex
	stty 9600 ignbrk -brkint -icrnl -imaxbel -opost -isig -icanon -iexten -echo noflsh <$(serialport)
	avrdude -b 9660 -p $(mc) -c $(programmer) -P $(serialport) -v -U flash:w:tablo.hex:i

#avrdude -b 9660 -p attiny2313 -c usbasp -v -U flash:w:tablo.hex:i
