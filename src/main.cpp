#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include "gpio.hpp"
#include "rcc.hpp"
#include "i2c.hpp"
#include "bme280.hpp"

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
    rcc.setMsiTo8MHz();
    rcc.enableGpioClock(0);
    rcc.enableGpioClock(1);

    GpioPortHandle<0x48000000U> gpioA;
    GpioPortHandle<0x48000400U> gpioB;

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


    while(1) {
        bme280.readTemp();
        gpioA.togglePin(5);
        basicDelay(1000);
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