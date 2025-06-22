#include "clk.h"


void CLK_Init_80_mhz(void)
{
    RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
    RST_CLK_HSEconfig(RST_CLK_HSE_ON);
    if (RST_CLK_HSEstatus() == SUCCESS) /* Good HSE clock */
    {
        /* Select HSE clock as CPU_PLL input clock source */
        /* Set PLL multiplier to 10                       */
        RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul5);
        /* Enable CPU_PLL */
        RST_CLK_CPU_PLLcmd(ENABLE);
        if (RST_CLK_CPU_PLLstatus() == SUCCESS) /* Good CPU PLL */
        {
            /* Set CPU_C3_prescaler to 2 */
            // RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV2);
            /* Set CPU_C2_SEL to CPU_PLL output instead of CPU_C1 clock */
            RST_CLK_CPU_PLLuse(ENABLE);

            /* Setup internal DUcc voltage regulator work mode based on clock frequency */
           BKP_DUccMode(BKP_DUcc_upto_80MHz);
            /* Setup EEPROM access delay to 3: 10*HSE/2 = 80 > 25MHz */
            EEPROM_SetLatency(EEPROM_Latency_3);

            /* Select CPU_C3 clock on the CPU clock MUX */
            RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3);
            SystemCoreClockUpdate();
        }
    }

    // MDR_RST_CLK->HS_CONTROL |= 0b01; //включить HSE
    // while(MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_HSE_RDY != 1); //ждем запуска HSE

    // MDR_RST_CLK->CPU_CLOCK |= 0b10; // CPU_C1 - HSE    

    // MDR_RST_CLK->PLL_CONTROL |= 4 << RST_CLK_PLL_CONTROL_PLL_CPU_MUL_Pos | 1 << RST_CLK_PLL_CONTROL_PLL_CPU_ON_Pos; //включить PLL, множитель 4+1  
    // // MDR_RST_CLK->PLL_CONTROL |= 9 << RST_CLK_PLL_CONTROL_PLL_CPU_MUL_Pos | 1 << RST_CLK_PLL_CONTROL_PLL_CPU_ON_Pos; //включить PLL, множитель 9+1  
    // while(MDR_RST_CLK->CLOCK_STATUS & RST_CLK_CLOCK_STATUS_PLL_CPU_RDY_Pos != 1); //ждем запуска PLL_CPU
    // MDR_RST_CLK->CPU_CLOCK |= 0b100; // CPU_C2 - PLL_CPU

    // MDR_EEPROM->CMD &= ~EEPROM_CMD_DELAY_Msk | 0x18; // выставляем задержку EEPROM в 3 цикла
    // #define LDO_TRIM  *(uint8_t *) 0x08000FA0 + *(uint8_t *) 0x08000FA1 + *(uint8_t *) 0x08000FA2
    // MDR_BKP->REG_0E |= LDO_TRIM << BKP_REG_0E_TRIM_Pos;
    // MDR_BKP->REG_0E |= 0b111111; // Выбор режима работы встроенного регулятора напряжения частота более 80 МГц; + нагрузка 100 ом
    

    // MDR_RST_CLK->CPU_CLOCK |= 0b01 << RST_CLK_CPU_CLOCK_HCLK_SEL_Pos; // CPU_C3 - HCLK
}