#ifndef GPIO_HPP
#define GPIO_HPP
#include <cstdint>

struct GPIO {
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

template<uint32_t base_addr>
class GpioPortHandle {
    private:
        enum class Mode {
            input = 0b00,
            output =  0b01,
            alternate = 0b10,
            analog = 0b11
        };

        GPIO* const m_GPIO; 

    public:
        void setPinMode(Mode mode, uint32_t pinNum) {
            const uint32_t shift = pinNum*2U;
            constexpr uint32_t resetValue = 0b11U;
            const uint32_t setValue = static_cast<uint32_t>(mode);

            m_GPIO->MODER &= ~(resetValue << shift);
            m_GPIO->MODER |= (setValue << shift);
        }
        void setPinState(bool state, uint32_t pinNum) {
            if(state) {
                m_GPIO->BSRR = (0b1U << pinNum);
            } else {
                m_GPIO->BRR = (0b1U << pinNum);
            }
        }
        void togglePin(uint32_t pinNum) {
            m_GPIO->ODR ^= (0b1U << pinNum);
        }
};

#endif