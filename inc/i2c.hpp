#ifndef I2C_HPP
#define I2C_HPP
#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

namespace I2cBitPositions {
    //CR1
    static constexpr uint32_t CR1_PE         = (0b1U << 0U);
    static constexpr uint32_t CR1_TXIE       = (0b1U << 1U);
    static constexpr uint32_t CR1_RXIE       = (0b1U << 2U);
    static constexpr uint32_t CR1_STOPIE     = (0b1U << 5U);
    static constexpr uint32_t CR1_TCIE       = (0b1U << 6U);

    //CR2
    static constexpr uint32_t CR2_SADD_pos   = 1U;
    static constexpr uint32_t CR2_NBYTES_pos = 16U;

    static constexpr uint32_t CR2_RD_WRN     = (0b1U << 10U);
    static constexpr uint32_t CR2_START      = (0b1U << 13U);
    static constexpr uint32_t CR2_STOP       = (0b1U << 14U);
    static constexpr uint32_t CR2_AUTOEND    = (0b1U << 25U);

    //ISR
    static constexpr uint32_t ISR_TXIS       = (0b1U << 1U);
    static constexpr uint32_t ISR_RXNE       = (0b1U << 2U);
    static constexpr uint32_t ISR_STOPF      = (0b1U << 5U);
    static constexpr uint32_t ISR_TC         = (0b1U << 6U);

    //ICR
    static constexpr uint32_t ICR_STOPCF     = (0b1U << 5U);
}

struct I2C_regs {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t TIMINGR;
    volatile uint32_t TIMEOUTR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t PECR;
    volatile uint32_t RXDR;
    volatile uint32_t TXDR;
};

template<uint32_t baseAddr>
class I2cHandle {
    private:
        I2C_regs* const m_I2C;
        
        uint32_t m_devAddr;
        
        volatile uint8_t m_buffIterator;
        volatile uint8_t m_bufferLimiter;

        uint8_t m_txBuffer[2];
        uint8_t m_rxBuffer[8];

        uint8_t m_rxDataSize;
        uint8_t* volatile m_rxBufferDestinationPtr;

        bool m_readModeOn;

        volatile bool m_transferComplete;
        TaskHandle_t m_taskToNotify = nullptr;

    public:
        static I2cHandle<baseAddr>* instance;
        I2cHandle() : m_I2C(reinterpret_cast<I2C_regs*>(baseAddr)) {
            instance = this;
        }
        ~I2cHandle() = default;

        void enable() {
            using namespace I2cBitPositions;
            //setup interrupts <- TCIE STOPIE | opt for now: NACKIE ERRIE
            m_I2C->CR1 |= (CR1_TCIE | CR1_STOPIE);
            
            m_I2C->CR1 |= CR1_PE;
        }

        void setTiming() {
            //values for specific bits from stm32l476 reference manual
            constexpr uint32_t SCLL = (0x13U << 0U);
            constexpr uint32_t SCLH = (0xFU << 8U);
            constexpr uint32_t SDADEL = (0x2U << 16U);
            constexpr uint32_t SCLDEL = (0x4U << 20U);
            constexpr uint32_t PRESC = (0x1U << 28U);

            constexpr uint32_t timingValue = (SCLL | SCLH | SDADEL | SCLDEL | PRESC);

            m_I2C->TIMINGR = timingValue;
        }

        //this functions work only in 7 bit addressing mode and take only 1 byte to send
        void write(uint32_t devAddr, 
                   uint8_t innerAddr,
                   uint8_t value)
        {
            using namespace I2cBitPositions;
            m_transferComplete = false;
            m_txBuffer[0] = innerAddr;
            m_txBuffer[1] = value;
            m_bufferLimiter = 2;
            m_buffIterator = 0;
            m_readModeOn = 0;

            uint32_t cr2RegMask = m_I2C->CR2;
            cr2RegMask = 0;

            uint8_t nbytes;
            nbytes = 0x2U;

            m_taskToNotify = xTaskGetCurrentTaskHandle();
            
            cr2RegMask =    ((devAddr << CR2_SADD_pos) |
                            (nbytes << CR2_NBYTES_pos) |
                            CR2_AUTOEND | CR2_START);
            
            m_I2C->CR1 |= CR1_TXIE;
            
            m_I2C->CR2 = cr2RegMask;

            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }

