MCU = atmega328p
F_CPU = 16000000UL
BAUD = 115200
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS = -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU) -Wall -I. -Imicro-ecc

# Fichiers sources locaux
SRC_LOCAL = main.c uart.c ui.c rng.c storage.c commands.c
# Fichier source de la librairie externe (micro-ecc)S
SRC_LIB = micro-ecc/uECC.c

# Tous les objets sont placés dans le répertoire courant
OBJ_LOCAL = $(SRC_LOCAL:.c=.o)
OBJ_LIB = uECC.o # L'objet de la librairie sera uECC.o
OBJ = $(OBJ_LOCAL) $(OBJ_LIB)

# Règle explicite pour compiler uECC.c en uECC.o
# $< est la dépendance (micro-ecc/uECC.c), $@ est la cible (uECC.o)
$(OBJ_LIB): $(SRC_LIB)
	$(CC) $(CFLAGS) -c $< -o $@

# Règle implicite pour les fichiers locaux
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

all: yubino.hex

yubino.elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

yubino.hex: yubino.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: yubino.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:yubino.hex

clean:
	rm -f *.o *.elf *.hex