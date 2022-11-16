#ifndef __LCD9341_SPI_H
#define __LCD9341_SPI_H

#include "stm32f3xx_hal.h"
#include "stdlib.h"
#include "fonts.h"

#define	LCD9341_BLACK_16BIT_COLOR   0x0000
#define	LCD9341_BLUE_16BIT_COLOR    0x001F
#define	LCD9341_RED_16BIT_COLOR     0xF800
#define	LCD9341_GREEN_16BIT_COLOR   0x07E0
#define LCD9341_CYAN_16BIT_COLOR    0x07FF
#define LCD9341_MAGENTA_16BIT_COLOR 0xF81F
#define LCD9341_YELLOW_16BIT_COLOR  0xFFE0
#define LCD9341_WHITE_16BIT_COLOR   0xFFFF

#define BLACK_COLOR 		0x000u
#define WHITE_TEXT_COLOR 	0x9EF7u
#define RED_COLOR 			0x00F8u
#define DRED_COLOR 			0x0078u
#define LRED_COLOR 			0xEFFBu
#define GREEN_COLOR 		0xE007u
#define LGREEN_COLOR 		0xEF7Fu
#define DGREEN_COLOR 		0xE003u
#define BLUE_COLOR 			0x1F00u
#define LBLUE_COLOR 		0xFF03u
#define DBLUE_COLOR 		0x0F00u
#define LGREY_COLOR			0x14A5u
#define LLGREY_COLOR		0x9AD6u
#define DDGREY_COLOR 		0x4529u
#define DGREY_COLOR 		0x8A52u
#define GREY_COLOR 			0xEF7Bu
#define PURPLE_COLOR 		0x1FF8u
#define DPURPLE_COLOR		0x0F78u
#define LPURPLE_COLOR 		0xFFFBu
#define YELLOW_COLOR		0xE0FFu
#define	LYELLOW_COLOR		0xEFFFu
#define DYELLOW_COLOR		0xE07Bu

//--Command List-----------------------------------------------------
#define LCD9341_CMD_NO_OPERATION 	0x00U
#define LCD9341_CMD_SOFTWARE_RESET	0x01U
//#define LCD9341_CMD_READ_DISPLAY_ID						0x04U
//#define LCD9341_CMD_READ_DISPLAY_STATUS					0x09U
//#define LCD9341_CMD_READ_DISPLAY_POWER_MODE 			0x0AU
//#define LCD9341_CMD_READ_DISPLAY_MADCTL					0x0BU
#define LCD9341_CMD_READ_DISPLAY_PIXEL_FORMAT			0x0CU
#define LCD9341_CMD_READ_DISPLAY_IMAGE_FORMAT			0x0DU
//#define LCD9341_CMD_READ_DISPLAY_SIGNAL_MODE			0x0EU
//#define LCD9341_CMD_READ_DISPLAY_SELF_DIAGNOSTIC_RESULT	0x0FU
//#define LCD9341_CMD_ENTER_SLEEP_MODE		0x10U
#define LCD9341_CMD_SLEEP_OUT		 		0x11U
#define LCD9341_CMD_PARTIAL_MODE_ON			0x12U
#define LCD9341_CMD_NORMAL_DISPLAY_MODE_ON	0x13U
//#define LCD9341_CMD_DISPLAY_INVERSION_OFF	0x20U
//#define LCD9341_CMD_DISPLAY_INVERSION_ON	0x21U
//#define LCD9341_CMD_GAMMA_SET	0x26U
#define LCD9341_CMD_DISPLAY_OFF	0x28U
#define LCD9341_CMD_DISPLAY_ON	0x29U
#define LCD9341_CMD_COLUMN_ADDRESS_SET	0x2AU
#define LCD9341_CMD_PAGE_ADDRESS_SET	0x2BU
#define LCD9341_CMD_MEMORY_WRITE	0x2cU
#define LCD9341_CMD_COLOR_SET		0x2DU
#define LCD9341_CMD_MEMORY_READ		0x2EU
#define LCD9341_CMD_PARTIAL_AREA	0x30U
//#define LCD9341_CMD_VERTICAL_SCROLLING_DEFINITION 0x33U
//#define LCD9341_CMD_TEARING_EFFECT_LINE_OFF	0x34U
//#define LCD9341_CMD_TEARING_EFFECT_LINE_ON	0x35U
#define LCD9341_CMD_MEMORY_ACCESS_CONTROL	0x36U
//#define LCD9341_CMD_VERTICAL_SCROLLING_START_ADDRESS	0x37U
#define LCD9341_CMD_IDLE_MODE_OFF	0x38U
#define LCD9341_CMD_IDLE_MODE_ON	0x39U
#define LCD9341_CMD_PIXEL_FORMAT_SET	0x3AU
#define LCD9341_CMD_WRITE_MEMORY_CONTINUE	0x3CU
#define LCD9341_CMD_READ_MEMORY_CONTINUE	0x3EU
//#define LCD9341_CMD_SET_TEAR_SCANLINE	0x44U
//#define LCD9341_CMD_GET_SCANLINE		0x45U
//#define LCD9341_CMD_WRITE_DISPLAY_BRIGHTNESS	0x51U
//#define LCD9341_CMD_READ_DISPLAY_BRIGHTNESS		0x52U
//#define LCD9341_CMD_WRITE_CTRL_DISPLAY			0x53U
//#define LCD9341_CMD_READ_CTRL_DISPLAY			0x54U
//#define LCD9341_CMD_WRITE_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x55U
//#define LCD9341_CMD_READ_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x56U
//#define LCD9341_CMD_WRITE_CABC_MINIMUM_BRIGHTNESS				0x5EU
//#define LCD9341_CMD_READ_CABC_MINIMUM_BRIGHTNESS				0x5FU
//#define LCD9341_CMD_READ_ID1	0xDAU
//#define LCD9341_CMD_READ_ID2	0xDBU
//#define LCD9341_CMD_READ_ID3	0xDCU

