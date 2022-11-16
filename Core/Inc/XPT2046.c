#include "XPT2046.h"
//#include <fastmath.h>

SPI_HandleTypeDef *XPT2046_hSPI;
TIM_HandleTypeDef *XPT2046_htim;

GPIO_TypeDef * XPT2046_IRQ_port;
GPIO_TypeDef * XPT2046_CS_port;

uint16_t XPT2046_IRQ_pin;
uint16_t XPT2046_CS_pin;


volatile static struct {
	uint16_t x;
	uint16_t y;
	uint16_t z1;
	//	uint16_t z2;
	TouchEventType event;
	//	uint8_t update;
	XPT2046_Orientation swapFlip; //0bxys   x-flipX, y-flipY, s - swapXY
	uint16_t resX;
	uint16_t resY;
}touchStatus;

volatile static struct {
	uint16_t x;
	uint16_t y;
	TouchEventType event;
	uint8_t update; // 0 - waiting for event, 1 - pendling event
} touchEvent;



volatile struct {
	int x[5], xfb[5];
	int y[5], yfb[5];
	int a[7];
} calibration;



static uint8_t rxBuff[RX_TX_BUFF_SIZE];
static uint8_t txBuff[RX_TX_BUFF_SIZE]={
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_X),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Y),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		(XPT2046_COMMAND_BIT | XPT2046_GET_Z1),0,
		0
};




void XPT2046_setup(	SPI_HandleTypeDef* _hSPI,
		GPIO_TypeDef* _IRQ_port,	uint16_t _IRQ_pin,
		GPIO_TypeDef* _CS_port,		uint16_t _CS_pin,TIM_HandleTypeDef* _htim ){
	XPT2046_hSPI = _hSPI;
	XPT2046_IRQ_port 	= _IRQ_port;
	XPT2046_IRQ_pin		= _IRQ_pin;
	XPT2046_CS_port		= _CS_port;
	XPT2046_CS_pin		= _CS_pin;
	XPT2046_htim		= _htim;

	touchStatus.x=0;
	touchStatus.y=0;
	touchStatus.z1=0;
	touchStatus.event=UP;
	touchStatus.swapFlip=TS_HORISONTAL;
	touchStatus.resX=320;
	touchStatus.resY=240;

	return;
}

void XPT2046_SetSwapFlip(XPT2046_Orientation o){
	touchStatus.swapFlip=o;
}

void XPT2046_SetResolution(uint16_t resX,uint16_t resY){
	touchStatus.resX=resX;
	touchStatus.resY=resY;
}

void XPT2046_Start(){
	HAL_TIM_Base_Start_IT(XPT2046_htim);
}

void XPT2046_ReadRawData_DMA(TIM_HandleTypeDef* htim){
	if(htim==XPT2046_htim){
		HAL_GPIO_WritePin(XPT2046_CS_port, XPT2046_CS_pin, GPIO_PIN_RESET);
		HAL_SPI_TransmitReceive_DMA(XPT2046_hSPI, txBuff, rxBuff, RX_TX_BUFF_SIZE);
	}
}

