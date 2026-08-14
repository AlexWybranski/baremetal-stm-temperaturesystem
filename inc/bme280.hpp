#ifndef BME280_HPP
#define BME280_HPP
#include <cstdint>

template<typename I2cPort>
class BME280Handle {
    private:
        I2cPort& m_i2c;

        static constexpr uint8_t BME_ADDR = 0x76;
        static constexpr uint8_t CHIP_ID = 0xD0;
        static constexpr uint8_t TEMP_MSB = 0xFA;
        static constexpr uint8_t COMP_PAR = 0x88;
        static constexpr uint8_t CONFIG = 0xF5;
        static constexpr uint8_t CTRL_MEAS = 0xF4;

        uint16_t dig_T1 = 0; //two addresses 0x88 / 0x89
        int16_t dig_T2 = 0; // 0x8A / 0x8B
        int16_t dig_T3 = 0; // 0x8C / 0x8D
        
        int32_t t_fine = 0;
        
        int32_t temperature = 0;

        uint8_t m_bmeBuffer[8]{};

        int32_t compensateTemp(uint32_t temp_raw) {
            int32_t var1, var2, T;

            int32_t t1, t2, t3;

            t1 = static_cast<int32_t>(dig_T1);
            t2 = static_cast<int32_t>(dig_T2);
            t3 = static_cast<int32_t>(dig_T3);

            var1 = (((temp_raw >> 3U) - (t1 << 1U)) * t2) >> 11U;
            var2 = (((((temp_raw >> 4U) - t1) * ((temp_raw >> 4U) - t1)) >> 12U) * t3) >> 14U;

            t_fine = var1 + var2;

            T = (t_fine * 5 + 128) >> 8;
            return T;
        }
        
        public:
        explicit BME280Handle(I2cPort &i2c) : m_i2c(i2c) {}
        ~BME280Handle() = default;

        bool isAlive() {
            m_i2c.read(BME_ADDR, CHIP_ID, m_bmeBuffer, 1);
            if(m_bmeBuffer[0] != 0x60) {
                return false;
            } else {
                return true;
            }
        }
        
        void init() {
            m_i2c.read(BME_ADDR, COMP_PAR, m_bmeBuffer, 6);
            dig_T1 =    ((m_bmeBuffer[0] << 0) |
                        (m_bmeBuffer[1] << 8));

            dig_T2 =    ((m_bmeBuffer[2] << 0) |
                        (m_bmeBuffer[3] << 8));

            dig_T3 =    ((m_bmeBuffer[4] << 0) |
                        (m_bmeBuffer[5] << 8));

            m_i2c.read(BME_ADDR, CTRL_MEAS, m_bmeBuffer, 1);
            m_bmeBuffer[0] |= ((0b11U << 0U) | (0b001U << 5U));
            m_i2c.write(BME_ADDR, CTRL_MEAS, m_bmeBuffer[0]);
            m_bmeBuffer[0] = 0;
            m_i2c.read(BME_ADDR, CONFIG, m_bmeBuffer, 1);
            m_bmeBuffer[0] |= (0b011U << 5U);
            m_i2c.write(BME_ADDR, CONFIG, m_bmeBuffer[0]);
        }

        int32_t readTemp() {
            m_i2c.read(BME_ADDR, TEMP_MSB, m_bmeBuffer, 3);
            temperature = ((m_bmeBuffer[0] << 12U) | (m_bmeBuffer[1] << 4U) | (m_bmeBuffer[2] >> 4U));
            temperature = compensateTemp(temperature);
            if(temperature > 9999) {
                temperature = 9999;
            }
            if(temperature < 0) {
                temperature = 0;
            }
            return temperature;
        }
};


#endif