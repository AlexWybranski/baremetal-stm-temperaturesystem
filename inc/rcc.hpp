#ifndef RCC_HPP
#define RCC_HPP
#include <cstdint>

struct RCC_regs {
    volatile uint32_t CR;            
    volatile uint32_t ICSCR;         
    volatile uint32_t CFGR;          
    volatile uint32_t PLLCFGR;       
    volatile uint32_t PLLSAI1CFGR;   
    volatile uint32_t PLLSAI2CFGR;   
    volatile uint32_t CIER;          
    volatile uint32_t CIFR;          
    volatile uint32_t CICR;          
    
    uint32_t          RESERVED_0;    

    volatile uint32_t AHB1RSTR;      
    volatile uint32_t AHB2RSTR;      
    volatile uint32_t AHB3RSTR;      
    
    uint32_t          RESERVED_1;    

    volatile uint32_t APB1RSTR1;     
    volatile uint32_t APB1RSTR2;     
    volatile uint32_t APB2RSTR;      
    
    uint32_t          RESERVED_2;    

    volatile uint32_t AHB1ENR;       
    volatile uint32_t AHB2ENR;       
    volatile uint32_t AHB3ENR;       
    
    uint32_t          RESERVED_3;    

    volatile uint32_t APB1ENR1;      
    volatile uint32_t APB1ENR2;      
    volatile uint32_t APB2ENR;       
    
    uint32_t          RESERVED_4;    

    volatile uint32_t AHB1SMENR;     
    volatile uint32_t AHB2SMENR;     
    volatile uint32_t AHB3SMENR;     
    
    uint32_t          RESERVED_5;    

    volatile uint32_t APB1SMENR1;    
    volatile uint32_t APB1SMENR2;    
    volatile uint32_t APB2SMENR;     
    
    uint32_t          RESERVED_6;    

    volatile uint32_t CCIPR;         
    
    uint32_t          RESERVED_7;    

    volatile uint32_t BDCR;          
    volatile uint32_t CSR;           
    volatile uint32_t CRRCR;         
    volatile uint32_t CCIPR2;        
};

template<uint32_t baseAddr>
class RccHandle {
    private:
        RCC_regs* const m_RCC; 
    public:
        RccHandle() : m_RCC(reinterpret_cast<RCC_regs*>(baseAddr)) {}
        ~RccHandle() = default;

        void enableGpioClock(uint32_t portOffset) {
            m_RCC->AHB2ENR |= (0b1U << portOffset);
        }

        void enableI2cClock() {

        }

        void enableUsartClock() {

        }
};

#endif