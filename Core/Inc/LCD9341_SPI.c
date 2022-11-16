/*
 * LCD9341_SPI.c
 *
 *  Created on: Oct 19, 2020
 *      Author: Eugenijus
 */

#include "LCD9341_SPI.h"

SPI_HandleTypeDef *hSPI;

const LCD9341_DMA_ModeTypeDef DMA = DMA_ON;


GPIO_TypeDef * RESET_port;
GPIO_TypeDef * CS_port;
GPIO_TypeDef * DC_port;
GPIO_TypeDef * CLK_port;

uint16_t RESET_pin;
uint16_t CS_pin;
uint16_t DC_pin;
uint16_t CLK_pin;

uint32_t Timeout=5000; //transmission timeout

static struct {
	uint16_t maxHeight;
	uint16_t maxWidth;
	uint16_t pageAddress_sp;
	uint16_t columnAddress_sp;
	uint16_t pageAddress_ep;
	uint16_t columnAddress_ep;
	uint8_t pixelFormat;
	struct{ uint8_t MY:1;
			uint8_t MX:1;
			uint8_t MV:1;
			uint8_t ML:1;
			uint8_t BRG:1;
			uint8_t MH:1;
			uint8_t reserved:2;} MADCTL;
	uint32_t dmaCounter;
	uint8_t dmaStatus;
} status;

static struct {
	uint16_t screenWidth;
	uint16_t screenHeight;
	uint16_t textColor;
	uint16_t bgColor;
	uint8_t overlay;
	sFONT font;
	uint16_t positionX;
	uint16_t positionY;
	uint16_t* symbol;
	uint8_t* symbol18bit;
}textMode;

static uint8_t isHorisontal(){
	return status.MADCTL.MV;
}

void LCD9341_setup(	SPI_HandleTypeDef* _hSPI,
					GPIO_TypeDef* _RESET_port,	uint16_t _RESET_pin,
					GPIO_TypeDef* _CS_port,		uint16_t _CS_pin,
					GPIO_TypeDef* _DC_port,		uint16_t _DC_pin,
					GPIO_TypeDef * _CLK_port,	uint16_t _CLK_pin){
	hSPI = _hSPI;
	RESET_port 	= _RESET_port;
	RESET_pin	= _RESET_pin;
	CS_port		= _CS_port;
	CS_pin		= _CS_pin;
	DC_port		= _DC_port;
	DC_pin		= _DC_pin;
	CLK_port	= _CLK_port;
	CLK_pin		= _CLK_pin;

	status.dmaCounter=0;
	status.dmaStatus=0;

	return;
}

void LCD9341_TextModeInit(sFONT font, uint16_t textColor, uint16_t bgColor, uint8_t overlay){
	textMode.font=font;
	textMode.textColor=textColor;
	textMode.bgColor=bgColor;
	textMode.screenWidth=status.maxWidth/font.Width;
	textMode.screenHeight=status.maxHeight/font.Height;

	textMode.positionX=0;
	textMode.positionY=0;
	if (textMode.symbol) free(textMode.symbol);
	textMode.symbol=malloc(sizeof(uint16_t) * (font.Height*font.Width));
	if (textMode.symbol18bit) free(textMode.symbol18bit);
	textMode.symbol18bit=malloc(sizeof(uint8_t)*3*(font.Height*font.Width));

	textMode.overlay=overlay;


}

