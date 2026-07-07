CC		:=arm-none-eabi-gcc
CXX 	:=arm-none-eabi-g++
LD		:=arm-none-eabi-g++

MACH	:=cortex-m4

CFLAGS		:= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -Wall -std=gnu11 -O0 -g
CXXFLAGS	:= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -O0 -g -fno-exceptions -fno-rtti -fno-threadsafe-statics -std=gnu++23
LINKFLAGS	:= --specs=nano.specs -mcpu=$(MACH) -lstdc++ -mfloat-abi=hard -nostartfiles -mthumb -T startup/stm32_linker_script.ld -Wl,-Map=build/main.map


CSRCS		:= $(shell find startup src -name '*.c')
CPPSRCS		:= $(shell find src -name '*.cpp')

CPPOBJS   := $(patsubst src/%.cpp,build/%.o,$(CPPSRCS))
COBJS     := $(patsubst startup/%.c,build/%.o,$(patsubst src/%.c,build/%.o,$(CSRCS)))

TARGET		:= build/main.elf
#dodaj obsługę kompilacji dla folderu freertos
all: $(TARGET)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

build/%.o: startup/%.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(COBJS) $(CPPOBJS)
	$(LD) $(LINKFLAGS) $^ -o $@

clean:
	rm -rf build/*

load: $(TARGET)
	openocd -f /usr/share/openocd/scripts/board/st_nucleo_l4.cfg

.PHONY: all clean load