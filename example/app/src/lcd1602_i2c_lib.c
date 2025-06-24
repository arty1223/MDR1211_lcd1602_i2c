/*
 * lcd1602_i2c_lib.c
 *
 *  Библиотека для китайских дисплеев
 *  Created on: Nov 24, 2020
 *      Authors: Oleg Volkov & Konstantin Golinskiy
 *      Создать свой символ: https://www.quinapalus.com/hd44780udg.html
 *  Modified on: Jun 23, 2025
 *      Authors: Artyom Skibitskiy
 *      Поддержка К1986ВЕ92FI и F1I. Перед использованием ознакомится с примером и readme.md.
 * 		Функция HAL_delay() заменена на delay() при помощи таймера systick. Встроенного аналога SPL не предоставляет.
 * 		Функция HAL_I2C_Master_Transmit() заменена на transmitI2C() использующую SPL функции. Встроенного аналога SPL не предоставляет.
 * 		В остальном оригинальный код остался без изменений.
 */

#include "lcd1602_i2c_lib.h"
/*-----------------------------------Настройки----------------------------------*/

#define Adress 0x27 << 1 // Адрес устройства.
bool backlight = true;	 // Начальная установка для подсветки вкл/выкл.
char lcd1602_tx_buffer[40] = {
	0,
}; // глобальный буфер данных. В него записывается текст.
uint8_t global_buffer = 0; // глобальная переменная байта данных, отправляемая дисплею.
/*-----------------------------------Настройки----------------------------------*/

/*============================Вспомогательные функции============================*/
/*-------------Функция для отправки данных при инициализации дисплея-------------*/
/// Функция предназначена для отправки байта данных по шине i2c. Содержит в себе delay. Без него инициализация дисплея не проходит.
/// \param *init_Data - байт, например 0x25, где 2 (0010) это DB7-DB4 или DB3-DB0, а 5(0101) это сигналы LED, E, RW, RS соответственно

void initI2C()
{
	RST_CLK_PCLKcmd((RST_CLK_PCLK_PORTC | RST_CLK_PCLK_I2C), ENABLE);
	I2C_InitTypeDef I2C_InitStruct; // Структуры для настройки переферии.
	PORT_InitTypeDef PortInit;
	/* Настройка пинов PORTC 0,1 (I2C_SCL,I2C_SDA) другие выводы - см. документацию*/
	PORT_StructInit(&PortInit);
	PortInit.PORT_Pin = (PORT_Pin_0 | PORT_Pin_1);
	PortInit.PORT_FUNC = PORT_FUNC_ALTER;
	PortInit.PORT_SPEED = PORT_SPEED_MAXFAST;
	PortInit.PORT_MODE = PORT_MODE_DIGITAL;
	PortInit.PORT_PULL_UP = PORT_PULL_UP_ON;
	PortInit.PORT_PD = PORT_PD_OPEN;
	PORT_Init(MDR_PORTC, &PortInit);

	/* Включение I2C */
	I2C_Cmd(ENABLE);

	/* Заполняем структуру для I2C с нужными нам параметрами */
	I2C_StructInit(&I2C_InitStruct);
	I2C_InitStruct.I2C_ClkDiv = 39;
	I2C_InitStruct.I2C_Speed = I2C_SPEED_UP_TO_400KHz;
	I2C_Init(&I2C_InitStruct);
}

typedef enum
{
	I2C_OK = 0x00U,
	I2C_ERROR = 0x01U,
	I2C_BUSY = 0x02U,
	I2C_ACK_TIMEOUT = 0x03U,
	I2C_nTRANS_TIMEOUT = 0x04U
} I2C_Status;

