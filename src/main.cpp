#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.hpp"
#include "rcc.hpp"
#include "i2c.hpp"
#include "bme280.hpp"
#include "display.hpp"
#include "nvic.hpp"

constexpr uint32_t RCC_BASEADDR     = 0x40021000U;
constexpr uint32_t I2C1_BASEADDR    = 0x40005400U;
constexpr uint32_t GPIOA_BASEADDR   = 0x48000000U;
constexpr uint32_t GPIOB_BASEADDR   = 0x48000400U;
constexpr uint32_t GPIOC_BASEADDR   = 0x48000800U;
constexpr uint32_t NVIC_BASEADDR    = 0xE000E000U;

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
    void I2C1_EV_Handler(void) {
        if(I2cHandle<I2C1_BASEADDR>::instance != nullptr) {
            I2cHandle<I2C1_BASEADDR>::instance->handleIRQ();
        }
    }

    void SystemInit(void) {
        RccHandle<RCC_BASEADDR>::setMsiTo8MHz();
    }
}
void basicDelay(uint32_t ms);

StackType_t readTemperatureTask[256];
StackType_t displayTemperatureTask[256];

StaticTask_t readTemperatureTaskBuffer;
StaticTask_t displayTemperatureTaskBuffer;

TaskHandle_t readTemperatureTaskHandle = nullptr;
TaskHandle_t displayTemperatureTaskHandle = nullptr;

struct d_ReadTempTaskContext {
    GpioPortHandle<GPIOB_BASEADDR>* gpioB_ptr;
    I2cHandle<I2C1_BASEADDR>* i2c_ptr;
    BME280Handle<I2cHandle<I2C1_BASEADDR>>* bme280_ptr;
};

struct d_DisplayTempTaskContext {
    DisplayDriver<GpioPortHandle<GPIOC_BASEADDR>, basicDelay>* display_ptr;
    GpioPortHandle<GPIOC_BASEADDR>* gpioC_ptr;
};

void vReadTempTask(void* pvParameters);
void vDisplayTempTask(void* pvParameters);

int main(void) {
    static NvicHandler<NVIC_BASEADDR> nvic;
    static RccHandle<RCC_BASEADDR> rcc;
    
    // GpioPortHandle<GPIOA_BASEADDR> gpioA;
    static GpioPortHandle<GPIOB_BASEADDR> gpioB;
    static GpioPortHandle<GPIOC_BASEADDR> gpioC;
    
    static I2cHandle<I2C1_BASEADDR> i2c;

    static BME280Handle<I2cHandle<I2C1_BASEADDR>> bme280(i2c);
    static DisplayDriver<GpioPortHandle<GPIOC_BASEADDR>, basicDelay> display(gpioC);

    static d_ReadTempTaskContext readTempTaskContext;
    readTempTaskContext.gpioB_ptr = &gpioB;
    readTempTaskContext.i2c_ptr = &i2c;
    readTempTaskContext.bme280_ptr = &bme280;

    static d_DisplayTempTaskContext displayTempTaskContext;
    displayTempTaskContext.display_ptr = &display;
    displayTempTaskContext.gpioC_ptr = &gpioC;

    nvic.setIRQpriority(31, 6);
    nvic.setIRQpriority(32, 6);
    nvic.enableIRQ(31);
    nvic.enableIRQ(32);

    for(int i = 1; i < 3; ++i) {
        rcc.enableGpioClock(i);
    }

    //PB8 - SCL PB9 - SDA
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 8);
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 9);
    gpioB.setPinType(1, 8);
    gpioB.setPinType(1, 9);
    gpioB.setPinAlternateFunction(4, 8);
    gpioB.setPinAlternateFunction(4, 9);

    rcc.enableI2cClock();

    i2c.enable();
    i2c.setTiming();

    // gpioA.setPinMode(decltype(gpioA)::Mode::output, 5);

    for(int i = 0; i < 12; ++i) {
        gpioC.setPinMode(decltype(gpioC)::Mode::output, i);
    }

    gpioC.setPinState(1, 0);
    gpioC.setPinState(1, 1);
    gpioC.setPinState(1, 2);
    gpioC.setPinState(1, 3);

    display.testDisplay();

    readTemperatureTaskHandle = xTaskCreateStatic(  
        vReadTempTask, "readTemp", 256, &readTempTaskContext, 3, readTemperatureTask, &readTemperatureTaskBuffer);

    displayTemperatureTaskHandle = xTaskCreateStatic(
        vDisplayTempTask, "display", 256, &displayTempTaskContext, 2, displayTemperatureTask, &displayTemperatureTaskBuffer);

    
    vTaskStartScheduler();

    while(1) {}

    return 0;
}

void vReadTempTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_ReadTempTaskContext*>(pvParameters);
    
    auto* gpioB = context->gpioB_ptr;
    auto* i2c = context->i2c_ptr;
    auto* bme280 = context->bme280_ptr;
    
    if(!(bme280->isAlive())) {
        *(volatile uint32_t*)(0xE000ED0C) = 0x05FA0004; //to modify later
    }

    bme280->init();

    int32_t temperature = 0;

    while(1) {
        temperature = bme280->readTemp();

        xTaskNotify(displayTemperatureTaskHandle, static_cast<uint32_t>(temperature), eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(950));
    }
}

void vDisplayTempTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_DisplayTempTaskContext*>(pvParameters);

    auto* gpioC = context->gpioC_ptr;
    auto* display = context->display_ptr;

    uint32_t receivedTemp = 0;

    while(1) {
        if(xTaskNotifyWait(0x0UL, 0xffffffffUL, &receivedTemp, 0) == pdTRUE) {
            display->sliceTemp(static_cast<int32_t>(receivedTemp));
        }

        display->displayTemp();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
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