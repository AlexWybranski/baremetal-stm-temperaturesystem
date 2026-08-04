#ifndef USART_HPP
#define USART_HPP
#include <cstdint>

namespace UsartBitPositions {
    //CR1
    static constexpr uint32_t CR1_UE = (0b1U << 0);
    static constexpr uint32_t CR1_RE = (0b1U << 2U);
    static constexpr uint32_t CR1_TE = (0b1U << 3U);
    static constexpr uint32_t CR1_RXNEIE = (0b1U << 5U);
    static constexpr uint32_t CR1_TCIE = (0b1U << 6U);
    static constexpr uint32_t CR1_TXEIE = (0b1U << 7U);
    
    //ISR
    static constexpr uint32_t ISR_RXNE = (0b1U << 5U);
    static constexpr uint32_t ISR_TCE = (0b1U << 6U);
    static constexpr uint32_t ISR_TXE = (0b1U << 7U);

    //ICR
    static constexpr uint32_t ICR_TCCF = (0b1U << 6U);
}

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

        static constexpr uint32_t ASCII_ALIGN = 48;
        static constexpr char RETURN[2] = {'\r', '\n'};

        char m_txBuffer[6];
        uint8_t m_txBuffIt = 0;
        
        char m_rxBuffer[32];
        size_t m_rxHead;
        size_t m_rxTail;

        const char* temperatureCommand = "get temp";
        static constexpr int tempCommandLength = 8;

    public:
        static UsartHandle<baseAddr>* instance;
        UsartHandle() : m_USART(reinterpret_cast<USART_regs*>(baseAddr)) {
            instance = this;
        }
        ~UsartHandle() = default;
        
        void setBaudRate(uint32_t clockSpeed, uint32_t baudRate) {
            uint32_t brrRegMask;
            brrRegMask = clockSpeed / baudRate;
            m_USART->BRR = brrRegMask;
        }
        
        //uart is set up to work in 8N1 mode (8 bit word, nonparity, 1 bit stop)
        void enable() {
            using namespace UsartBitPositions;
            uint32_t cr1RegMask;
            cr1RegMask = (CR1_RE | CR1_TE | CR1_RXNEIE);
            m_USART->CR1 = cr1RegMask;
            m_USART->CR1 |= CR1_UE; 
        }
        
        bool parseCommand(char* command) {
            for(int i = 0; i < tempCommandLength; ++i) {
                if(temperatureCommand[i] != command[i]) {
                    return false;
                }
            }
            return true;
        }

        void commandResponse(int32_t temperature) {
            using namespace UsartBitPositions;
            m_txBuffer[0] = static_cast<char>((temperature / 1000) + ASCII_ALIGN);
            m_txBuffer[1] = static_cast<char>(((temperature / 100) % 10) + ASCII_ALIGN);
            m_txBuffer[2] = static_cast<char>(((temperature / 10) % 10) + ASCII_ALIGN);
            m_txBuffer[3] = static_cast<char>((temperature % 10) + ASCII_ALIGN);
            m_txBuffer[4] = RETURN[0];
            m_txBuffer[5] = RETURN[1];

            m_USART->CR1 |= CR1_TXEIE;
        }

        bool takeByteFromRxBuffer(char& byte) {
            if(m_rxTail == m_rxHead) return false;

            byte = m_rxBuffer[m_rxTail];
            ++m_rxTail;

            if(m_rxTail >= 32) m_rxTail = 0;

            return true;
        }
        
        void handleIRQ() {
            using namespace UsartBitPositions;
            if((m_USART->ISR & ISR_RXNE) && (m_USART->CR1 & CR1_RXNEIE)) {
                m_rxBuffer[m_rxHead] = m_USART->RDR;
                ++m_rxHead;

                if(m_rxHead >= 32) {
                    m_rxHead = 0;
                }
                return;
            }

            if((m_USART->ISR & ISR_TXE) && (m_USART->CR1 & CR1_TXEIE)) {
                m_USART->TDR = m_txBuffer[m_txBuffIt];
                ++m_txBuffIt;

                if(m_txBuffIt >= 6) {
                    m_txBuffIt = 0;
                    m_USART->CR1 &= ~CR1_TXEIE;
                }

                return;
            }
        }
};

template <uint32_t baseAddr>
UsartHandle<baseAddr>* UsartHandle<baseAddr>::instance = nullptr;


#endif