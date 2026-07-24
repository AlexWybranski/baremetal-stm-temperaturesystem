#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.hpp"
#include "rcc.hpp"
#include "i2c.hpp"
#include "bme280.hpp"
#include "display.hpp"

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
void basicDelay(uint32_t ms);


int main(void) {
    RccHandle<0x40021000U> rcc;
    rcc.setMsiTo8MHz();
    rcc.enableGpioClock(0);
    rcc.enableGpioClock(1);
    rcc.enableGpioClock(2);

    GpioPortHandle<0x48000000U> gpioA;
    GpioPortHandle<0x48000400U> gpioB;
    GpioPortHandle<0x48000800U> gpioC;

    //PB8 - SCL PB9 - SDA
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 8);
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 9);
    gpioB.setPinType(1, 8);
    gpioB.setPinType(1, 9);
    gpioB.setPinAlternateFunction(4, 8);
    gpioB.setPinAlternateFunction(4, 9);

    rcc.enableI2cClock();

    I2cHandle<0x40005400U> i2c;
    i2c.enable();
    i2c.setTiming();

    gpioA.setPinMode(decltype(gpioA)::Mode::output, 5);

    BME280Handle<decltype(i2c)> bme280(i2c);

    if(!(bme280.isAlive())) {
        return 0;
    }

    bme280.init();

    DisplayDriver<decltype(gpioC), basicDelay> display(gpioC);

    gpioC.setPinMode(decltype(gpioC)::Mode::output, 0);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 1);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 2);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 3);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 4);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 5);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 6);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 7);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 8);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 9);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 10);
    gpioC.setPinMode(decltype(gpioC)::Mode::output, 11);

    gpioC.setPinState(1, 0);
    gpioC.setPinState(1, 1);
    gpioC.setPinState(1, 2);
    gpioC.setPinState(1, 3);

    int32_t temperature;

    display.testDisplay();

    while(1) {
        temperature = bme280.readTemp();
        display.sliceTemp(temperature);
        display.displayTemp();
        basicDelay(50);
    }
    return 0;
}

void basicDelay(uint32_t ms) {
    volatile uint32_t remaining_ms = ms; 

    while (remaining_ms > 0U) {
        for (volatile uint32_t i = 0U; i < 1000U; i = i + 1) {
            __asm__ __volatile__("nop");
        }
        remaining_ms = remaining_ms - 1;
    }
}