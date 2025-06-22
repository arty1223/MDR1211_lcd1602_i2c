#include "systick.h"
uint64_t millis = 0;
void initSystick()
{
    SysTick->LOAD = 80000 - 1;
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk; // HCLK
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    NVIC_EnableIRQ(SysTick_IRQn);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

volatile void delay(uint32_t ms)
{
    uint64_t cur = millis;
    while (millis - cur < ms)
    {
        __NOP();
    }
}

uint64_t GetTick(){
    return millis;
}

void SysTick_Handler()
{
  millis++;
}