// адрес, байт данных, таймаут на всю передачу.
static I2C_Status transmitI2C(uint8_t adr, uint8_t data, uint32_t timeout)
{
	uint32_t tickstart = GetTick();

	while (I2C_GetFlagStatus(I2C_FLAG_BUS_FREE) != SET)
	{ // ждем освобождения линии в течении 25 мсек
		if (GetTick() - tickstart >= 25)
		{
			return I2C_BUSY;
		}
	}
	I2C_Send7bitAddress(adr, I2C_Direction_Transmitter);
	while (I2C_GetFlagStatus(I2C_FLAG_nTRANS) != SET)
	{
		if (GetTick() - tickstart >= timeout)
		{
			return I2C_nTRANS_TIMEOUT;
		}
	}
	while (I2C_GetFlagStatus(I2C_FLAG_SLAVE_ACK) != SET)
	{
		if (GetTick() - tickstart >= timeout)
		{
			return I2C_ACK_TIMEOUT;
		}
	}

	I2C_SendByte(data);
	while (I2C_GetFlagStatus(I2C_FLAG_nTRANS) != SET)
	{
		if (GetTick() - tickstart >= timeout)
		{
			return I2C_nTRANS_TIMEOUT;
		}
	}
	while (I2C_GetFlagStatus(I2C_FLAG_SLAVE_ACK) != SET)
	{
		if (GetTick() - tickstart >= timeout)
		{
			return I2C_ACK_TIMEOUT;
		}
	}

	I2C_SendSTOP();
	return I2C_OK;
}
static void lcd1602_Send_init_Data(uint8_t *init_Data)
{
	if (backlight)
	{
		*init_Data |= 0x08; // Включить подсветку
	}
	else
	{
		*init_Data &= ~0x08; // Выключить подсветку
	}
	*init_Data |= 0x04; // Устанавливаем стробирующий сигнал E в 1

	transmitI2C(Adress, *init_Data, 10);
	delay(5);
	*init_Data &= ~0x04; // Устанавливаем стробирующий сигнал E в 0
	transmitI2C(Adress, *init_Data, 10);
	delay(5);
}
/*-------------Функция для отправки данных при инициализации дисплея-------------*/

/*--------------------Функция отправки байта информации на дисплей---------------*/
/// Функция отправки байта информации на дисплей
/// \param Data - Байт данныйх
static void lcd1602_Write_byte(uint8_t Data)
{
	transmitI2C(Adress, Data, 10);
}
/*--------------------Функция отправки байта информации на дисплей---------------*/

/*----------------------Функция отправки пол байта информации--------------------*/
/// Функция отправки пол байта информации
/// \*param Data - байт данных
static void lcd1602_Send_cmd(uint8_t Data)
{
	Data <<= 4;
	lcd1602_Write_byte(global_buffer |= 0x04);	// Устанавливаем стробирующий сигнал E в 1
	lcd1602_Write_byte(global_buffer | Data);	// Отправляем в дисплей полученный и сдвинутый байт
	lcd1602_Write_byte(global_buffer &= ~0x04); // Устанавливаем стробирующий сигнал E в 0.
}
/*----------------------Функция отправки пол байта информации--------------------*/

/*----------------------Функция отправки байта данных----------------------------*/
/// Функция отправки байта данных на дисплей
/// \param Data - байт данных
/// \param mode - отправка команды. 1 - RW = 1(отправка данных). 0 - RW = 0(отправка команды).
static void lcd1602_Send_data_symbol(uint8_t Data, uint8_t mode)
{
	if (mode == 0)
	{
		lcd1602_Write_byte(global_buffer &= ~0x01); // RS = 0
	}
	else
	{
		lcd1602_Write_byte(global_buffer |= 0x01); // RS = 1
	}
	uint8_t MSB_Data = 0;
	MSB_Data = Data >> 4;		// Сдвигаем полученный байт на 4 позичии и записываем в переменную
	lcd1602_Send_cmd(MSB_Data); // Отправляем первые 4 бита полученного байта
	lcd1602_Send_cmd(Data);		// Отправляем последние 4 бита полученного байта
}
/*----------------------Функция отправки байта данных----------------------------*/