void XPT2046_ReadRawData_DMA_Complete(SPI_HandleTypeDef* hspi){
	if(hspi!=XPT2046_hSPI) return;

	static uint8_t debounceCounter = 0;
	uint16_t tempX,tempY,tempZ;
	static uint16_t results[ITER];

	HAL_GPIO_WritePin(XPT2046_CS_port, XPT2046_CS_pin, GPIO_PIN_SET);
	uint16_t* rawData = (uint16_t*)(rxBuff+BUFF_Z1_OFFSET);
	for (int i = 0; i<ITER;i++){
		results[i]=(swap_uint16_t((rawData[i])&0x7FFF)>>3);
	}
	qsort(results, ITER, sizeof(*results), comp_uint16_t);
	tempZ=results[ITER/2];




	switch(touchStatus.event){
	case UP:
		if(tempZ==0){
			if(debounceCounter>0){
				debounceCounter--;
			}
			//return;
		} else{
			debounceCounter++;
			if(debounceCounter >= DEBOUNCE_SAMPLES_COUNT){
				debounceCounter=0;


				rawData = (uint16_t*)(rxBuff+BUFF_X_OFFSET);


				for (int i = 0; i<ITER;i++){
					results[i]=(swap_uint16_t((rawData[i])&0x7FFF)>>3);
				}


				qsort(results, ITER, sizeof(*results), comp_uint16_t);
				tempX=results[ITER/2];

				rawData = (uint16_t*)(rxBuff+BUFF_Y_OFFSET);
				for (int i = 0; i<ITER;i++){
					results[i]=(swap_uint16_t((rawData[i])&0x7FFF)>>3);
				}
				qsort(results, ITER, sizeof(*results), comp_uint16_t);
				tempY=results[ITER/2];

				if(touchStatus.swapFlip & XPT2046_SF_SWAP){
					touchStatus.x=tempY;
					touchStatus.y=tempX;
				}
				else{
					touchStatus.x=tempX;
					touchStatus.y=tempY;
				}
				touchStatus.event=DOWN;

				XPT2046_NewRawEvent(DOWN); //callback on DOWN


			}
			return;
		}
		break;
	case DOWN:
		if(tempZ==0){
			debounceCounter++;
			if(debounceCounter>=DEBOUNCE_SAMPLES_COUNT){
				debounceCounter=0;
				touchStatus.event=UP;

				XPT2046_NewRawEvent(UP);//callback on UP
			}
		}else{
			if(debounceCounter>0){
				debounceCounter--;
			}

			rawData = (uint16_t*)(rxBuff+BUFF_X_OFFSET);

			for (int i = 0; i<ITER;i++){
				results[i]=(swap_uint16_t((rawData[i])&0x7FFF)>>3);
			}
			qsort(results, ITER, sizeof(*results), comp_uint16_t);
			tempX=results[ITER/2];

			rawData = (uint16_t*)(rxBuff+BUFF_Y_OFFSET);
			for (int i = 0; i<ITER;i++){
				results[i]=(swap_uint16_t((rawData[i])&0x7FFF)>>3);
			}
			qsort(results, ITER, sizeof(*results), comp_uint16_t);
			tempY=results[ITER/2];


			if(touchStatus.swapFlip & XPT2046_SF_SWAP){
				touchStatus.x=tempY;
				touchStatus.y=tempX;
			}
			else{
				touchStatus.x=tempX;
				touchStatus.y=tempY;
			}
		}
		break;

	default:
		break;
	}


	//	touchStatus.z1 =(uint16_t)((1400 * tempX / 4096.0) * ((4096.0 / tempZ) - 1) - 3850 * ((1-tempY) / 4096.0));
	/*
	touchStatus.x=tempX;
	touchStatus.y=tempY;
	touchStatus.z1=tempZ;
	 */
}

void XPT2046_NewRawEvent(TouchEventType event){
	if (touchEvent.update > 0){
		return;
	}
	uint16_t x,y;
	XPT2046_GetCalibratedXY(&x,&y);
	touchEvent.x=x;
	touchEvent.y=y;
	touchEvent.event=event;
	touchEvent.update=1;
}

void XPT2046_WaitForUpEvent(){
	uint8_t r;
	uint16_t x;
	uint16_t y;
	TouchEventType event;
	do{
		r=XPT2046_GetEvent(&x, &y, &event);
	}while(!r && event==DOWN);

}

uint8_t XPT2046_GetEvent(uint16_t* x, uint16_t* y, TouchEventType* event){
	if(touchEvent.update == 0){
		return 0;
	}else{
		*x = touchEvent.x;
		*y = touchEvent.y;
		*event = touchEvent.event;
		touchEvent.update = 0;
		return 1;
	}
}
/*
uint16_t XPT2046_ReadData(uint8_t command){
	uint16_t result;
	HAL_GPIO_WritePin(XPT2046_CS_port, XPT2046_CS_pin, GPIO_PIN_RESET);
	//HAL_Delay(1);
	HAL_SPI_Transmit(XPT2046_hSPI, &command, 1, Timeout);
	HAL_SPI_Receive(XPT2046_hSPI, (uint8_t*)&result, 2, Timeout);
	HAL_GPIO_WritePin(XPT2046_CS_port, XPT2046_CS_pin, GPIO_PIN_SET);
	return result;
}*/


void XPT2046_GetRawTouchEvent(uint16_t* x, uint16_t* y, TouchEventType event){

	while(touchStatus.event==event){}

	while(1){
		if(touchStatus.event==event){
			*x=touchStatus.x;
			*y=touchStatus.y;
			break;
		}
	}
}

void XPT2046_CalibrationTouch(uint16_t x, uint16_t y){

	static uint16_t i= 0;

	uint16_t tx;
	uint16_t ty;

	XPT2046_GetRawTouchEvent(&tx,&ty,DOWN);

	calibration.x[i]=tx;
	calibration.y[i]=ty;
	calibration.xfb[i]=x;
	calibration.yfb[i]=y;

	if (i == 4){
		perform_calibration();
		i=0;
		return;
	}

	i++;


}

/*void XPT2046_GetCalibratedTouchEvent(uint16_t* x, uint16_t* y, TouchEventType event){

	XPT2046_GetRawTouchEvent(x, y, event);
	XPT2046_Calibrate(x,y);

}*/