//--Command List End-------------------------------------------------

//---Pins Control----------------------------------------------------
#define RESET_ACTIVE()	(HAL_GPIO_WritePin(RESET_port,	RESET_pin,	GPIO_PIN_RESET	))
#define RESET_IDLE()	(HAL_GPIO_WritePin(RESET_port,	RESET_pin,	GPIO_PIN_SET	))
#define CS_ACTIVE()		(HAL_GPIO_WritePin(CS_port,		CS_pin,		GPIO_PIN_RESET	))
#define CS_IDLE()		(HAL_GPIO_WritePin(CS_port,		CS_pin,		GPIO_PIN_SET	))
#define DC_COMMAND()	(HAL_GPIO_WritePin(DC_port,		DC_pin,		GPIO_PIN_RESET	))
#define DC_DATA()		(HAL_GPIO_WritePin(DC_port,		DC_pin,		GPIO_PIN_SET	))
#define CLK_RESET()		(HAL_GPIO_WritePin(CLK_port,	CLK_pin,	GPIO_PIN_RESET	))
#define CLK_SET()		(HAL_GPIO_WritePin(CLK_port,	CLK_pin,	GPIO_PIN_SET	))
//---Pins Control End-------------------------------------------------

#define MAX_X 0x00EF
#define MAX_Y 0x013F

#define MIN_DMA_BUFF_SIZE 8
//all the buffers under this size will be sent without dma.
//if this buffer to small

typedef enum{
	DMA_OFF = 0x00U,
	DMA_ON
}LCD9341_DMA_ModeTypeDef;

typedef enum{
	VERTICAL = 0x08U,
	HORISONTAL = 0x28,
	VERTICAL_H_FLIP = 0x48,
	HORISONTAL_H_FLIP = 0xA8,
	VERTICAL_V_FLIP = 0x88,
	HORISONTAL_V_FLIP = 0x68,
	VERTICAL_HV_FLIP = 0xC8,
	HORISONTAL_HV_FLIP = 0xE8
}LCD9341_MemoryAccessControl;

void LCD9341_setup(	SPI_HandleTypeDef* _hSPI,
					GPIO_TypeDef* _RESET_port,	uint16_t _RESET_pin,
					GPIO_TypeDef* _CS_port,		uint16_t _CS_pin,
					GPIO_TypeDef* _DC_port,		uint16_t _DC_pin,
					GPIO_TypeDef* _CLK_port,	uint16_t _CLK_pin);

