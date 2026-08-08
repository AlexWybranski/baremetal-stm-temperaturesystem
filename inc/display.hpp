#ifndef DISPLAY_HPP
#define DISPLAY_HPP
#include <cstdint>

template<typename GpioPort, void (*DelayFunc)(uint32_t)>
class DisplayDriver {
    private:
        GpioPort& gpio;

        static constexpr uint32_t DIG_1 = 0;
        static constexpr uint32_t DIG_2 = 1;
        static constexpr uint32_t DIG_3 = 2;
        static constexpr uint32_t DIG_4 = 3;
        static constexpr uint32_t SEG_A = 4;
        static constexpr uint32_t SEG_B = 5;
        static constexpr uint32_t SEG_C = 6;
        static constexpr uint32_t SEG_D = 7;
        static constexpr uint32_t SEG_E = 8;
        static constexpr uint32_t SEG_F = 9;
        static constexpr uint32_t SEG_G = 10;
        static constexpr uint32_t SEG_DP = 11;
        
        uint8_t tempDigits[4];

        static constexpr uint32_t ALL_DIG_MASK = ((1U << DIG_1) | (1U << DIG_2) | 
                                                 (1U << DIG_3) | (1U << DIG_4));

        static constexpr uint32_t ALL_SEG_MASK = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C) | (1U << SEG_D) |
                                                (1U << SEG_E) | (1U << SEG_F) |
                                                (1U << SEG_G) | (1U << SEG_DP));

        static constexpr uint32_t PIN_MASK_0 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C) | (1U << SEG_D) |
                                                (1U << SEG_E) | (1U << SEG_F));

        static constexpr uint32_t PIN_MASK_1 = ((1U << SEG_B) | (1U << SEG_C));

        static constexpr uint32_t PIN_MASK_2 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_G) | (1U << SEG_D) |
                                                (1U << SEG_E));

        static constexpr uint32_t PIN_MASK_3 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C) | (1U << SEG_G) |
                                                (1U << SEG_D));

        static constexpr uint32_t PIN_MASK_4 = ((1U << SEG_B) | (1U << SEG_C) |
                                                (1U << SEG_G) | (1U << SEG_F));

        static constexpr uint32_t PIN_MASK_5 = ((1U << SEG_A) | (1U << SEG_C) |
                                                (1U << SEG_D) | (1U << SEG_F) |
                                                (1U << SEG_G));

        static constexpr uint32_t PIN_MASK_6 = ((1U << SEG_A) | (1U << SEG_G) |
                                                (1U << SEG_C) | (1U << SEG_D) |
                                                (1U << SEG_E) | (1U << SEG_F));

        static constexpr uint32_t PIN_MASK_7 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C));

        static constexpr uint32_t PIN_MASK_8 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C) | (1U << SEG_D) |
                                                (1U << SEG_E) | (1U << SEG_F) |
                                                (1U << SEG_G));

        static constexpr uint32_t PIN_MASK_9 = ((1U << SEG_A) | (1U << SEG_B) |
                                                (1U << SEG_C) | (1U << SEG_D) |
                                                (1U << SEG_G) | (1U << SEG_F));  
                                                
        static constexpr uint32_t DIGIT_MAP[10] = {PIN_MASK_0, PIN_MASK_1, PIN_MASK_2, PIN_MASK_3, 
                                                   PIN_MASK_4, PIN_MASK_5, PIN_MASK_6, PIN_MASK_7,
                                                   PIN_MASK_8, PIN_MASK_9};

    public:
        DisplayDriver(GpioPort &gpio) : gpio(gpio) {}
        ~DisplayDriver() = default;

        void sliceTemp(int32_t temperature) {
            tempDigits[0] = static_cast<uint8_t>(temperature / 1000);
            tempDigits[1] = static_cast<uint8_t>((temperature / 100) % 10);
            tempDigits[2] = static_cast<uint8_t>((temperature / 10) % 10);
            tempDigits[3] = static_cast<uint8_t>(temperature % 10);
        }

        void displayTemp() {
            for(uint32_t pinNum = 0; pinNum < 4; ++pinNum) {
                uint8_t currentDig = tempDigits[pinNum];

                gpio.setMultiplePinsState(1, ALL_DIG_MASK); 

                gpio.setMultiplePinsState(0, ALL_SEG_MASK);

                uint32_t segmentPattern = DIGIT_MAP[currentDig];
                    
                if(pinNum == 1) {
                    segmentPattern |= (1U << SEG_DP); 
                }

                gpio.setMultiplePinsState(1, segmentPattern);
                
                gpio.setPinState(0, pinNum); 
 
                vTaskDelay(pdMS_TO_TICKS(2));
            }
            gpio.setMultiplePinsState(1, ALL_DIG_MASK); 
            gpio.setMultiplePinsState(0, ALL_SEG_MASK);
        }

        void testDisplay() {
            for(uint32_t pin = 0; pin<4; ++pin) {
                gpio.setMultiplePinsState(0, ALL_SEG_MASK);
                gpio.setPinState(1, pin);
                for(uint32_t seg = 4; seg < 12; ++seg) {
                    gpio.setPinState(1, seg);
                    gpio.setPinState(0, pin);

                    DelayFunc(100U);

                    gpio.setPinState(0, seg);
                    gpio.setPinState(1, pin);
                }
                for(uint32_t digit = 0; digit < 10; ++digit) {
                    gpio.setPinState(1, pin);
                    gpio.setMultiplePinsState(1, DIGIT_MAP[digit]);
                    gpio.setPinState(0, pin);
                    
                    DelayFunc(100U);
                    
                    gpio.setMultiplePinsState(0, ALL_SEG_MASK);
                    gpio.setPinState(1, pin);
                }
            }
        }
};

#endif