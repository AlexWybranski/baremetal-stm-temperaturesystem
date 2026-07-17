#ifndef USART_HPP
#define USART_HPP
#include <cstdint>

struct USART_regs {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
};

template<uint32_t baseAddr>
class UsartHandle {
    private:
        USART_regs* const m_USART;
    public:
        UsartHandle() : m_USART(reinterpret_cast<USART_regs*>(baseAddr)) {}
        ~UsartHandle() = default;
};

#endif