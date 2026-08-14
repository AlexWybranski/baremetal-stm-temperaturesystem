#ifndef USART_HPP
#define USART_HPP
#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"

namespace UsartBitPositions {
    //CR1
    static constexpr uint32_t CR1_UE         = (0b1U << 0);
    static constexpr uint32_t CR1_RE         = (0b1U << 2U);
    static constexpr uint32_t CR1_TE         = (0b1U << 3U);
    static constexpr uint32_t CR1_RXNEIE     = (0b1U << 5U);
    static constexpr uint32_t CR1_TCIE       = (0b1U << 6U);
    static constexpr uint32_t CR1_TXEIE      = (0b1U << 7U);
    
    //ISR
    static constexpr uint32_t ISR_ORE        = (0b1U << 3U);
    static constexpr uint32_t ISR_RXNE       = (0b1U << 5U);
    static constexpr uint32_t ISR_TCE        = (0b1U << 6U);
    static constexpr uint32_t ISR_TXE        = (0b1U << 7U);

    //ICR
    static constexpr uint32_t ICR_ORECF      = (0b1U << 3U);
    static constexpr uint32_t ICR_TCCF       = (0b1U << 6U);
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

        static constexpr char TAIL_CHARS[3] = {'\r', '\n', '\0'};

        char m_txBuffer[32]{};
        volatile uint8_t m_txBuffIt = 0;
        
        char m_rxBuffer[32]{};
        volatile size_t m_rxHead = 0;
        volatile size_t m_rxTail = 0;

        TaskHandle_t m_taskToNotify = nullptr;

    public:
        static UsartHandle<baseAddr>* instance;
        UsartHandle() : m_USART(reinterpret_cast<USART_regs*>(baseAddr)) {
            instance = this;
        }
        ~UsartHandle() = default;

        void setTaskToNotify() {
            m_taskToNotify = xTaskGetCurrentTaskHandle();
        }
        
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

        void transmitText(const char* text) {
            using namespace UsartBitPositions;
            for(uint8_t i = 0; i < 32; ++i) {
                m_txBuffer[i] = 0;
            }
            uint8_t it = 0;
            while(true) {
                if(it >= 29 || text[it] == '\0') {
                    m_txBuffer[it] = TAIL_CHARS[0];
                    m_txBuffer[it+1] = TAIL_CHARS[1];
                    m_txBuffer[it+2] = TAIL_CHARS[2];
                    break;
                }
                m_txBuffer[it] = text[it];
                it++;
            }

            m_USART->CR1 |= CR1_TXEIE;
        }

        bool takeByteFromRxBuffer(char& byte) {
            if(m_rxTail == m_rxHead) return false;

            byte = m_rxBuffer[m_rxTail];
            size_t tempVar = m_rxTail + 1;
            m_rxTail = tempVar;

            if(m_rxTail >= 32) m_rxTail = 0;

            return true;
        }
        
        void handleIRQ() {
            using namespace UsartBitPositions;
            if(m_USART->ISR & ISR_ORE) {
                m_USART->ICR |= ICR_ORECF;
            }

            if((m_USART->ISR & ISR_RXNE) && (m_USART->CR1 & CR1_RXNEIE)) {
                uint32_t rxHelper;
                rxHelper = m_USART->RDR;
                if (rxHelper == '\r') {
                    rxHelper = '\n';
                }
                
                m_rxBuffer[m_rxHead] = static_cast<char>(rxHelper);
                size_t tempVar = m_rxHead + 1;
                m_rxHead = tempVar;
                
                if(m_rxHead >= 32) {
                    m_rxHead = 0;
                }
                
                if(rxHelper == '\n') {
                    BaseType_t xHigherPriorityTaskHasWoken = pdFALSE;
                    vTaskNotifyGiveFromISR(m_taskToNotify, &xHigherPriorityTaskHasWoken);
                    portYIELD_FROM_ISR(xHigherPriorityTaskHasWoken);
                    return;
                }
                
                return;
            }

            if((m_USART->ISR & ISR_TXE) && (m_USART->CR1 & CR1_TXEIE)) {    
                m_USART->TDR = static_cast<uint8_t>(m_txBuffer[m_txBuffIt]);
                
                if(m_txBuffer[m_txBuffIt] == '\0') {
                    m_txBuffIt = 0;
                    m_USART->CR1 &= ~CR1_TXEIE;
                } else {
                    size_t tempVar = m_txBuffIt + 1;
                    m_txBuffIt = tempVar;
                }

                return;
            }
        }
};

template <uint32_t baseAddr>
UsartHandle<baseAddr>* UsartHandle<baseAddr>::instance = nullptr;


#endif