void LCD9341_TextModeTextColor(uint16_t textColor){
	textMode.textColor = textColor;
}
void LCD9341_TextModeBgColor(uint16_t bgColor){
	textMode.bgColor = bgColor;
}
void LCD9341_TextModeSetOverlay(uint8_t overlay){
	textMode.overlay=overlay;
}
void LCD9341_TextModeSetFont(sFONT font){
	if(!textMode.symbol || font.Height != textMode.font.Height || font.Width != textMode.font.Width){
		if (textMode.symbol) free(textMode.symbol);
		textMode.symbol=malloc(sizeof(uint16_t) * (font.Height*font.Width));
		if (textMode.symbol18bit) free(textMode.symbol18bit);
		textMode.symbol18bit=malloc(sizeof(uint8_t)*3*(font.Height*font.Width));
		textMode.screenWidth=status.maxWidth/font.Width;
		textMode.screenHeight=status.maxHeight/font.Height;
		textMode.positionX=0;
		textMode.positionY=0;
	}
	textMode.font=font;
}
void LCD9341_TextModeSetPosition(uint16_t x, uint16_t y){
	if (x>=textMode.screenWidth) x=textMode.screenWidth-1;
	if (y>=textMode.screenHeight) y=textMode.screenHeight-1;

	textMode.positionX=x;
	textMode.positionY=y;
}
void LCD9341_TextModeNextPosition(){
	textMode.positionX++;
	if(textMode.positionX<textMode.screenWidth) return;
	LCD9341_TextModeNextLine();
}

void LCD9341_TextModeNextLine(){
		textMode.positionX=0;
		textMode.positionY++;
		if(textMode.positionY<=textMode.screenHeight) return;
		textMode.positionY=0;
}
void LCD9341_PutChar16bit(uint16_t x, uint16_t y, char c){

	uint32_t s = textMode.font.Width*textMode.font.Height;
	if(c<32 || c>126) c=32;
	c-=32;
	uint16_t index = c*(textMode.font.Width/8 + ((textMode.font.Width%8)?1:0))*textMode.font.Height;

	uint16_t i=0;
	uint16_t j;
	uint16_t fd;

	while(i<s){
		fd=(textMode.font.table)[index];
		for(j=0;j<8;j++){
			if(fd & (0x80>>j)) textMode.symbol[i+j]=textMode.textColor;
			else textMode.symbol[i+j]=textMode.bgColor;
			if( (i+j+1)%textMode.font.Width == 0 ) {j++;break;}
		}
		index++;
		i=i+j;
	}
	LCD9341_SetRectAccess(x, y, x+textMode.font.Width-1, y+textMode.font.Height-1);
	if(DMA){
		LCD9341_MemoryWriteBytesDMA( (uint8_t*)textMode.symbol, s*2, s*2);
	}else{
		LCD9341_MemoryWriteBytes( (uint8_t*)textMode.symbol, s*2, s*2);
	}


}
void LCD9341_PutChar16bitOverlay(uint16_t x, uint16_t y, char c){
	uint32_t s = textMode.font.Width*textMode.font.Height;
	LCD9341_ReadRect(textMode.symbol18bit,x, y, x+textMode.font.Width-1, y+textMode.font.Height-1);
	if(c<32 || c>126) c=32;
	c-=32;
	uint16_t index = c*(textMode.font.Width/8 + ((textMode.font.Width%8)?1:0))*textMode.font.Height;
	uint16_t i=0;
	uint16_t j;
	uint16_t fd;
	while(i<s){
		fd=(textMode.font.table)[index];
		for(j=0;j<8;j++){
			if(fd & (0x80>>j)) textMode.symbol[i+j]=textMode.textColor;
			else textMode.symbol[i+j]=LCD9341_TranslateColor18to16bit(textMode.symbol18bit[(i+j)*3], textMode.symbol18bit[(i+j)*3+1], textMode.symbol18bit[(i+j)*3+2]);
			if( (i+j+1)%textMode.font.Width == 0 ) {j++;break;}
		}
		index++;
		i+=j;
	}
	LCD9341_SetRectAccess(x, y, x+textMode.font.Width-1, y+textMode.font.Height-1);
	if(DMA){
		LCD9341_MemoryWriteBytesDMA( (uint8_t*)textMode.symbol, s*2, s*2);
	}else{
		LCD9341_MemoryWriteBytes( (uint8_t*)textMode.symbol, s*2, s*2);
	}
}
void LCD9341_PutString16bit(uint16_t x, uint16_t y, char*c){
	while(*c){
		LCD9341_PutChar16bit(x,y,*c);
		x+=textMode.font.Width;
		c++;
	}
}
void LCD9341_PutString16bitOverlay(uint16_t x, uint16_t y, char*c){
	while(*c){
		LCD9341_PutChar16bitOverlay(x,y,*c);
		x+=textMode.font.Width;
		c++;
	}
}
void LCD9341_TextModePutChar(char c){
	if(c=='\n') {LCD9341_TextModeNextLine(); return;}

	if(textMode.overlay){
		LCD9341_PutChar16bitOverlay(textMode.positionX*textMode.font.Width, textMode.positionY*textMode.font.Height, c);
	}else{
		LCD9341_PutChar16bit(textMode.positionX*textMode.font.Width, textMode.positionY*textMode.font.Height, c);
	}
	LCD9341_TextModeNextPosition();
}
void LCD9341_TextModePutString(char* c){
	while(*c){
		LCD9341_TextModePutChar(*c);
		c++;
	}
}
void LCD9341_TextModePutInt(int i){
	char str[11];
	itoa(i,str,10);
	LCD9341_TextModePutString(str);
}
void LCD9341_TextModePutUInt(unsigned i){
	char str[11];
	utoa(i,str,10);
	LCD9341_TextModePutString(str);
}

