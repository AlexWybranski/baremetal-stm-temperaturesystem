#ifndef NVIC_HPP
#define NVIC_HPP
#include <cstdint>

//baseAddr of nvic 0xE000E000
template<uint32_t baseAddr>
class NvicHandler {
    private:

    public:
    NvicHandler() {}
    ~NvicHandler() = default;
    
        void enableIRQ(uint32_t irqNum) {
            if(irqNum > 82) return;
            
            volatile uint32_t* ISER;
            
            uint8_t offsetMultiplier = (irqNum >> 5);
                
            uint32_t offset = (0x100 + (0x04 * offsetMultiplier));
                
            ISER = reinterpret_cast<volatile uint32_t*>(baseAddr + offset); 
                
            const uint32_t shift = irqNum % 32;
                
            *ISER |= (0x1U << shift);
        }
        
        void setIRQpriority(uint32_t irqNum, uint8_t priority) {
            if(priority > 15 || irqNum > 82) return;
            volatile uint32_t* IPR;

            const uint8_t resetValue = 0b11111111U;

            uint8_t offsetMultiplier = (irqNum >> 2);

            uint32_t offset = (0x400 + (0x04 * offsetMultiplier));

            IPR = reinterpret_cast<volatile uint32_t*>(baseAddr + offset);

            uint32_t shift = (irqNum % 4) * 8;

            *IPR &= ~((static_cast<uint32_t>(resetValue)) << shift); 
            *IPR |= ((static_cast<uint32_t>(priority)) << (shift + 4));
        }
};

#endif