void XPT2046_GetCalibratedXY(uint16_t* x, uint16_t* y){
	XPT2046_GetRawXY(x,y);
	XPT2046_Calibrate(x,y);
}
void XPT2046_GetRawXY(uint16_t* x, uint16_t* y){
 *x=touchStatus.x;
 *y=touchStatus.y;
}

void XPT2046_Calibrate(uint16_t* x, uint16_t* y){


	//	uint16_t xtemp = *x;
	//	uint16_t ytemp = *y;
	//
	//	unsigned int xt,yt;


	//-------------
	//	float a = calibration.a[5];
	//	float b = ytemp * calibration.a[0] + calibration.a[4] + xtemp * calibration.a[1];
	//	float c = xtemp * calibration.a[3];
	//
	//	float d = diskr(a,b,c);
	//
	//	if(d < 0){
	//		//xt = 999;
	//	}
	//	if(d == 0){
	//		xt = (unsigned int)(-b / (2 * a));
	//	}
	//	else {
	//		float sqrtD = sqrt(d);
	//
	//		int x1 = (unsigned int)((-b + sqrtD) / (2 * a));
	//		int x2 = (unsigned int)((-b - sqrtD) / (2 * a));
	//
	//
	//
	//		xt = ((x1 > 0) && (x2 > 0)) ? ((x1 < x2) ? x1 : x2) : ((x1 > x2) ? x1 : x2) ;
	//
	//	}
	//
	//	a = calibration.a[6];
	//	c = ytemp * calibration.a[2];
	//
	//	d = diskr(a,b,c);
	//
	//	if(d < 0){
	//		//yt = 999;
	//	}
	//	if(d == 0){
	//		yt = (unsigned int)(-b / (2 * a));
	//	}
	//	else {
	//		float sqrtD = sqrt(d);
	//
	//		int x1 = (unsigned int)((-b + sqrtD) / (2 * a));
	//		int x2 = (unsigned int)((-b - sqrtD) / (2 * a));
	//
	//		yt = ((x1 > 0) && (x2 > 0)) ? ((x1 < x2) ? x1 : x2) : ((x1 > x2) ? x1 : x2) ;
	//
	//	}
	//

	//-----------

	//	xt = calibration.xfb[0] + (int)((ytemp * calibration.a[2] - calibration.a[1] * xtemp) / calibration.a[4]);
	//
	//	yt = calibration.yfb[0] + (int)((xtemp * calibration.a[3] - calibration.a[0] * ytemp) / calibration.a[4]);


	//------------

	/*
	 *  tslib/plugins/linear.c
	 *
	 *  Copyright (C) 2016 Martin Kepplinger <martin.kepplinger@ginzinger.com>
	 *  Copyright (C) 2005 Alberto Mardegan <mardy@sourceforge.net>
	 *  Copyright (C) 2001 Russell King.
	 *
	 * This file is placed under the LGPL.  Please see the file
	 * COPYING for more details.
	 *
	 * SPDX-License-Identifier: LGPL-2.1
	 *
	 *
	 * Linearly scale touchscreen values
	 */
	int xtemp = *x;
	int ytemp = *y;
	float xt,yt;

	xt =	((calibration.a[0] +
			calibration.a[1]*xtemp +
			calibration.a[2]*ytemp) / calibration.a[6]);
	yt =	((calibration.a[3] +
			calibration.a[4]*xtemp +
			calibration.a[5]*ytemp) / calibration.a[6]);

	if(xt<0){
		*x=0;
	}else if(xt>=touchStatus.resX){
		*x=touchStatus.resX-1;
	} else{
		*x=xt;
	}

	if(yt<0){
		*y=0;
	}else if(yt>=touchStatus.resY){
		*y=touchStatus.resY-1;
	} else{
		*y=yt;
	}
	//----
	*x=(uint16_t)xt;
	*y=(uint16_t)yt;


}

/*int* XPT2046_GetCalCoeff(){
	return (int*)calibration.a;
}*/


int comp_uint16_t(const void *e1, const void *e2){
	uint16_t ee1 = *((uint16_t*)e1);
	uint16_t ee2 = *((uint16_t*)e2);
	if(ee1>ee2) return 1;
	if(ee1<ee2) return -1;
	return 0;
}

uint16_t swap_uint16_t(uint16_t n){
	uint16_t temp = n;
	return  ((temp & 0x00FFu)<<8) + ((temp & 0xFF00u)>>8);
}

