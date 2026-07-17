#ifndef GPIO_HPP
#define GPIO_HPP
#include <cstdint>

struct GPIO_regs {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
    volatile uint32_t BRR;
    volatile uint32_t ASCR;
};

template<uint32_t baseAddr>
class GpioPortHandle {
    private:
        GPIO_regs* const m_GPIO; 
    public:
        enum class Mode : uint32_t {
            input = 0b00U,
            output =  0b01U,
            alternate = 0b10U,
            analog = 0b11U
        };

        enum class Pull : uint32_t {
            none = 0b00U,
            pullup = 0b01U,
            pulldown = 0b10U
        };

        GpioPortHandle() : m_GPIO(reinterpret_cast<GPIO_regs*>(baseAddr)) {}
        ~GpioPortHandle() = default;

        void setPinMode(Mode mode, uint32_t pinNum) {
            const uint32_t shift = pinNum*2U;
            constexpr uint32_t resetValue = 0b11U;
            const uint32_t setValue = static_cast<uint32_t>(mode);

            m_GPIO->MODER &= ~(resetValue << shift);
            m_GPIO->MODER |= (setValue << shift);
        }

        void setPinType(bool isOpenDrain, uint32_t pinNum) {
            if(isOpenDrain) {
                m_GPIO->OTYPER |= (1U << pinNum);
            } else {
                m_GPIO->OTYPER &= ~(1U << pinNum);
            }
        }

        void setPinState(bool state, uint32_t pinNum) {
            if(state) {
                m_GPIO->BSRR = (0b1U << pinNum);
            } else {
                m_GPIO->BRR = (0b1U << pinNum);
            }
        }

        void setPinPullUp(Pull pull, uint32_t pinNum) {
            const uint32_t shift = pinNum*2U;
            constexpr uint32_t resetValue = 0b11U;
            const uint32_t setValue = static_cast<uint32_t>(pull);

            m_GPIO->PUPDR &= ~(resetValue << shift);
            m_GPIO->PUPDR |= (setValue << shift);
        }

        void setPinAlternateFunction(uint32_t function, uint32_t pinNum) {
            constexpr uint32_t resetValue = 0b1111U;
            const uint32_t setValue = function;
            
            if(pinNum >= 8) {
                const uint32_t shift = (pinNum - 8U)*4U;
                m_GPIO->AFRH &= ~(resetValue << shift);
                m_GPIO->AFRH |= (setValue << shift); 
            } else {
                const uint32_t shift = pinNum*4U;
                m_GPIO->AFRL &= ~(resetValue << shift);
                m_GPIO->AFRL |= (setValue << shift); 
            }
        }

        void togglePin(uint32_t pinNum) {
            m_GPIO->ODR ^= (0b1U << pinNum);
        }
};

#endif