/*----------------------Основная функция для отправки данных---------------------*/
/// Функция предназначена для отправки байта данных по шине i2c
/// \param *init_Data - байт, например 0x25, где 2 (0010) это DB7-DB4 или DB3-DB0, а 5(0101) это сигналы LED, E, RW, RS соответственно
static void lcd1602_Send_data(uint8_t *Data)
{

	if (backlight)
	{
		*Data |= 0x08;
	}
	else
	{
		*Data &= ~0x08;
	}
	*Data |= 0x04; // устанавливаем стробирующий сигнал E в 1
	transmitI2C(Adress, *Data, 10);
	*Data &= ~0x04; // устанавливаем стробирующий сигнал E в 0
	transmitI2C(Adress, *Data, 10);
}

/*----------------------Основная функция для отправки данных---------------------*/
/*============================Вспомогательные функции============================*/

/*-------------------------Функция инициализации дисплея-------------------------*/
/// Функция инициализации дисплея
void lcd1602_Init(void)
{
	/*========Power on========*/
	uint8_t tx_buffer = 0x30;
	/*========Wait for more than 15 ms after Vcc rises to 4.5V========*/
	delay(15);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Wait for more 4.1 ms========*/
	delay(5);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Wait for more 100 microsec========*/
	delay(1);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);

	/*========Включаем 4х-битный интерфейс========*/
	tx_buffer = 0x20;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Включаем 4х-битный интерфейс========*/

	/*======2 строки, шрифт 5х8======*/
	tx_buffer = 0x20;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0xC0;
	lcd1602_Send_init_Data(&tx_buffer);
	/*======2 строки, шрифт 5х8======*/

	/*========Выключить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x80;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Выключить дисплей========*/

	/*========Очистить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x10;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Очистить дисплей========*/

	/*========Режим сдвига курсора========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x30;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Режим сдвига курсора========*/

	/*========Инициализация завершена. Включить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0xC0;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Инициализация завершена. Включить дисплей========*/
}



/// Функция поддержания дисплея
void lcd1602_keep_alive(void)
{
	// /*========Power on========*/
	uint8_t tx_buffer = 0x30;
	/*========Wait for more than 15 ms after Vcc rises to 4.5V========*/
	delay(15);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Wait for more 4.1 ms========*/
	delay(5);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Wait for more 100 microsec========*/
	delay(1);
	/*========BF can not be checked before this instruction.========*/
	/*========Function set ( Interface is 8 bits long.========*/
	lcd1602_Send_init_Data(&tx_buffer);;

	/*========Включаем 4х-битный интерфейс========*/
	tx_buffer = 0x20;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Включаем 4х-битный интерфейс========*/

	/*======2 строки, шрифт 5х8======*/
	tx_buffer = 0x20;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0xC0;
	lcd1602_Send_init_Data(&tx_buffer);
	/*======2 строки, шрифт 5х8======*/

	/*========Выключить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x80;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Выключить дисплей========*/	

	/*========Инициализация завершена. Включить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0xC0;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Инициализация завершена. Включить дисплей========*/
	
	/*========Очистить дисплей========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x10;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Очистить дисплей========*/

	/*========Режим сдвига курсора========*/
	tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x30;
	lcd1602_Send_init_Data(&tx_buffer);
	/*========Режим сдвига курсора========*/
}



/*-------------------------Функция инициализации дисплея-------------------------*/

/*-------------------------Функция вывода символа на дисплей---------------------*/
/// Функция вывода символа на дисплей
/// \param* symbol - символ в кодировке utf-8
void lcd1602_Print_symbol(uint8_t symbol)
{
	uint8_t command;
	command = ((symbol & 0xf0) | 0x09); // Формирование верхнего полубайта в команду для дисплея
	lcd1602_Send_data(&command);
	command = ((symbol & 0x0f) << 4) | 0x09; // Формирование нижнего полубайта в команду для дисплея
	lcd1602_Send_data(&command);
}
/*-------------------------Функция вывода символа на дисплей---------------------*/

/*-------------------------Функция вывода текста на дисплей----------------------*/
/// Функция вывода символа на дисплей
/// \param *message - массив, который отправляем на дисплей.
/// Максимальная длина сообщения - 40 символов.
void lcd1602_Print_text(char *message)
{
	for (int i = 0; i < strlen(message); i++)
	{
		lcd1602_Print_symbol(message[i]);
	}
}
/*-------------------------Функция вывода текста на дисплей----------------------*/

