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
        I2cHandle() : m_I2C(reinterpret_cast<I2C_regs*>(base_addr)) {}
        ~I2cHandle() = default;
};

#endif