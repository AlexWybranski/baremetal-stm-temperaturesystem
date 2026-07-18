#ifndef I2C_HPP
#define I2C_HPP
#include <cstdint>

//CR2
static constexpr uint8_t SADD_pos = 1;
static constexpr uint8_t RD_WRN_pos = 10;
static constexpr uint8_t START_pos = 13;
static constexpr uint8_t STOP_pos = 14;
static constexpr uint8_t NBYTES_pos = 16;
static constexpr uint8_t AUTOEND_pos = 25;

//ISR
static constexpr uint8_t TXIS_pos = 1;
static constexpr uint8_t RXNE_pos = 2;
static constexpr uint8_t STOPF_pos = 5;
static constexpr uint8_t TC_pos = 6;

//ICR
static constexpr uint8_t STOPCF_pos = 5;

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
    public:
        I2cHandle() : m_I2C(reinterpret_cast<I2C_regs*>(baseAddr)) {}
        ~I2cHandle() = default;

        void enable() {
            m_I2C->CR1 |= (1U << 0);
        }

        void setTiming() {
            //values for specific bits from stm32l476 reference manual
            constexpr uint32_t SCLL = 0x13U << 0U;
            constexpr uint32_t SCLH = 0xFU << 8U;
            constexpr uint32_t SDADEL = 0x2U << 16U;
            constexpr uint32_t SCLDEL = 0x4U << 20U;
            constexpr uint32_t PRESC = 0x1U << 28U;

            constexpr uint32_t timingValue = (SCLL | SCLH | SDADEL | SCLDEL | PRESC);

            m_I2C->TIMINGR = timingValue;
        }

        //this function works only in 7 bit addressing mode
        void write(uint32_t devAddr, 
                   uint32_t innerAddr,
                   uint8_t* dataBuffer,
                   uint8_t messageSize)
        {
            uint32_t cr2RegMask = m_I2C->CR2;
            cr2RegMask = 0;

            cr2RegMask = ((devAddr << SADD_pos) |
                          (static_cast<uint32_t>(messageSize+1) << NBYTES_pos) |
                          (0b1U << AUTOEND_pos)
                          );

            m_I2C->CR2 = cr2RegMask;
            //start
            m_I2C->CR2 |= (0b1U << START_pos);

            //wait for TXIS flag
            while((m_I2C->ISR & (0b1U << TXIS_pos)) == 0) {}

            m_I2C->TXDR = innerAddr;

            for (uint8_t i = 0; i < messageSize; i++) {
                while((m_I2C->ISR & (0b1U << TXIS_pos)) == 0) {} // wait for TXIS
                m_I2C->TXDR = dataBuffer[i];
            }
            
            //wait for STOPF flag
            while((m_I2C->ISR & (0b1U << STOPF_pos)) == 0) {}

            //clear STOPCF flag
            m_I2C->ICR |= (0b1U << STOPCF_pos);
        }

        void read(uint32_t devAddr, 
                  uint8_t innerAddr,
                  uint8_t* dataBuffer,
                  uint8_t dataSize)
        {
            uint32_t cr2RegMask = m_I2C->CR2;
            cr2RegMask = 0;

            cr2RegMask = ((devAddr << SADD_pos) | (0x1U << NBYTES_pos));

            m_I2C->CR2 = cr2RegMask;
            //start
            m_I2C->CR2 |= (0b1U << START_pos);

            //wait for TXIS flag
            while((m_I2C->ISR & (0b1U << TXIS_pos)) == 0) {}

            m_I2C->TXDR = innerAddr;

            while((m_I2C->ISR & (0b1U << TC_pos)) == 0) {}

            cr2RegMask = 0;
            
            cr2RegMask = ((devAddr << SADD_pos) |
                          (static_cast<uint32_t>(dataSize) << NBYTES_pos) |
                          (0b1U << AUTOEND_pos) |
                          (0b1U << RD_WRN_pos)
                          );
            
            m_I2C->CR2 = cr2RegMask;
            m_I2C->CR2 |= (0b1U << START_pos);

            for(uint8_t i = 0; i < dataSize; i++) {
                while((m_I2C->ISR & (0b1U << RXNE_pos)) == 0) {}
                dataBuffer[i] = m_I2C->RXDR;
            }

            //wait for STOPF flag
            while((m_I2C->ISR & (0b1U << STOPF_pos)) == 0) {}

            //clear STOPCF flag
            m_I2C->ICR |= (0b1U << STOPCF_pos);
        }
};

#endif