int perform_calibration(){


	//	//----------------
	//	float dresx1 = (float)calibration.xfb[1]-(float)calibration.xfb[0];
	//	float dresx2 = (float)calibration.xfb[2]-(float)calibration.xfb[3];
	//	float dresx = (dresx1 + dresx2)/2;
	//
	//	float dresy1 = (float)calibration.yfb[3]-(float)calibration.yfb[0];
	//	float dresy2 = (float)calibration.yfb[2]-(float)calibration.yfb[1];
	//	float dresy = (dresy1 + dresy2)/2;
	//
	//	float dx1 = ((float)calibration.x[1]-(float)calibration.x[0]) / dresx;
	//	float dx2 = ((float)calibration.x[2]-(float)calibration.x[3]) / dresx;
	//
	//	float dy1 = ((float)calibration.y[3]-(float)calibration.y[0]) / dresy;
	//	float dy2 = ((float)calibration.y[2]-(float)calibration.y[1]) / dresy;
	//
	//	float cx = (dx2 - dx1) / dresy;
	//	float cy = (dy2 - dy1) / dresx;
	//
	//	float sx =((float)calibration.x[3] - (float)calibration.x[0]) / dresy;
	//	float sy = ((float)calibration.y[1] - (float)calibration.y[0]) / dresx;
	//
	////	float scx = sx + cx;
	////	float scy = sy + cy;
	//
	////	float scd = (scx * scy) - (dx1 * dy1);
	//
	//	calibration.a[0] = cx;
	//	calibration.a[1] = cy;
	//	calibration.a[2] = sx+dx1;
	//	calibration.a[3] = sy+dy1;
	//	calibration.a[4] = calibration.a[2] * calibration.a[3];
	//	calibration.a[5] = calibration.a[1] * calibration.a[2];
	//	calibration.a[6] = calibration.a[0] * calibration.a[3];
	//
	//
	////	calibration.a[0] = dx1;
	////	calibration.a[1] = dy1;
	////	calibration.a[2] = scx;
	////	calibration.a[3] = scy;
	////	calibration.a[4] = scd;
	//
	////-----------------

	//-------
	//

	/*
	 *  tslib/tests/ts_calibrate.c
	 *
	 *  Copyright (C) 2001 Russell King.
	 *
	 * This file is placed under the GPL.  Please see the file
	 * COPYING for more details.
	 *
	 * SPDX-License-Identifier: GPL-2.0+
	 *
	 *
	 * Graphical touchscreen calibration tool. This writes the configuration
	 * file used by tslib's "linear" filter plugin module to transform the
	 * touch samples according to the calibration.
	 */
	int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0;

	/* Get sums for matrix */
	n = x = y = x2 = y2 = xy = 0;
	for (j = 0; j < 5; j++) {
		n += 1.0;
		x += (float)calibration.x[j];
		y += (float)calibration.y[j];
		x2 += (float)(calibration.x[j]*calibration.x[j]);
		y2 += (float)(calibration.y[j]*calibration.y[j]);
		xy += (float)(calibration.x[j]*calibration.y[j]);
	}

	/* Get determinant of matrix -- check if determinant is too small */
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if (det < 0.1 && det > -0.1) {

		return 0;
	}

	/* Get elements of inverse matrix */
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

	/* Get sums for x calibration */
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)calibration.xfb[j];
		zx += (float)(calibration.xfb[j]*calibration.x[j]);
		zy += (float)(calibration.xfb[j]*calibration.y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer x coord */
	calibration.a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	calibration.a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	calibration.a[2] = (int)((c*z + f*zx + i*zy)*(scaling));



	/* Get sums for y calibration */
	z = zx = zy = 0;
	for (j = 0; j < 5; j++) {
		z += (float)calibration.yfb[j];
		zx += (float)(calibration.yfb[j]*calibration.x[j]);
		zy += (float)(calibration.yfb[j]*calibration.y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer y coord */
	calibration.a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	calibration.a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	calibration.a[5] = (int)((c*z + f*zx + i*zy)*(scaling));


	/* If we got here, we're OK, so assign scaling to a[6] and return */
	calibration.a[6] = (int)scaling;

	//------------
	return 1;
}

//float diskr(float a, float b, float c){
//	return b * b + 4 * a * c;
//}

//uint16_t sqrt(uint32_t x){
//	uint32_t fa,sa,ta;
//	uint32_t error,error_last;
//
//	// Choose an (arbitrary) first approach as the given number divided by 2
//	// The closer this number is to the final answer the faster this routines completes.
//	//fa = x/2;
//	fa = x>>1;		// Divide number by 2
//
//	// Divide the argument number by the first approach
//	sa = x/fa;
//
//	// Get the mean between the two previous numbers (add them and divide by 2).
//	ta = (fa+sa)>>1;
//
//	error_last=-1;
//	error=0;
//
//	/*
//	 * Repeat this routine until the integer output value is no longer changing.
//	 */
//	do {
//		error_last=error;
//		fa = ta;
//		sa = x/fa;
//		ta = (fa+sa)>>1;
//		error=iabs(x - ta*ta);
//	} while (error_last!=error);
//
//	// Return the integer result: square root of the input argument x.
//	return ta;
//}
