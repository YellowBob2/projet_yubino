# MCU and clock (override on command line, e.g. make MCU=atmega328p F_CPU=16000000UL)
MCU ?= atmega328p
F_CPU ?= 16000000UL

CC := avr-gcc
OBJCOPY := avr-objcopy
SIZE := avr-size
AVRDUDE := avrdude

# Warnings demandés
WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings \
            -Wnested-externs -Winline -Wno-long-long \
            -Wstrict-prototypes -Wno-array-bounds \
            -Wmissing-prototypes -Wredundant-decls -Wmissing-declarations
						#-Wno-unused-variable -Wno-unused-function -Wno-sign-compare -Wno-unused-parameter -Wno-redundant-decls -Wno-missing-prototypes

CFLAGS  := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -std=gnu11 $(WARNINGS) \
           -ffunction-sections -fdata-sections -I. -Imicro-ecc
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

# Sources: inclure micro-ecc/uECC.c pour générer micro-ecc/uECC.o
SRCS := main.c rng.c storage.c uart.c ring_buffer.c ui.c micro-ecc/uECC.c
OBJS := $(SRCS:.c=.o)

TARGET := authenticator
ELF    := $(TARGET).elf
HEX    := $(TARGET).hex

# avrdude settings (override if needed)
AVRDUDE_PROGRAMMER ?= usbasp
AVRDUDE_PORT ?= /dev/ttyUSB0
AVRDUDE_BAUD ?= 115200

.PHONY: all clean flash size

all: $(HEX) size

# Link
$(ELF): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Convert to hex
$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "Generated $@"

# Generic rule to compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# If you prefer an explicit rule for micro-ecc object:
# micro-ecc/uECC.o: micro-ecc/uECC.c
#   $(CC) $(CFLAGS) -c $< -o $@

size: $(ELF)
	$(SIZE) --format=avr --mcu=$(MCU) $(ELF)

flash: $(HEX)
	@echo "Flashing $(HEX) to $(MCU) via $(AVRDUDE_PROGRAMMER) on $(AVRDUDE_PORT)"
	$(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -P $(AVRDUDE_PORT) -b $(AVRDUDE_BAUD) -U flash:w:$(HEX):i

clean:
	rm -f $(OBJS) $(ELF) $(HEX)

# Convenience: show variables
print-%:
	@echo '$*=$($*)'
