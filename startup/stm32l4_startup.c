#include <stdint.h>

#define SRAM_START     0x20000000U
#define SRAM_SIZE      (96*1024)
#define SRAM_END       ((SRAM_START) + (SRAM_SIZE))

#define STACK_START     SRAM_END

extern uint32_t __text_end__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t _sidata;

int main(void);
void __libc_init_array(void);

void Default_Handler(void) {
    while(1) {

    }
}
void Reset_Handler(void);
void NMI_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void SVCall_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void Debug_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)             __attribute__((weak, alias("Default_Handler")));
    
void WWDG_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void PVD_PVM_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void RTC_TAMP_STAMP_Handler(void)      __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void FLASH_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void RCC_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void EXTI0_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void EXTI1_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void EXTI2_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void EXTI3_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void EXTI4_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void DMA1_CH1_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH2_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH3_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH4_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH5_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH6_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA1_CH7_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void ADC1_2_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void CAN1_TX_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void CAN1_RX0_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM2_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void TIM3_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void TIM4_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void SPI1_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void SPI2_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void USART1_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void USART2_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void USART3_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void RTC_ALARM_Handler(void)           __attribute__((weak, alias("Default_Handler")));
void DFSDM1_FLT3_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void TIM8_BRK_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void TIM8_UP_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM8_TRG_COM_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void TIM8_CC_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void ADC3_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void FMC_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void SDMMC1_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void TIM5_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void SPI3_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void UART4_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void UART5_Handler(void)               __attribute__((weak, alias("Default_Handler")));
void TIM6_DACUNDER_Handler(void)       __attribute__((weak, alias("Default_Handler")));
void TIM7_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void DMA2_CH1_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA2_CH2_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA2_CH3_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA2_CH4_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA2_CH5_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DFSDM1_FLT0_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void DFSDM1_FLT1_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void DFSDM1_FLT2_Handler(void)         __attribute__((weak, alias("Default_Handler")));
void COMP_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void LPTIM1_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void LPTIM2_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void OTG_FS_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void DMA2_CH6_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void DMA2_CH7_Handler(void)            __attribute__((weak, alias("Default_Handler")));
void LPUART1_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void QUADSPI_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_Handler(void)             __attribute__((weak, alias("Default_Handler")));
void SAI1_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void SAI2_Handler(void)                __attribute__((weak, alias("Default_Handler")));
void SWPMI1_Handler(void)              __attribute__((weak, alias("Default_Handler")));
void TSC_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void LCD_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void AES_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void RNG_and_HASH_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void FPU_Handler(void)                 __attribute__((weak, alias("Default_Handler")));
void HASH_and_CRS_Handler(void)        __attribute__((weak, alias("Default_Handler")));


