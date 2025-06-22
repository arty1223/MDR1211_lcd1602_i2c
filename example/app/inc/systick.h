#pragma once
#include "app.h"
extern uint64_t millis;
void initSystick(); //расчитано на 80 МГц
uint64_t GetTick(); // как в HAL
volatile void delay(uint32_t ms); // задержка в милисекундах