void LCD9341_TextModeCls(){
	LCD9341_FillScreen16bit(textMode.bgColor);
	textMode.positionX=0;
	textMode.positionY=0;
}

uint16_t LCD9341_Make16bitRGBColor(uint16_t r, uint16_t g, uint16_t b){
	r=((r>0x001F)?0x001F:r)<<11; // 5bit max 32 color
	g=((g>0x003F)?0x003F:g)<<5;  // 6bit max 64 color
	b=((b>0x001F)?0x001F:b);     // 5bit max 32 color
	uint16_t c = r+g+b;
	return ((c & 0xFF00)>>8) + ((c & 0x00FF)<<8);

}


uint16_t LCD9341_TranslateColor18to16bit(uint16_t r,uint16_t g,uint16_t b){
	return LCD9341_Make16bitRGBColor(r>>3,g>>2,b>>3);
}



void LCD9341_SendCommandByte(uint8_t cmd)
{
	DC_COMMAND();
	HAL_SPI_Transmit (hSPI, &cmd, 1, Timeout);
}
void LCD9341_SendDataByte(uint8_t dt)
{
	DC_DATA();
	HAL_SPI_Transmit (hSPI, &dt, 1, Timeout);
}
void LCD9341_SendDataBuff(uint8_t* buff, uint32_t buff_size){
	DC_DATA();
	HAL_SPI_Transmit(hSPI, (uint8_t*)buff, buff_size, Timeout);
}

// sends amount of bytes from buffer. If buffer is smaller than amount, then sending it several times
void LCD9341_SendDataBuffBytes(uint8_t* buff, uint32_t buff_size, uint32_t amount){
	DC_DATA();
	uint32_t size = amount/buff_size;
	while(size){
		HAL_SPI_Transmit(hSPI, buff, buff_size, Timeout);
		size--;
	}
	size = amount%buff_size;
	if(size) { HAL_SPI_Transmit(hSPI, buff, size, Timeout); }

}

void LCD9341_SendDataBuffDMA(uint8_t* dt, uint32_t size){
	DC_DATA();
	HAL_SPI_Transmit_DMA(hSPI, dt, size);
}

void LCD9341_MemoryWrite(uint8_t *dt, uint32_t size){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_WRITE);
	LCD9341_SendDataBuff(dt,size);
	CS_IDLE();

}
void LCD9341_MemoryWriteBytes(uint8_t *buff, uint32_t buff_size, uint32_t amount){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_WRITE);
	LCD9341_SendDataBuffBytes(buff,buff_size,amount);
	CS_IDLE();
}

