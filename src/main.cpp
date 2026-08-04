#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "gpio.hpp"
#include "rcc.hpp"
#include "i2c.hpp"
#include "usart.hpp"
#include "bme280.hpp"
#include "display.hpp"
#include "nvic.hpp"

constexpr uint32_t RCC_BASEADDR     = 0x40021000U;
constexpr uint32_t I2C1_BASEADDR    = 0x40005400U;
constexpr uint32_t USART1_BASEADDR  = 0x40013800U;
constexpr uint32_t GPIOA_BASEADDR   = 0x48000000U;
constexpr uint32_t GPIOB_BASEADDR   = 0x48000400U;
constexpr uint32_t GPIOC_BASEADDR   = 0x48000800U;
constexpr uint32_t NVIC_BASEADDR    = 0xE000E000U;

constexpr uint32_t CLOCK_SPEED      = 8000000U;

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

    void USART1_Handler(void) {
        if(UsartHandle<USART1_BASEADDR>::instance != nullptr) {
            UsartHandle<USART1_BASEADDR>::instance->handleIRQ();
        }
    }

    void SystemInit(void) {
        RccHandle<RCC_BASEADDR>::setMsiTo8MHz();
    }

    void SystemReset(void) { 
        *(volatile uint32_t*)(0xE000ED0C) = 0x05FA0004; // ← 
    }
}
void basicDelay(uint32_t ms);

StackType_t readTemperatureTask[256];
StackType_t displayTemperatureTask[256];
StackType_t respondUartTask[256];

StaticTask_t readTemperatureTaskBuffer;
StaticTask_t displayTemperatureTaskBuffer;
StaticTask_t respondUartTaskBuffer;

TaskHandle_t readTemperatureTaskHandle = nullptr;
TaskHandle_t displayTemperatureTaskHandle = nullptr;
TaskHandle_t respondUartTaskHandle = nullptr;

SemaphoreHandle_t xTemperatureMutex = nullptr;
StaticSemaphore_t xMutexBuffer;
int32_t g_temperaute;

struct d_ReadTempTaskContext {
    GpioPortHandle<GPIOB_BASEADDR>* gpioB_ptr;
    I2cHandle<I2C1_BASEADDR>* i2c_ptr;
    BME280Handle<I2cHandle<I2C1_BASEADDR>>* bme280_ptr;
};

struct d_DisplayTempTaskContext {
    DisplayDriver<GpioPortHandle<GPIOC_BASEADDR>, basicDelay>* display_ptr;
    GpioPortHandle<GPIOC_BASEADDR>* gpioC_ptr;
};

struct d_RespondUartTaskContext {
    UsartHandle<USART_BASEADDR>* usart_ptr;
}

void vReadTempTask(void* pvParameters);
void vDisplayTempTask(void* pvParameters);
void vRespondUartTask(void* pvParameters);

int main(void) {
    static NvicHandler<NVIC_BASEADDR> nvic;
    static RccHandle<RCC_BASEADDR> rcc;
    
    static GpioPortHandle<GPIOA_BASEADDR> gpioA;
    static GpioPortHandle<GPIOB_BASEADDR> gpioB;
    static GpioPortHandle<GPIOC_BASEADDR> gpioC;
    
    static I2cHandle<I2C1_BASEADDR> i2c;
    static UsartHandle<USART1_BASEADDR> usart;

    static BME280Handle<I2cHandle<I2C1_BASEADDR>> bme280(i2c);
    static DisplayDriver<GpioPortHandle<GPIOC_BASEADDR>, basicDelay> display(gpioC);

    static d_ReadTempTaskContext readTempTaskContext;
    readTempTaskContext.gpioB_ptr = &gpioB;
    readTempTaskContext.i2c_ptr = &i2c;
    readTempTaskContext.bme280_ptr = &bme280;

    static d_DisplayTempTaskContext displayTempTaskContext;
    displayTempTaskContext.display_ptr = &display;
    displayTempTaskContext.gpioC_ptr = &gpioC;

    static d_RespondUartTaskContext respondUartTaskContext;
    respondUartTaskContext.usart_ptr = &usart;

    if(xTemperatureMutex == nullptr) {
        xTemperatureMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
        if(xTemperatureMutex != nullptr) {
            xSemaphoreGive(xTemperatureMutex);
        }
    }


    nvic.setIRQpriority(31, 6);
    nvic.setIRQpriority(32, 6);
    nvic.enableIRQ(31);
    nvic.enableIRQ(32);

    for(int i = 0; i < 3; ++i) {
        rcc.enableGpioClock(i);
    }

    //PA9 - TX  PA10 - RX
    gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 9);
    gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 10);
    gpioA.setPinAlternateFunction(7, 9);
    gpioA.setPinAlternateFunction(7, 10);

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

    usart.setBaudRate(CLOCK_SPEED, 9600);
    usart.enable();

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
        SystemReset();
    }

    bme280->init();

    int32_t temperature = 0;

    while(1) {
        xSemaphoreTake(xTemperatureMutex, portMAX_DELAY);
        temperature = bme280->readTemp();
        xSemaphoreGive(xTemperatureMutex);

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

void vRespondUartTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_RespondUartTaskContext*>(pvParameters);

    auto* usart = context->usart_ptr;

    char receivedCharacter;
    char[8] receivedCommand;
    int32_t localTemp;

    while(1) {
        for(int i = 0; i < 8; ++i) {
            if(takeByteFromRxBuffer(receivedCharacter)) {
                receivedCommand[i] = receivedCharacter;
            } else {
                break;
            }
        }

        if(parseCommand(receivedCommand)) {
            commandResponse(localTemp);
        }
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