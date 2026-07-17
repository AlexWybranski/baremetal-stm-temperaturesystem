#ifndef I2C_HPP
#define I2C_HPP
#include <cstdint>

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

        void i2cEnable() {
            m_I2C->CR1 |= (1U << 0);
        }

        void i2cSetTiming() {
            //values for specific bits from stm32l476 reference manual
            constexpr uint32_t SCLL = 0x13U << 0U;
            constexpr uint32_t SCLH = 0xFU << 8U;
            constexpr uint32_t SDADEL = 0x2U << 16U;
            constexpr uint32_t SCLDEL = 0x4U << 20U;
            constexpr uint32_t PRESC = 0x1U << 28U;

            constexpr uint32_t timingValue = (SCLL | SCLH | SDADEL | SCLDEL | PRESC);

            m_I2C->TIMINGR = timingValue;
        }
};

#endif