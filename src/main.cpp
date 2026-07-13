#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.hpp"
#include "rcc.hpp"

extern "C" {
    void _init(void) {}
    int main(void);
    void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                        StackType_t **ppxIdleTaskStackBuffer,
                                        uint32_t *pulIdleTaskStackSize )
    {
        static StaticTask_t xIdleTaskTCB;
        static StackType_t uxIdleTaskStack[128];

        *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
        *ppxIdleTaskStackBuffer = uxIdleTaskStack;
        *pulIdleTaskStackSize = 128;
    }
}
void basicDelay(int ms);


int main(void) {
    RccHandle<0x40021000U> rcc;
    rcc.enableGpioClock(0);

    GpioPortHandle<0x48000000U> gpioA;

    gpioA.setPinMode(decltype(gpioA)::Mode::output, 5);

    while(1) {
        gpioA.togglePin(5);
        basicDelay(250);
    }
    return 0;
}

void basicDelay(int ms) {
    ms *= 1000;

    while(ms>0) {
        __asm__ __volatile__("nop");
        --ms;
    }
}