uint32_t vector_table[] __attribute__((section (".vector_table"))) = {
    STACK_START,
    (uint32_t)(uintptr_t)&Reset_Handler,
    (uint32_t)(uintptr_t)&NMI_Handler,
    (uint32_t)(uintptr_t)&HardFault_Handler,
    (uint32_t)(uintptr_t)&MemManage_Handler,
    (uint32_t)(uintptr_t)&BusFault_Handler,
    (uint32_t)(uintptr_t)&UsageFault_Handler,
    0,
    0,
    0,
    0,
    (uint32_t)(uintptr_t)&SVCall_Handler,
    (uint32_t)(uintptr_t)&Debug_Handler,
    0,
    (uint32_t)(uintptr_t)&PendSV_Handler,
    (uint32_t)(uintptr_t)&SysTick_Handler,
    (uint32_t)(uintptr_t)&WWDG_Handler,
    (uint32_t)(uintptr_t)&PVD_PVM_Handler,
    (uint32_t)(uintptr_t)&RTC_TAMP_STAMP_Handler,
    (uint32_t)(uintptr_t)&RTC_WKUP_Handler,
    (uint32_t)(uintptr_t)&FLASH_Handler,
    (uint32_t)(uintptr_t)&RCC_Handler,
    (uint32_t)(uintptr_t)&EXTI0_Handler,
    (uint32_t)(uintptr_t)&EXTI1_Handler,
    (uint32_t)(uintptr_t)&EXTI2_Handler,
    (uint32_t)(uintptr_t)&EXTI3_Handler,
    (uint32_t)(uintptr_t)&EXTI4_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH1_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH2_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH3_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH4_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH5_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH6_Handler,
    (uint32_t)(uintptr_t)&DMA1_CH7_Handler,
    (uint32_t)(uintptr_t)&ADC1_2_Handler,
    (uint32_t)(uintptr_t)&CAN1_TX_Handler,
    (uint32_t)(uintptr_t)&CAN1_RX0_Handler,
    (uint32_t)(uintptr_t)&CAN1_RX1_Handler,
    (uint32_t)(uintptr_t)&CAN1_SCE_Handler,
    (uint32_t)(uintptr_t)&EXTI9_5_Handler,
    (uint32_t)(uintptr_t)&TIM1_BRK_Handler,
    (uint32_t)(uintptr_t)&TIM1_UP_Handler,
    (uint32_t)(uintptr_t)&TIM1_TRG_COM_Handler,
    (uint32_t)(uintptr_t)&TIM1_CC_Handler,
    (uint32_t)(uintptr_t)&TIM2_Handler,
    (uint32_t)(uintptr_t)&TIM3_Handler,
    (uint32_t)(uintptr_t)&TIM4_Handler,
    (uint32_t)(uintptr_t)&I2C1_EV_Handler,
    (uint32_t)(uintptr_t)&I2C1_ER_Handler,
    (uint32_t)(uintptr_t)&I2C2_EV_Handler,
    (uint32_t)(uintptr_t)&I2C2_ER_Handler,
    (uint32_t)(uintptr_t)&SPI1_Handler,
    (uint32_t)(uintptr_t)&SPI2_Handler,
    (uint32_t)(uintptr_t)&USART1_Handler,
    (uint32_t)(uintptr_t)&USART2_Handler,
    (uint32_t)(uintptr_t)&USART3_Handler,
    (uint32_t)(uintptr_t)&EXTI15_10_Handler,
    (uint32_t)(uintptr_t)&RTC_ALARM_Handler,
    (uint32_t)(uintptr_t)&DFSDM1_FLT3_Handler,
    (uint32_t)(uintptr_t)&TIM8_BRK_Handler,
    (uint32_t)(uintptr_t)&TIM8_UP_Handler,
    (uint32_t)(uintptr_t)&TIM8_TRG_COM_Handler,
    (uint32_t)(uintptr_t)&TIM8_CC_Handler,
    (uint32_t)(uintptr_t)&ADC3_Handler,
    (uint32_t)(uintptr_t)&FMC_Handler,
    (uint32_t)(uintptr_t)&SDMMC1_Handler,
    (uint32_t)(uintptr_t)&TIM5_Handler,
    (uint32_t)(uintptr_t)&SPI3_Handler,
    (uint32_t)(uintptr_t)&UART4_Handler,
    (uint32_t)(uintptr_t)&UART5_Handler,
    (uint32_t)(uintptr_t)&TIM6_DACUNDER_Handler,
    (uint32_t)(uintptr_t)&TIM7_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH1_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH2_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH3_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH4_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH5_Handler,
    (uint32_t)(uintptr_t)&DFSDM1_FLT0_Handler,
    (uint32_t)(uintptr_t)&DFSDM1_FLT1_Handler,
    (uint32_t)(uintptr_t)&DFSDM1_FLT2_Handler,
    (uint32_t)(uintptr_t)&COMP_Handler,
    (uint32_t)(uintptr_t)&LPTIM1_Handler,
    (uint32_t)(uintptr_t)&LPTIM2_Handler,
    (uint32_t)(uintptr_t)&OTG_FS_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH6_Handler,
    (uint32_t)(uintptr_t)&DMA2_CH7_Handler,
    (uint32_t)(uintptr_t)&LPUART1_Handler,
    (uint32_t)(uintptr_t)&QUADSPI_Handler,
    (uint32_t)(uintptr_t)&I2C3_EV_Handler,
    (uint32_t)(uintptr_t)&I2C3_ER_Handler,
    (uint32_t)(uintptr_t)&SAI1_Handler,
    (uint32_t)(uintptr_t)&SAI2_Handler,
    (uint32_t)(uintptr_t)&SWPMI1_Handler,
    (uint32_t)(uintptr_t)&TSC_Handler,
    (uint32_t)(uintptr_t)&LCD_Handler,
    (uint32_t)(uintptr_t)&AES_Handler,
    (uint32_t)(uintptr_t)&RNG_and_HASH_Handler,
    (uint32_t)(uintptr_t)&FPU_Handler,
    (uint32_t)(uintptr_t)&HASH_and_CRS_Handler
};

void Reset_Handler(void) {
    uint32_t size = (uint32_t)&__data_end__ - (uint32_t)&__data_start__;

    uint8_t *pDestination = (uint8_t*)&__data_start__;
    uint8_t *pSource = (uint8_t*)&_sidata;

    for (uint32_t i = 0; i < size; i++)
    {
        *pDestination++ = *pSource++; 
    }
    
    uint32_t bss_size = (uint32_t)&__bss_end__ - (uint32_t)&__bss_start__;
    pDestination = (uint8_t*)&__bss_start__;
    for (uint32_t i = 0; i < bss_size; i++)
    {
        *pDestination++ = 0;
    }

    __libc_init_array();

    main();
}