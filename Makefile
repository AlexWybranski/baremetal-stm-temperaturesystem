CC		=arm-none-eabi-gcc
CXX 	=arm-none-eabi-g++
LD		=arm-none-eabi-g++

MACH=cortex-m4

CFLAGS= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -Wall -std=gnu11 -O0 -g
CXXFLAGS= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -O0 -g -fno-exceptions -fno-rtti -fno-threadsafe-statics -std=gnu++23
LINKFLAGS= --specs=nano.specs -mcpu=$(MACH) -mfloat-abi=hard -nostartfiles -mthumb -T stm32_linker_script.ld -Wl,-Map=main.map

all: stm32l4_startup.o main.o syscalls.o main.elf 

main.o:main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

stm32l4_startup.o:stm32l4_startup.c
	$(CC) $(CFLAGS) $^ -o $@

syscalls.o:syscalls.c
	$(CC) $(CFLAGS) $^ -o $@

main.elf:main.o stm32l4_startup.o syscalls.o
	$(LD) $(LINKFLAGS) $^ -o $@

clean:
	rm -rf *.o *.elf *.map

load:
	openocd -f /usr/share/openocd/scripts/board/st_nucleo_l4.cfg