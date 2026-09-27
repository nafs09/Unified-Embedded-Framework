/// @file include/uef/uhal/target_config.h
/// @brief Supplies safe defaults for compile-time target capability switches.

#ifndef UEF_UHAL_TARGET_CONFIG_H
#define UEF_UHAL_TARGET_CONFIG_H

/* Target builds may override these values in their board configuration header. */
#ifndef UHAL_HAS_FPU
#  define UHAL_HAS_FPU 0
#endif
#ifndef UHAL_HAS_DOUBLE_FPU
#  define UHAL_HAS_DOUBLE_FPU 0
#endif
#ifndef UHAL_HAS_DSP_SIMD
#  define UHAL_HAS_DSP_SIMD 0
#endif
#ifndef UHAL_HAS_HW_DIV
#  define UHAL_HAS_HW_DIV 0
#endif
#ifndef UHAL_HAS_CMSIS_DSP
#  define UHAL_HAS_CMSIS_DSP 0
#endif
#ifndef UHAL_HAS_IQMATH
#  define UHAL_HAS_IQMATH 0
#endif
#ifndef UHAL_HAS_DCACHE
#  define UHAL_HAS_DCACHE 0
#endif
#ifndef UHAL_HAS_ICACHE
#  define UHAL_HAS_ICACHE 0
#endif
#ifndef UHAL_HAS_TCM
#  define UHAL_HAS_TCM 0
#endif
#ifndef UHAL_HAS_CCM
#  define UHAL_HAS_CCM 0
#endif
#ifndef UHAL_HAS_MPU
#  define UHAL_HAS_MPU 0
#endif
#ifndef UHAL_HAS_DWT
#  define UHAL_HAS_DWT 0
#endif
#ifndef UHAL_HAS_DMA
#  define UHAL_HAS_DMA 0
#endif
#ifndef UHAL_HAS_DMAMUX
#  define UHAL_HAS_DMAMUX 0
#endif
#ifndef UHAL_HAS_BDMA
#  define UHAL_HAS_BDMA 0
#endif
#ifndef UHAL_HAS_UART
#  define UHAL_HAS_UART 0
#endif
#ifndef UHAL_HAS_LPUART
#  define UHAL_HAS_LPUART 0
#endif
#ifndef UHAL_HAS_SPI
#  define UHAL_HAS_SPI 0
#endif
#ifndef UHAL_HAS_I2C
#  define UHAL_HAS_I2C 0
#endif
#ifndef UHAL_HAS_CAN
#  define UHAL_HAS_CAN 0
#endif
#ifndef UHAL_HAS_FDCAN
#  define UHAL_HAS_FDCAN 0
#endif
#ifndef UHAL_HAS_USB_FS
#  define UHAL_HAS_USB_FS 0
#endif
#ifndef UHAL_HAS_USB_HS
#  define UHAL_HAS_USB_HS 0
#endif
#ifndef UHAL_HAS_QSPI
#  define UHAL_HAS_QSPI 0
#endif
#ifndef UHAL_HAS_SDMMC
#  define UHAL_HAS_SDMMC 0
#endif
#ifndef UHAL_HAS_ETH
#  define UHAL_HAS_ETH 0
#endif
#ifndef UHAL_HAS_ADC
#  define UHAL_HAS_ADC 0
#endif
#ifndef UHAL_HAS_DAC
#  define UHAL_HAS_DAC 0
#endif
#ifndef UHAL_HAS_COMP
#  define UHAL_HAS_COMP 0
#endif
#ifndef UHAL_HAS_OPAMP
#  define UHAL_HAS_OPAMP 0
#endif
#ifndef UHAL_HAS_HRTIM
#  define UHAL_HAS_HRTIM 0
#endif
#ifndef UHAL_HAS_LPTIM
#  define UHAL_HAS_LPTIM 0
#endif
#ifndef UHAL_HAS_IWDG
#  define UHAL_HAS_IWDG 0
#endif
#ifndef UHAL_HAS_WWDG
#  define UHAL_HAS_WWDG 0
#endif
#ifndef UHAL_HAS_RTC
#  define UHAL_HAS_RTC 0
#endif
#ifndef UHAL_HAS_FLASH_WRITE
#  define UHAL_HAS_FLASH_WRITE 0
#endif
#ifndef UHAL_HAS_CRC
#  define UHAL_HAS_CRC 0
#endif
#ifndef UHAL_HAS_CORDIC
#  define UHAL_HAS_CORDIC 0
#endif
#ifndef UHAL_HAS_FMAC
#  define UHAL_HAS_FMAC 0
#endif
#ifndef UHAL_HOST_SIMULATION
#  define UHAL_HOST_SIMULATION 0
#endif

#ifndef UHAL_CACHE_LINE_SIZE
#  define UHAL_CACHE_LINE_SIZE 0U
#endif

#endif /* UEF_UHAL_TARGET_CONFIG_H */