void LCD9341_Power_ON();
void LCD9341_HW_reset(void);
void LCD9341_SendCommandByte(uint8_t cmd);
void LCD9341_SendDataByte(uint8_t dt);
void LCD9341_ReceiveDataByte(uint8_t *dt);
void LCD9341_ReceiveDataBuff(uint8_t *dt,uint16_t size);
void LCD9341_ReceiveDataBuffDMA(uint8_t *dt, uint16_t size);
void LCD9341_MemoryWrite(uint8_t *buff, uint32_t size);
void LCD9341_MemoryWriteBytes(uint8_t *buff, uint32_t buff_size, uint32_t amount);
void LCD9341_MemoryWriteContinue(uint8_t *buff, uint32_t size);
void LCD9341_MemoryWriteContinueDMA(uint8_t *buff, uint32_t size);
void LCD9341_MemoryWriteBytesDMA(uint8_t *buff, uint32_t size, uint32_t amount);
void LCD9341_MemoryRead(uint8_t *dt, uint16_t size);
void LCD9341_MemoryReadDMA(uint8_t *buff, uint16_t size);
void LCD9341_AlignCoordsPage(uint16_t  *a1, uint16_t *a2);
void LCD9341_AlignCoordsColumn(uint16_t  *a1, uint16_t *a2);
void LCD9341_AlignCoords(uint16_t* x1, uint16_t* y1, uint16_t* x2, uint16_t* y2);


void LCD9341_SetMemoryAccessControl(LCD9341_MemoryAccessControl rotation); //rotation, orientation, update direction
void LCD9341_SetPixelFormat16bit();
void LCD9341_SetPixelFormat18bit();
void LCD9341_SetColumnAddress(uint16_t sp, uint16_t ep);
void LCD9341_SetPageAddress(uint16_t sp, uint16_t ep);
void LCD9341_SetFullScreenAccess();
void LCD9341_SetRectAccess(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);


void LCD9341_FillScreen16bit(uint16_t color);
void LCD9341_DMAInterrupt(SPI_HandleTypeDef* hspi);
void LCD9341_FillRect16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD9341_FillRectFromBuff16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* buff, uint16_t buff_size);
void LCD9341_DrawPixel16bit(uint16_t x, uint16_t y, uint16_t color);
void LCD9341_DrawLine16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD9341_DrawLineLength16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t length, uint16_t color);
void LCD9341_DrawCircle16bit(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);
void LCD9341_DrawCircle16bit2(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);

//void LCD9341_PutChar16x11(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bgcolor);
void LCD9341_PutChar16bit(uint16_t x, uint16_t y, char c);
void LCD9341_PutChar16bitOverlay(uint16_t x, uint16_t y, char c);
void LCD9341_PutString16bit(uint16_t x, uint16_t y, char*c);
void LCD9341_PutString16bitOverlay(uint16_t x, uint16_t y, char*c);

void LCD9341_TextModeNextLine();
void LCD9341_TextModeNextPosition();
void LCD9341_TextModeSetPosition(uint16_t x, uint16_t y);
void LCD9341_TextModeInit(sFONT font, uint16_t textColor, uint16_t bgColor, uint8_t overlay);
void LCD9341_TextModePutChar(char c);
void LCD9341_TextModePutString(char* c);
void LCD9341_TextModePutInt(int i);
void LCD9341_TextModePutUInt(unsigned i);
void LCD9341_TextModeCls();
void LCD9341_TextModeTextColor(uint16_t textColor);
void LCD9341_TextModeBgColor(uint16_t bgColor);
void LCD9341_TextModeSetOverlay(uint8_t overlay);
void LCD9341_TextModeSetFont(sFONT font);
uint16_t LCD9341_Make16bitRGBColor(uint16_t r, uint16_t g, uint16_t b);
uint16_t LCD9341_TranslateColor18to16bit(uint16_t r,uint16_t g,uint16_t b);
void LCD9341_ReadRect(uint8_t *buff, uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2);

void LCD9341_RandomRect(int(rand)(void));
void LCD9341_RandomLine(int(rand)(void));
void LCD9341_RandomPixel(int(rand)(void));

#endif