void LCD9341_MemoryWriteDMA(uint8_t *dt, uint32_t size){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_WRITE);
	LCD9341_SendDataBuffDMA(dt,size);
}
void LCD9341_MemoryWriteBytesDMA(uint8_t *buff, uint32_t buff_size, uint32_t amount){
	if(buff_size<MIN_DMA_BUFF_SIZE || amount<MIN_DMA_BUFF_SIZE) {
		LCD9341_MemoryWriteBytes(buff, buff_size, amount);
		return;
	}
	uint32_t size = amount/buff_size;

	if(size){
		status.dmaCounter=size;
		status.dmaStatus=1;
		LCD9341_MemoryWriteDMA(buff, buff_size);
		while(status.dmaStatus){}
	}
	size=amount%buff_size;

	if(size<MIN_DMA_BUFF_SIZE){
		LCD9341_MemoryWriteContinue(buff, size);
		return;
	}
	status.dmaCounter=1;
	status.dmaStatus=1;
	LCD9341_MemoryWriteContinueDMA(buff, size);
	while(status.dmaStatus){}

}


void LCD9341_MemoryWriteContinue(uint8_t *dt, uint32_t size){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_WRITE_MEMORY_CONTINUE);
	LCD9341_SendDataBuff(dt,size);
	CS_IDLE();

}
void LCD9341_MemoryWriteContinueDMA(uint8_t *dt, uint32_t size){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_WRITE_MEMORY_CONTINUE);
	LCD9341_SendDataBuffDMA(dt,size);
}

void LCD9341_SetMemoryAccessControl(LCD9341_MemoryAccessControl rotation){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_ACCESS_CONTROL);
	LCD9341_SendDataByte((uint8_t)rotation);
	CS_IDLE();

	if(rotation==HORISONTAL ||
			rotation==HORISONTAL_V_FLIP ||
			rotation==HORISONTAL_H_FLIP ||
			rotation==HORISONTAL_HV_FLIP){
		status.MADCTL.MV=1;
		status.maxWidth=MAX_Y;
		status.maxHeight=MAX_X;
	}else{
		status.MADCTL.MV=0;
		status.maxWidth=MAX_X;
		status.maxHeight=MAX_Y;
	}
}

void LCD9341_SetPixelFormat16bit(){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_PIXEL_FORMAT_SET);
	LCD9341_SendDataByte(0x55);
	CS_IDLE();
	status.pixelFormat=0x55;
}
/*void LCD9341_SetPixelFormat18bit(){
	LCD9341_SendCommandByte(LCD9341_CMD_PIXEL_FORMAT_SET);
	LCD9341_SendDataByte(0x66);
	status.pixelFormat=0x66;
}*/
void LCD9341_SetFullScreenAccess(){
	if(isHorisontal()){
		LCD9341_SetRectAccess(0,0,MAX_Y,MAX_X);
	}else{
		LCD9341_SetRectAccess(0,0,MAX_X,MAX_Y);

	}
}
void LCD9341_SetRectAccess(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
	LCD9341_SetPageAddress(y1,y2);
	LCD9341_SetColumnAddress(x1,x2);
}

void LCD9341_FillScreen16bit(uint16_t color){
	if(isHorisontal()){ LCD9341_FillRect16bit(0, 0, MAX_Y, MAX_X, color);}
	else			{ LCD9341_FillRect16bit(0, 0, MAX_X, MAX_Y, color);}
}
void LCD9341_FillRect16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color){
	LCD9341_SetRectAccess(x1,y1,x2,y2);
	uint32_t size = (status.columnAddress_ep - status.columnAddress_sp + 1)*
					(status.pageAddress_ep   - status.pageAddress_sp    + 1);
	uint16_t data[4] = {color, color, color, color};
	if(DMA){
		LCD9341_MemoryWriteBytesDMA((uint8_t*)data,8,size*2);
	} else {
		LCD9341_MemoryWriteBytes((uint8_t*)data, 8,size*2);
	}
}
void LCD9341_FillRectFromBuff16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* buff, uint16_t buff_size){
	LCD9341_SetRectAccess(x1,y1,x2,y2);
	uint32_t size = (status.columnAddress_ep - status.columnAddress_sp + 1)*
						(status.pageAddress_ep   - status.pageAddress_sp    + 1);
	if(DMA){
		LCD9341_MemoryWriteBytesDMA(buff,buff_size,size*2);
	} else {
		LCD9341_MemoryWriteBytes(buff, buff_size,size*2);
	}
}

