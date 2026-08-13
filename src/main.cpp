#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

#include "gpio.hpp"
#include "rcc.hpp"
#include "i2c.hpp"
#include "usart.hpp"
#include "bme280.hpp"
#include "display.hpp"
#include "nvic.hpp"
#include "iwdg.hpp"

#include "cmdparser.hpp"

constexpr uint32_t RCC_BASEADDR     = 0x40021000U;
constexpr uint32_t I2C1_BASEADDR    = 0x40005400U;
constexpr uint32_t USART1_BASEADDR  = 0x40004400U; //now: usart2 | default USART1 - 0x40013800U
constexpr uint32_t GPIOA_BASEADDR   = 0x48000000U;
constexpr uint32_t GPIOB_BASEADDR   = 0x48000400U;
constexpr uint32_t GPIOC_BASEADDR   = 0x48000800U;
constexpr uint32_t NVIC_BASEADDR    = 0xE000E000U;
constexpr uint32_t IWDG_BASEADDR    = 0x40003000U;

constexpr uint32_t CLOCK_SPEED      = 8000000U;

constexpr uint32_t SCB_AIRCR        = 0xE000ED0CU;
constexpr uint32_t SYSRESET_VECTKEY = 0x05FA0004U;

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

    void USART2_Handler(void) {
        if(UsartHandle<USART1_BASEADDR>::instance != nullptr) {
            UsartHandle<USART1_BASEADDR>::instance->handleIRQ();
        }
    }

    void SystemInit(void) {
        RccHandle<RCC_BASEADDR>::setMsiTo8MHz();
    }

    void SystemReset(void) { 
        *(volatile uint32_t*)(SCB_AIRCR) = SYSRESET_VECTKEY; 
    }
}
void basicDelay(uint32_t ms);

StackType_t readTemperatureTask[128];
StackType_t displayTemperatureTask[128];
StackType_t respondUartTask[128];
StackType_t watchdogTask[64];

StaticTask_t readTemperatureTaskBuffer;
StaticTask_t displayTemperatureTaskBuffer;
StaticTask_t respondUartTaskBuffer;
StaticTask_t watchdogTaskBuffer;

TaskHandle_t readTemperatureTaskHandle = nullptr;
TaskHandle_t displayTemperatureTaskHandle = nullptr;
TaskHandle_t respondUartTaskHandle = nullptr;
TaskHandle_t watchdogTaskHandle = nullptr;

SemaphoreHandle_t xTemperatureMutex = nullptr;
StaticSemaphore_t xMutexBuffer;

EventGroupHandle_t xWatchdogEventGroupHandle = nullptr;
StaticEventGroup_t xWatchdogEventGroup;

EventGroupHandle_t xDisplayEventGroupHandle = nullptr;
StaticEventGroup_t xDisplayEventGroup;

namespace WatchdogBits {
    constexpr EventBits_t xSensorIsAlive = (0b1U << 0);
    constexpr EventBits_t xDisplayIsAlive = (0b1U << 1U);
    constexpr EventBits_t xUartIsAlive = (0b1U << 2U);

    constexpr EventBits_t xAllFlags = (xSensorIsAlive | xDisplayIsAlive | xUartIsAlive);
};

namespace DisplayBits {
    constexpr EventBits_t xTemperatureChanged = (0b1U << 0);
    constexpr EventBits_t xTestDisplay = (0b1U << 1U);
}

int32_t g_temperature;

struct d_ReadTempTaskContext {
    BME280Handle<I2cHandle<I2C1_BASEADDR>>* bme280_ptr;
};

struct d_DisplayTempTaskContext {
    DisplayDriver<GpioPortHandle<GPIOC_BASEADDR>, basicDelay>* display_ptr;
};

struct d_RespondUartTaskContext {
    UsartHandle<USART1_BASEADDR>* usart_ptr;
    CommandParser* parser_ptr;
};

struct d_WatchdogTaskContext {
    IwdgHandle<IWDG_BASEADDR>* iwdg_ptr;
};

