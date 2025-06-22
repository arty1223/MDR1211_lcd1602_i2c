#pragma once
#include "app.h"

// на моей плате кварц стоит на 16мгц поэтому нужен такой костыль. если у вас 8 - закомментировать блок ниже.
#ifdef HSE_Value 
#undef HSE_Value
#define HSE_Value ((uint32_t)16000000) 
#endif


void CLK_Init_80_mhz(void);