void LCD9341_DrawPixel16bit(uint16_t x, uint16_t y, uint16_t color){
	LCD9341_FillRect16bit(x,y,x,y,color);
}
void LCD9341_DrawLine16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color){

	if(x1==x2 || y1==y2){ LCD9341_FillRect16bit(x1, y1, x2, y2, color); return; }

	int16_t dx = (x1<x2)? (x2-x1+1) : (x1-x2+1);
	int16_t dy = (y1<y2)? (y2-y1+1) : (y1-y2+1);

	int16_t k = ((x1<x2 && y1<y2) || (x1>x2 && y1>y2))? 1 : -1;

	LCD9341_AlignCoords(&x1, &y1, &x2, &y2);

	if(dx>dy){
		int16_t xi;
		int16_t yt;
		for(xi=0;xi<dx;xi++){
			yt = (dy*xi)/dx;
			if(k<0){
				LCD9341_DrawPixel16bit(x1+xi, y2-yt, color);
			}else{
				LCD9341_DrawPixel16bit(x1+xi, y1+yt, color);
			}
		}
	}else{
		int16_t yi;
		int16_t xt;
		for(yi=0;yi<dy;yi++){
			xt = (dx*yi)/dy;
			if(k<0){
				LCD9341_DrawPixel16bit(x1+xt, y2-yi, color);
			}else{
				LCD9341_DrawPixel16bit(x1+xt, y1+yi, color);
			}
		}
	}


}
void LCD9341_DrawCircle16bit(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color){
	uint32_t yt;
	uint32_t i;
	uint32_t r2=r*r;
	uint32_t lt;
	uint32_t a;
	for(i=0;i<=r;i++){
		for(a = r/2;a<=r;a++){
			yt=(i*a)/r;
			lt=yt*yt+a*a;
			if(r2<=lt){

				LCD9341_DrawPixel16bit(cx+a,cy+yt,color);
				LCD9341_DrawPixel16bit(cx+a,cy-yt,color);
				LCD9341_DrawPixel16bit(cx-a,cy+yt,color);
				LCD9341_DrawPixel16bit(cx-a,cy-yt,color);
				LCD9341_DrawPixel16bit(cx+yt,cy+a,color);
				LCD9341_DrawPixel16bit(cx+yt,cy-a,color);
				LCD9341_DrawPixel16bit(cx-yt,cy+a,color);
				LCD9341_DrawPixel16bit(cx-yt,cy-a,color);
				break;
			}
		}
	}
}
void LCD9341_DrawCircle16bit2(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color){
	//alternative. faster?
	int32_t x1=r;
	int32_t y1=0;
	int32_t t1,t2;

	LCD9341_DrawPixel16bit(cx+r,cy,color);
	LCD9341_DrawPixel16bit(cx,cy+r,color);
	LCD9341_DrawPixel16bit(cx-r,cy,color);
	LCD9341_DrawPixel16bit(cx,cy-r,color);

	t1 = 2*r-1;
	t2 = 0;

	while(x1>y1){
		if(abs(t1)<abs(t2)){
			x1--;
			t2=t1;
			t1+=(2*x1-1);
		}
		LCD9341_DrawPixel16bit(cx+x1,cy+y1,color);
		LCD9341_DrawPixel16bit(cx+x1,cy-y1,color);
		LCD9341_DrawPixel16bit(cx-x1,cy+y1,color);
		LCD9341_DrawPixel16bit(cx-x1,cy-y1,color);
		LCD9341_DrawPixel16bit(cx+y1,cy+x1,color);
		LCD9341_DrawPixel16bit(cx+y1,cy-x1,color);
		LCD9341_DrawPixel16bit(cx-y1,cy+x1,color);
		LCD9341_DrawPixel16bit(cx-y1,cy-x1,color);
		y1++;
		t2-=(2*y1+1);
		t1-=(2*y1+1);
	}
}
void LCD9341_DrawLineLength16bit(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t length, uint16_t color){

	int16_t dx = (x1<x2)? (x2-x1+1) : (x1-x2+1);
	int16_t dy = (y1<y2)? (y2-y1+1) : (y1-y2+1);

	int16_t k = ((x1<x2 && y1<y2) || (x1>x2 && y1>y2))? 1 : -1;
	uint32_t l=length*length;

	LCD9341_AlignCoords(&x1, &y1, &x2, &y2);
	uint32_t lt;
	if(dx>dy){
		int16_t xi;
		int16_t yt;
		for(xi=0;xi<dx;xi++){
			yt = (dy*xi)/dx;
			lt = yt*yt+xi*xi;
			if(l<=lt){
				if(k<0){
					LCD9341_DrawPixel16bit(x1+xi, y2-yt, color);
				}else{
					LCD9341_DrawPixel16bit(x1+xi, y1+yt, color);
				}
				return;
			}
		}
	}else{
		int16_t yi;
		int16_t xt;
		for(yi=0;yi<dy;yi++){
			xt = (dx*yi)/dy;
			lt=xt*xt+yi*yi;
			if(l<=lt){
				if(k<0){
					LCD9341_DrawPixel16bit(x1+xt, y2-yi, color);
				}else{
					LCD9341_DrawPixel16bit(x1+xt, y1+yi, color);
				}
				return;
			}
		}
	}
}