/*-------------------Функция установки курсора для вывода текста----------------*/
/// Функция установки курсора для вывода текста на дисплей
/// \param x - координата по оси x. от 0 до 39.
/// \param y - координата по оси y. от 0 до 3.
/// Видимая область:
/// Для дисплеев 1602 max x = 15, max y = 1.
/// Для дисплеев 2004 max x = 19, max y = 3.
void lcd1602_SetCursor(uint8_t x, uint8_t y)
{
	uint8_t command, adr;
	if (y > 3)
		y = 3;
	if (x > 39)
		x = 39;
	if (y == 0)
	{
		adr = x;
	}
	if (y == 1)
	{
		adr = x + 0x40;
	}
	if (y == 2)
	{
		adr = x + 0x14;
	}
	if (y == 3)
	{
		adr = x + 0x54;
	}
	command = ((adr & 0xf0) | 0x80);
	lcd1602_Send_data(&command);

	command = (adr << 4);
	lcd1602_Send_data(&command);
}
/*-------------------Функция установки курсора для вывода текста----------------*/

/*------------------------Функция перемещения текста влево-----------------------*/
/// Функция перемещения текста влево
/// Если ее повторять с периодичностью, получится бегущая строка
void lcd1602_Move_to_the_left(void)
{
	uint8_t command;
	command = 0x18;
	lcd1602_Send_data(&command);

	command = 0x88;
	lcd1602_Send_data(&command);
}
/*------------------------Функция перемещения текста влево-----------------------*/

/*------------------------Функция перемещения текста вправо----------------------*/
/// Функция перемещения текста вправо
/// Если ее повторять с периодичностью, получится бегущая строка
void lcd1602_Move_to_the_right(void)
{
	uint8_t command;
	command = 0x18;
	lcd1602_Send_data(&command);

	command = 0xC8;
	lcd1602_Send_data(&command);
}
/*------------------------Функция перемещения текста вправо----------------------*/

/*---------------------Функция включения/выключения подсветки--------------------*/
/// Булевая функция включения/выключения подсветки
/// \param state - состояние подсветки.
/// 1 - вкл. 0 - выкл.
void lcd1602_Backlight(bool state)
{
	if (state)
	{
		backlight = true;
	}
	else
	{
		backlight = false;
	}
}
/*---------------------Функция включения/выключения подсветки--------------------*/

/*---------------------Функция создания своего символа-------------------------- */
/// Функция создания своего собственного символа и запись его в память.
/// \param *my_Symbol - массив с символом
/// \param memory_adress - номер ячейки: от 1 до 8. Всего 8 ячеек.
void lcd1602_Create_symbol(uint8_t *my_Symbol, uint8_t memory_adress)
{
	lcd1602_Send_data_symbol(((memory_adress * 8) | 0x40), 0);
	for (uint8_t i = 0; i < 8; i++)
	{
		lcd1602_Send_data_symbol(my_Symbol[i], 1); // Записываем данные побайтово в память
	}
}
/*---------------------Функция создания своего символа-------------------------- */

/*-------------------------Функция очистки дисплея-------------------------------*/

void lcd1602_Clean(void)
{
	/// Аппаратная функция очистки дисплея.
	/// Удаляет весь текст, возвращает курсор в начальное положение.
	uint8_t tx_buffer = 0x00;
	lcd1602_Send_init_Data(&tx_buffer);
	tx_buffer = 0x10;
	lcd1602_Send_init_Data(&tx_buffer);
}
/*-------------------------Функция очистки дисплея-------------------------------*/

void lcd1602_Clean_Text(void)
{
	/// Альтернативная функция очистки дисплея
	/// Заполняет все поле памяти пробелами
	/// Работает быстрее, чем lcd1602_Clean, но в отличии от нее не возвращает курсор в начальное положение
	lcd1602_SetCursor(0, 0);
	lcd1602_Print_text("                                        ");
	lcd1602_SetCursor(0, 1);
	lcd1602_Print_text("                                        ");
}
