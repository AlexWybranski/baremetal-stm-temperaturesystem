#ifndef IWDG_HPP
#define IWDG_HPP
#include <cstdint>

struct IWDG_regs {
    volatile uint32_t KR;
    volatile uint32_t PR;
    volatile uint32_t RLR;
    volatile uint32_t SR;
    volatile uint32_t WINR;
};


template<uint32_t baseAddr>
class IwdgHandle {
    private:
        IWDG_regs* const m_IWDG;

        static constexpr uint32_t enableValue       = 0x0000CCCCU;
        static constexpr uint32_t modifyValue       = 0x00005555U;
        static constexpr uint32_t feedValue         = 0x0000AAAAU;

        static constexpr uint32_t prescalerValue    = 0b011U;
        static constexpr uint32_t reloadValue       = 4000U;

    public:
        IwdgHandle() : m_IWDG(reinterpret_cast<IWDG_regs*>(baseAddr)) {}
        ~IwdgHandle() = default;

        void setup() {
            m_IWDG->KR = enableValue;
            m_IWDG->KR = modifyValue;
            m_IWDG->PR |= (prescalerValue << 0U);
            m_IWDG->RLR |= (reloadValue << 0U);

            while(m_IWDG->SR != 0) {}

            m_IWDG->KR = feedValue;
        }

        void feedWatchdog() {
            m_IWDG->KR = feedValue;
        }
};

#endif