        void read(uint32_t devAddr, 
                  uint8_t innerAddr,
                  uint8_t* dataBuffer,
                  uint8_t dataSize)
        {           
            using namespace I2cBitPositions;
            m_transferComplete = false;
            m_devAddr = devAddr;
            m_rxBufferDestinationPtr = dataBuffer;
            m_rxDataSize = dataSize;
            m_txBuffer[0] = innerAddr;
            m_bufferLimiter = 1;
            m_buffIterator = 0;
            m_readModeOn = 1;

            uint32_t cr2RegMask = m_I2C->CR2;
            uint8_t nbytes = 0x1U;
            
            m_taskToNotify = xTaskGetCurrentTaskHandle();

            cr2RegMask = 0;

            cr2RegMask =    ((devAddr << CR2_SADD_pos) |
                            (nbytes << CR2_NBYTES_pos) | CR2_START);
            
            m_I2C->CR1 |= CR1_TXIE;
            m_I2C->CR2 = cr2RegMask;

            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }

        void handleIRQ() {
            using namespace I2cBitPositions;

            if((m_I2C->ISR & ISR_RXNE) && (m_I2C->CR1 & CR1_RXIE)) {
                if(m_buffIterator < m_bufferLimiter) {
                    m_rxBufferDestinationPtr[m_buffIterator] = m_I2C->RXDR;
                    ++m_buffIterator;
                } else {
                    m_I2C->CR1 &= ~CR1_RXIE;
                }
                return;
            }

            if((m_I2C->ISR & ISR_TXIS) && (m_I2C->CR1 & CR1_TXIE)) {
                if(m_buffIterator < m_bufferLimiter) {
                    m_I2C->TXDR = m_txBuffer[m_buffIterator];
                    ++m_buffIterator;
                } else {
                    m_I2C->CR1 &= ~CR1_TXIE;
                }
                return;
            }

            if((m_I2C->ISR & ISR_TC) && (m_I2C->CR1 & CR1_TCIE)) {
                m_buffIterator = 0;
                m_bufferLimiter = m_rxDataSize; 
                uint32_t cr2RegMask;

                m_I2C->CR1 &= ~CR1_TXIE;
                m_I2C->CR1 |= CR1_RXIE;

                cr2RegMask =    ((m_devAddr << CR2_SADD_pos) |
                                (static_cast<uint32_t>(m_rxDataSize) << CR2_NBYTES_pos) |
                                CR2_AUTOEND |
                                CR2_RD_WRN |
                                CR2_START
                                );

                m_I2C->CR2 = cr2RegMask;

                return;
            }

            if((m_I2C->ISR & ISR_STOPF) && (m_I2C->CR1 & CR1_STOPIE)) {
                m_I2C->ICR |= ICR_STOPCF;
                m_transferComplete = true;
            }

            if(m_transferComplete) {
                BaseType_t xHigherPriorityTaskHasWoken = pdFALSE;

                if (m_taskToNotify != nullptr) {
                    vTaskNotifyGiveFromISR(m_taskToNotify, &xHigherPriorityTaskHasWoken);
                    m_taskToNotify = nullptr;
                }

                if(m_readModeOn && (m_rxBufferDestinationPtr != nullptr)) {
                    m_rxBufferDestinationPtr = nullptr;
                }

                m_buffIterator = 0;
                m_bufferLimiter = 0;
                m_readModeOn = 0;

                m_I2C->CR1 &= ~CR1_RXIE;
                m_I2C->CR1 &= ~CR1_TXIE;

                portYIELD_FROM_ISR(xHigherPriorityTaskHasWoken);

                return;
            }
        }
};

template <uint32_t baseAddr>
I2cHandle<baseAddr>* I2cHandle<baseAddr>::instance = nullptr;

#endif