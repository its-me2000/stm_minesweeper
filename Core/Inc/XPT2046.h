/*
 * XPT2046.h
 *
 *  Created on: Dec 7, 2020
 *      Author: Eugenijus
 */

#ifndef __XPT2046_H

#include "stm32f3xx_hal.h"
#include "stdlib.h"

#define __XPT2046_H

#define ITER 10
#define RX_TX_BUFF_SIZE (((ITER+1)*3*2)+1)
#define BUFF_X_OFFSET 3
#define BUFF_Y_OFFSET (BUFF_X_OFFSET + (ITER*2) + 2)
#define BUFF_Z1_OFFSET (BUFF_Y_OFFSET + (ITER*2) + 2)
#define DEBOUNCE_SAMPLES_COUNT 5
#define Timeout 1000

#define XPT2046_COMMAND_BIT 0b10000000u
#define XPT2046_GET_X		0b01010000u
#define XPT2046_GET_Y		0b00010000u
#define XPT2046_GET_Z1		0b00110000u
#define XPT2046_GET_Z2		0b01000000u
#define XPT2046_8BIT		0b00001000u
#define XPT2046_REF			0b00000100u
#define XPT2046_PD1			0b00000010u
#define XPT2046_PD2			0b00000001u
#define XPT2046_IRQ_OFF		0b00000011u

#define XPT2046_SF_SWAP   0b001u
#define XPT2046_SF_FLIP_X 0b100u
#define XPT2046_SF_FLIP_Y 0b010u

typedef enum{
	TS_VERTICAL 		  = 0, //0b001u,
	TS_HORISONTAL 		  = XPT2046_SF_SWAP,
	TS_VERTICAL_H_FLIP 	  =  				  XPT2046_SF_FLIP_X, //0b101u,
	TS_HORISONTAL_H_FLIP  = XPT2046_SF_SWAP | XPT2046_SF_FLIP_X, //0x100u,
	TS_VERTICAL_V_FLIP 	  =  									  XPT2046_SF_FLIP_Y, //0b011u,
	TS_HORISONTAL_V_FLIP  = XPT2046_SF_SWAP |					  XPT2046_SF_FLIP_Y, // 0b010u,
	TS_VERTICAL_HV_FLIP	  =  				  XPT2046_SF_FLIP_X | XPT2046_SF_FLIP_Y, //0x111u
	TS_HORISONTAL_HV_FLIP = XPT2046_SF_SWAP | XPT2046_SF_FLIP_X | XPT2046_SF_FLIP_Y, //0x110u
}XPT2046_Orientation;

typedef enum {
	UP, DOWN
} TouchEventType;

void XPT2046_setup(	SPI_HandleTypeDef* _hSPI,
					GPIO_TypeDef* _IRQ_port,	uint16_t _IRQ_pin,
					GPIO_TypeDef* _CS_port,		uint16_t _CS_pin,TIM_HandleTypeDef* _htim );
void XPT2046_SetSwapFlip(XPT2046_Orientation o);
void XPT2046_Start();
//uint16_t XPT2046_ReadData(uint8_t command);
void XPT2046_ReadRawData_DMA(TIM_HandleTypeDef *htim);
void XPT2046_ReadRawData_DMA_Complete(SPI_HandleTypeDef* hspi);
void XPT2046_NewRawEvent(TouchEventType event);



//int* XPT2046_GetCalCoeff();
void XPT2046_SetResolution(uint16_t resX,uint16_t resY);
void XPT2046_GetRawXY(uint16_t* x, uint16_t* y);
void XPT2046_GetCalibratedXY(uint16_t* x, uint16_t* y);
void XPT2046_CalibrationTouch(uint16_t x, uint16_t y);
void XPT2046_Calibrate(uint16_t* x, uint16_t* y);
void XPT2046_GetRawTouchEvent(uint16_t* x, uint16_t* y, TouchEventType event);
//void XPT2046_GetCalibratedTouchEvent(uint16_t* x, uint16_t* y, TouchEventType event);
void XPT2046_WaitForUpEvent();
uint8_t XPT2046_GetEvent(uint16_t* x, uint16_t* y, TouchEventType* event);
void XPT2046_NewRawEvent(TouchEventType event);
int perform_calibration();


int comp_uint16_t(const void *e1, const void *e2);
uint16_t swap_uint16_t(uint16_t n);
//uint16_t sqrt(uint32_t x);
//float diskr(float a, float b, float c);

#endif /* __XPT2046_H */