void vReadTempTask(void* pvParameters);
void vDisplayTempTask(void* pvParameters);
void vRespondUartTask(void* pvParameters);
void vWatchdogTask(void* pvParameters);

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

    static CommandParser parser;

    static IwdgHandle<IWDG_BASEADDR> iwdg;

    static d_ReadTempTaskContext readTempTaskContext;
    readTempTaskContext.bme280_ptr = &bme280;

    static d_DisplayTempTaskContext displayTempTaskContext;
    displayTempTaskContext.display_ptr = &display;

    static d_RespondUartTaskContext respondUartTaskContext;
    respondUartTaskContext.usart_ptr = &usart;
    respondUartTaskContext.parser_ptr = &parser;

    static d_WatchdogTaskContext watchdogTaskContext;
    watchdogTaskContext.iwdg_ptr = &iwdg;

    if(xTemperatureMutex == nullptr) {
        xTemperatureMutex = xSemaphoreCreateMutexStatic(&xMutexBuffer);
        if(xTemperatureMutex != nullptr) {
            xSemaphoreGive(xTemperatureMutex);
        }
    }

    nvic.setIRQpriority(31, 6);
    nvic.setIRQpriority(32, 6);
    nvic.setIRQpriority(38, 5);
    nvic.enableIRQ(31);
    nvic.enableIRQ(32);
    nvic.enableIRQ(38);

    for(int i = 0; i < 3; ++i) {
        rcc.enableGpioClock(i);
    }

    //PA9 - TX  PA10 - RX
    // gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 9);
    // gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 10);
    // gpioA.setPinAlternateFunction(7, 9);
    // gpioA.setPinAlternateFunction(7, 10);

    // //st-link is connected to pa2 and pa3 with default usart2
    gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 2);
    gpioA.setPinMode(decltype(gpioA)::Mode::alternate, 3);
    gpioA.setPinAlternateFunction(7, 2);
    gpioA.setPinAlternateFunction(7, 3);

    //PB8 - SCL PB9 - SDA
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 8);
    gpioB.setPinMode(decltype(gpioB)::Mode::alternate, 9);
    gpioB.setPinPullUp(decltype(gpioB)::Pull::pullup, 8);
    gpioB.setPinPullUp(decltype(gpioB)::Pull::pullup, 9);
    gpioB.setPinType(1, 8);
    gpioB.setPinType(1, 9);
    gpioB.setPinAlternateFunction(4, 8);
    gpioB.setPinAlternateFunction(4, 9);

    rcc.enableI2cClock();
    rcc.enableUsartClock();

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

    readTemperatureTaskHandle = xTaskCreateStatic(  
        vReadTempTask, "readTemp", 128, &readTempTaskContext, 2, readTemperatureTask, &readTemperatureTaskBuffer);

    displayTemperatureTaskHandle = xTaskCreateStatic(
        vDisplayTempTask, "display", 128, &displayTempTaskContext, 1, displayTemperatureTask, &displayTemperatureTaskBuffer);

    respondUartTaskHandle = xTaskCreateStatic(
        vRespondUartTask, "respondUart", 128, &respondUartTaskContext, 3, respondUartTask, &respondUartTaskBuffer);

    watchdogTaskHandle = xTaskCreateStatic(
        vWatchdogTask, "watchdog", 64, &watchdogTaskContext, 4, watchdogTask, &watchdogTaskBuffer);

    xWatchdogEventGroupHandle = xEventGroupCreateStatic(&xWatchdogEventGroup);

    configASSERT(xWatchdogEventGroupHandle);

    xDisplayEventGroupHandle = xEventGroupCreateStatic(&xDisplayEventGroup);

    configASSERT(xDisplayEventGroupHandle);

    vTaskStartScheduler();

    while(1) {}

    return 0;
}

void vReadTempTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_ReadTempTaskContext*>(pvParameters);
    
    auto* bme280 = context->bme280_ptr;
    
    if(bme280->isAlive()) {
        bme280->init();
    } else {
        SystemReset();
    }

    int32_t localTemperature = 0;

    while(1) {
        if(bme280->isAlive()) {
            xEventGroupSetBits(xWatchdogEventGroupHandle, WatchdogBits::xSensorIsAlive);

            localTemperature = bme280->readTemp();
    
            xSemaphoreTake(xTemperatureMutex, portMAX_DELAY);
            g_temperature = localTemperature;
            xSemaphoreGive(xTemperatureMutex);

            xEventGroupSetBits(xDisplayEventGroupHandle, DisplayBits::xTemperatureChanged);
        }

        vTaskDelay(pdMS_TO_TICKS(950));
    }
}

void vDisplayTempTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_DisplayTempTaskContext*>(pvParameters);

    auto* display = context->display_ptr;

    uint32_t localTemp = 0;

    EventBits_t displayBits;

    while(1) {
        xEventGroupSetBits(xWatchdogEventGroupHandle, WatchdogBits::xDisplayIsAlive);

        displayBits = xEventGroupClearBits(xDisplayEventGroupHandle, (DisplayBits::xTemperatureChanged | DisplayBits::xTestDisplay));

        if((displayBits & DisplayBits::xTemperatureChanged) != 0) {
            xSemaphoreTake(xTemperatureMutex, portMAX_DELAY);
            localTemp = g_temperature;
            xSemaphoreGive(xTemperatureMutex);

            display->sliceTemp(static_cast<int32_t>(localTemp));
        }

        if((displayBits & DisplayBits::xTestDisplay) != 0) {
            display->testDisplay();
        }

        display->displayTemp();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void vRespondUartTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_RespondUartTaskContext*>(pvParameters);

    auto* usart = context->usart_ptr;
    auto* parser = context->parser_ptr;

    char receivedCharacter;
    char receivedCommand[32];
    uint8_t receivedCommandLength;

    char respondText[32];

    int32_t localTemperature;

    usart->setTaskToNotify();

    while(1) {
        xEventGroupSetBits(xWatchdogEventGroupHandle, WatchdogBits::xUartIsAlive);

        if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000)) == pdPASS) {
            for(uint8_t i = 0; i < 32; ++i) {
                respondText[i] = 0;
            }
            
            uint8_t it = 0;
            while(usart->takeByteFromRxBuffer(receivedCharacter)) {
                if(receivedCharacter == '\n') {
                    receivedCommand[it] = '\0';
                    break;
                }
                receivedCommand[it] = receivedCharacter;
                it++;
            }
            
            receivedCommandLength = it; 
            
            CommandParser::Command cmd = parser->parseCommand(receivedCommand, receivedCommandLength);
            
            using enum CommandParser::Command;
            
            switch(cmd) {
                case GetTemp:
                    xSemaphoreTake(xTemperatureMutex, portMAX_DELAY);
                    localTemperature = g_temperature;
                    xSemaphoreGive(xTemperatureMutex);
                    parser->generateTextForGetTemp(respondText, localTemperature);
                    usart->transmitText(respondText);
                    break;
            
                case ResetSystem:
                    parser->generateTextForCommand(respondText, cmd);
                    usart->transmitText(respondText);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    SystemReset();
                    break;

                case TestDisplay:
                    parser->generateTextForCommand(respondText, cmd);
                    usart->transmitText(respondText);
                    xEventGroupSetBits(xDisplayEventGroupHandle, DisplayBits::xTestDisplay);
                    break;
            
                case Unknown:
                    parser->generateTextForCommand(respondText, cmd);
                    usart->transmitText(respondText);
                    break;
            }

        }
    }
}

void vWatchdogTask(void* pvParameters) {
    auto* context = reinterpret_cast<d_WatchdogTaskContext*>(pvParameters); 

    auto* iwdg = context->iwdg_ptr;

    iwdg->setup();

    while(1) {
        EventBits_t tasksBits = xEventGroupWaitBits(xWatchdogEventGroupHandle,
                                                    WatchdogBits::xAllFlags,
                                                    pdTRUE,
                                                    pdTRUE,
                                                    pdMS_TO_TICKS(3000));

        if((tasksBits & WatchdogBits::xAllFlags) == WatchdogBits::xAllFlags) {
            iwdg->feedWatchdog();
        }
    }
}


void basicDelay(uint32_t ms) {
    volatile uint32_t remaining_ms = ms; 

    while (remaining_ms > 0U) {
        for (volatile uint32_t i = 0U; i < 1000U; i = i + 1) {
            __asm__ __volatile__("nop");
        }
        uint32_t tempVar = remaining_ms - 1;
        remaining_ms = tempVar;
    }
}