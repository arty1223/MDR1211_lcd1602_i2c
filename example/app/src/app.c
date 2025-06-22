#include "app.h"

extern char lcd1602_tx_buffer[40];
uint8_t flag = 0;
uint8_t procent;
uint64_t counter = 0;
uint32_t Delay = 100;
float counter_float = 0.0f;

uint16_t i = 0;
unsigned long T1, T2;

int main(void)
{
	RST_CLK_DeInit();
	CLK_Init_80_mhz(); //
	initSystick();

	initI2C();
	// delay(15);		// ждем пока запустится дисплей
	lcd1602_Init(); // инициализация дисплея.
	lcd1602_Clean();
	// lcd1602_Backlight(false);
	// Далее идет пример из оригинального репозитория без каких-либо изменений за исключением замены функций HAL на мои реализации.
	while (1)
	{
		if (millis - T1 >= Delay)
		{
			T1 = millis;
			if (flag == 0)
			{
				lcd1602_SetCursor(0, 0);
				if (procent < 25)
				{
					sprintf(lcd1602_tx_buffer, "Loading                          ");
					lcd1602_Print_text(lcd1602_tx_buffer);
				}
				else if (procent >= 25 && procent <= 50)
				{
					sprintf(lcd1602_tx_buffer, "Loading.                         ");
					lcd1602_Print_text(lcd1602_tx_buffer);
				}
				else if (procent >= 50 && procent <= 75)
				{
					sprintf(lcd1602_tx_buffer, "Loading..                        ");
					lcd1602_Print_text(lcd1602_tx_buffer);
				}
				else if (procent >= 75)
				{
					sprintf(lcd1602_tx_buffer, "Loading...                       ");
					lcd1602_Print_text(lcd1602_tx_buffer);
				}

				procent = procent + 4;
				sprintf(lcd1602_tx_buffer, "%d%c", procent, 0x25);
				lcd1602_SetCursor(10, 1);
				lcd1602_Print_text(lcd1602_tx_buffer);
				i++;
				if (i == 25)
				{
					Delay = 300;
					flag = 1;
					i = 0;
					lcd1602_Clean();
					lcd1602_Backlight(0);
				}
			}
			if (flag == 1)
			{
				lcd1602_SetCursor(0, 0);
				sprintf(lcd1602_tx_buffer, "Test display");
				lcd1602_Print_text(lcd1602_tx_buffer);
				i++;
				if (i > 5)
				{
					lcd1602_Backlight(1);
					if (i == 10)
					{
						flag = 2;
						i = 0;
						lcd1602_Clean();
					}
				}
			}

			if (flag == 2)
			{
				lcd1602_SetCursor(0, 0);
				sprintf(lcd1602_tx_buffer, "                Soldering iron");
				lcd1602_Print_text(lcd1602_tx_buffer);

				lcd1602_SetCursor(0, 1);
				sprintf(lcd1602_tx_buffer, "                How are you?)");
				lcd1602_Print_text(lcd1602_tx_buffer);
				lcd1602_Move_to_the_left();
				i++;
				if (i == 35)
				{
					flag = 3;
					i = 0;
					lcd1602_Clean();
				}
			}
			if (flag == 3)
			{
				i++;
				if (i < 5)
				{
					lcd1602_SetCursor(0, 0);
					sprintf(lcd1602_tx_buffer, "I feel good :)");
					lcd1602_Print_text(lcd1602_tx_buffer);
				}
				if (i >= 5 && i < 10)
				{
					lcd1602_Clean();
					lcd1602_SetCursor(0, 1);
					lcd1602_Print_text(lcd1602_tx_buffer);
				}
				else if (i >= 10 && i < 15)
				{
					flag = 4;
					i = 0;
					lcd1602_Clean();
					Delay = 200;
				}
			}
			if (flag == 4)
			{
				if (i < 14)
				{
					lcd1602_SetCursor(0, 0);
					lcd1602_Print_text(":)");
					lcd1602_Move_to_the_right();
					i++;
				}
				else if (i == 14)
				{
					lcd1602_Clean();
					lcd1602_SetCursor(14, 1);
					lcd1602_Print_text(":)");
					i++;
				}
				else if (i > 14)
				{
					lcd1602_Move_to_the_left();
					i++;
					if (i == 30)
					{
						flag = 5;
						i = 0;
						lcd1602_Clean();
						Delay = 100;
					}
				}
			}
			if (flag == 5)
			{
				lcd1602_Backlight(false);
				lcd1602_SetCursor(0, 0);
				sprintf(lcd1602_tx_buffer, "int = %d", GetTick() / 1000);
				lcd1602_Print_text(lcd1602_tx_buffer);
				
				// lcd1602_SetCursor(0, 0);
				// sprintf(lcd1602_tx_buffer, "int = %d", counter);
				// lcd1602_Print_text(lcd1602_tx_buffer);

				// lcd1602_SetCursor(0, 1);
				// sprintf(lcd1602_tx_buffer, "float = %.4f ", counter_float);
				// lcd1602_Print_text(lcd1602_tx_buffer);

				// lcd1602_SetCursor(0, 2);
				// sprintf(lcd1602_tx_buffer, "time = %d", GetTick() / 1000);
				// lcd1602_Print_text(lcd1602_tx_buffer);				
			}
			// if (millis - T2 >= 10)
			// {
			// 	T2 = millis;
			// 	counter++;
			// 	counter_float = counter_float + 0.0025f;
			// }
		}
	}
}
