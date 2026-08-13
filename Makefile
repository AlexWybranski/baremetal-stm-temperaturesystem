CC		:=arm-none-eabi-gcc
CXX 	:=arm-none-eabi-g++
LD		:=arm-none-eabi-g++

MACH	:=cortex-m4

INCFLAGS	:= -Iinc -Ifreertos/include -Ifreertos/source/ARM_CM4F

CFLAGS		:= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -mfpu=fpv4-sp-d16 -Wall -std=gnu11 -O2 -g
CXXFLAGS	:= -c -mthumb -mfloat-abi=hard -mcpu=$(MACH) -mfpu=fpv4-sp-d16 -O2 -g -fno-exceptions -fno-rtti -fno-threadsafe-statics -std=gnu++23
LINKFLAGS	:= --specs=nano.specs -mcpu=$(MACH) -mfpu=fpv4-sp-d16 -lstdc++ -mfloat-abi=hard -nostartfiles -mthumb -T startup/stm32_linker_script.ld -Wl,-Map=build/main.map

CFLAGS		+= $(INCFLAGS)
CXXFLAGS	+= $(INCFLAGS)

RTOSSRCS	:= $(shell find freertos -name '*.c')
CSRCS		:= $(shell find startup src -name '*.c')
CPPSRCS		:= $(shell find src -name '*.cpp')

RTOSOBJS	:= $(patsubst freertos/%.c,build/freertos/%.o,$(RTOSSRCS))
CPPOBJS   	:= $(patsubst src/%.cpp,build/%.o,$(CPPSRCS))
COBJS     	:= $(patsubst startup/%.c,build/%.o,$(patsubst src/%.c,build/%.o,$(CSRCS)))

TARGET		:= build/main.elf

all: $(TARGET)

build/freertos/%.o: freertos/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $< -o $@

build/%.o: startup/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET): $(COBJS) $(CPPOBJS) $(RTOSOBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LINKFLAGS) $^ -o $@

clean:
	rm -rf build/*

load: $(TARGET)
	openocd -f /usr/share/openocd/scripts/board/st_nucleo_l4.cfg

clang-analysis: $(TARGET)
	clang-tidy src/main.cpp src/cmdparser.cpp   --header-filter=inc/.*   -- -std=c++23 -Iinc -Ifreertos/include -Ifreertos/source/ARM_CM4F

cppcheck: $(TARGET)
	cppcheck --enable=all \
         --std=c++23 \
         -D__GNUC__ \
         --inline-suppr \
         --suppress=missingIncludeSystem \
         -I inc \
         -i freertos \
         -i startup \
         -i src/syscalls.c \
         src inc

.PHONY: all clean load clang-analysis cppcheck