/*
 * if DMA_ON, function should be called in these callbacks:
 *
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	LCD9341_DMAInterrupt(hspi);
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	LCD9341_DMAInterrupt(hspi);
}
*/
void LCD9341_DMAInterrupt(SPI_HandleTypeDef* hspi){
	if(hspi==hSPI){
		status.dmaCounter-=1;
		if(status.dmaCounter==0){
			HAL_SPI_DMAStop(hspi);
			CS_IDLE();
			status.dmaStatus=0;}
	}
}

void LCD9341_SetPageAddress(uint16_t sp, uint16_t ep){
	LCD9341_AlignCoordsPage(&sp,&ep);
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_PAGE_ADDRESS_SET);
	uint8_t data[4]={sp>>8 & 0xFF, sp & 0xFF,ep>>8 & 0xFF, ep & 0xFF};
	LCD9341_SendDataBuff(data, 4);
	CS_IDLE();
	status.pageAddress_sp=sp;
	status.pageAddress_ep=ep;
}
void LCD9341_SetColumnAddress(uint16_t sp, uint16_t ep){
	LCD9341_AlignCoordsColumn(&sp,&ep);
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_COLUMN_ADDRESS_SET);
	uint8_t data[4]={sp>>8 & 0xFF, sp & 0xFF,ep>>8 & 0xFF, ep & 0xFF};
	LCD9341_SendDataBuff(data, 4);
	CS_IDLE();
	status.columnAddress_sp=sp;
	status.columnAddress_ep=ep;
}
void LCD9341_AlignCoords(uint16_t* x1, uint16_t* y1, uint16_t* x2, uint16_t* y2){
	LCD9341_AlignCoordsColumn(y1,y2);
	LCD9341_AlignCoordsPage(x1,x2);
}
void LCD9341_AlignCoordsColumn(uint16_t  *a1, uint16_t *a2){
	uint16_t x1 = *a1;
	uint16_t x2 = *a2;
	if (x1>x2) {uint16_t t=x1; x1=x2; x2=t;} //start position should be greater than end position
	if (x1>status.maxWidth){x1=status.maxWidth; x2=status.maxWidth;}
	if (x2>status.maxWidth){x2=status.maxWidth;}
	*a1=x1;
	*a2=x2;
}
void LCD9341_AlignCoordsPage(uint16_t  *a1, uint16_t *a2){
	uint16_t x1 = *a1;
	uint16_t x2 = *a2;
	if (x1>x2) {uint16_t t=x1; x1=x2; x2=t;} //start position should be greater than end position
			if (x1>status.maxHeight){x1=status.maxHeight;x2=status.maxHeight;}
			if (x2>status.maxHeight){x2=status.maxHeight;}
	*a1=x1;
	*a2=x2;
}

void LCD9341_ReceiveDataByte(uint8_t *dt){

	DC_DATA();
	HAL_SPI_Receive(hSPI, dt, 1, Timeout);
	CS_IDLE();
}
void LCD9341_ReceiveDataBuff(uint8_t *dt, uint16_t size){
	DC_DATA();
	uint8_t dummy[1];
	HAL_SPI_Receive(hSPI, dummy, 1, Timeout);
	HAL_SPI_Receive(hSPI, dt, size, Timeout);
	CS_IDLE();
}
void LCD9341_ReceiveDataBuffDMA(uint8_t *dt, uint16_t size){
	DC_DATA();
	uint8_t dummy[1];
	HAL_SPI_Receive(hSPI, dummy, 1, Timeout);
	HAL_SPI_Receive_DMA(hSPI, dt, size);
}


void LCD9341_MemoryRead(uint8_t *dt, uint16_t size){
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_READ);
	LCD9341_ReceiveDataBuff(dt,size);
	CS_IDLE();
}

void LCD9341_MemoryReadDMA(uint8_t *buff, uint16_t size){
	if(size<MIN_DMA_BUFF_SIZE) {
		LCD9341_MemoryRead(buff, size);
		return;
	}
	status.dmaCounter=1;
	status.dmaStatus=1;
	CS_ACTIVE();
	LCD9341_SendCommandByte(LCD9341_CMD_MEMORY_READ);
	LCD9341_ReceiveDataBuffDMA(buff,size);
	while(status.dmaStatus){}
}

void LCD9341_ReadRect(uint8_t *buff, uint16_t x1,uint16_t y1, uint16_t x2, uint16_t y2){
	LCD9341_SetRectAccess(x1, y1, x2, y2);
	uint32_t size = (status.columnAddress_ep - status.columnAddress_sp + 1)*
						(status.pageAddress_ep   - status.pageAddress_sp    + 1);
	if(DMA){
		LCD9341_MemoryReadDMA(buff,size*3);
	}else{
		LCD9341_MemoryRead(buff,size*3);
	}

}



void LCD9341_HW_reset(void)
{
	RESET_ACTIVE();
	HAL_Delay(5);
	RESET_IDLE();
}
void LCD9341_Power_ON(){
//	uint8_t s;
	CS_ACTIVE();											//Chip Select
	LCD9341_HW_reset();										//Hardware Reset
	LCD9341_SendCommandByte(LCD9341_CMD_SOFTWARE_RESET);	//Software Reset
	HAL_Delay(1000);
	LCD9341_SendCommandByte(LCD9341_CMD_SLEEP_OUT);				//Sleep mode off
	HAL_Delay(300);
	LCD9341_SendCommandByte(LCD9341_CMD_DISPLAY_ON);
	HAL_Delay(1000);
	CS_IDLE();
}

/*void LCD9341_RandomRect(int(rand)(void)) {
	LCD9341_FillRect16bit(rand()%status.maxWidth, rand()%status.maxHeight,rand()%status.maxWidth, rand()%status.maxHeight, rand());
}
void LCD9341_RandomLine(int(rand)(void)) {
	LCD9341_DrawLine16bit(rand()%status.maxWidth, rand()%status.maxHeight,rand()%status.maxWidth, rand()%status.maxHeight, rand());
}
void LCD9341_RandomPixel(int(rand)(void)) {
	LCD9341_DrawPixel16bit(rand()%status.maxWidth, rand()%status.maxHeight, rand());
}*/
