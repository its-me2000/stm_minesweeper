/*
 * gameUI.c
 *
 *  Created on: Jan 26, 2021
 *      Author: Eugenijus
 */

#include "gameUI.h"
//#include "minegame.h"
#define FIELD_SIZE_X 12
#define FIELD_SIZE_Y 12
#define FIELD_MINES 15

#define TILE_SIZE_X 17
#define TILE_SIZE_Y 17
#define TILE_GAP_X 1
#define TILE_GAP_Y 1
#define TILE_CHAR_OFFSET_X 3
#define TILE_CHAR_OFFSET_Y 3

#define BUTTON_BEWEL 0
#define BUTTON_FONT Font16
#define BUTTON_TEXT_OFFSET_X 10
#define BUTTON_TEXT_OFFSET_Y 13

extern tile*  field;
extern uint16_t field_width;
extern uint16_t field_height;
extern uint16_t field_mines;


struct {
	uint16_t x1;
	uint16_t x2;
	uint16_t y1;
	uint16_t y2;
}Field;

struct {
	uint16_t x;
	uint16_t y;
	uint16_t sizeX;
	uint16_t sizeY;
	uint8_t state;
}ResetButton;

struct {
	uint16_t x;
	uint16_t y;
	uint16_t sizeX;
	uint16_t sizeY;
	mouse_button mb;
}MarkButton;

struct {
	uint16_t x;
	uint16_t y;
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t time;
	uint8_t update;
}Timer;

struct {
	uint16_t x;
	uint16_t y;
	uint16_t sizeX;
	uint16_t sizeY;
	int16_t count;
}Counter;


void SplashScreen(){

	LCD9341_FillScreen16bit(BLACK_COLOR);
	LCD9341_TextModeInit(Font24, WHITE_TEXT_COLOR, BLACK_COLOR, 0);

	uint16_t i = 23;

	float Mx,My,Md,Ix,Iy,Id,Nx,Ny,Nd,Ex,Ey,Ed;

	uint16_t t=25;

	Mx = 0; My = 91; Md = 128.0/t;
	Ix = 145; Iy = 0; Id = 91.0/t;
	Nx = 162; Ny = 239-24; Nd = (Ny-91)/t;
	Ex = 320-17; Ey = 91; Ed = (Ex-179)/t;

	for (i=0; i < t; i++){

		LCD9341_TextModeTextColor(WHITE_TEXT_COLOR);
		LCD9341_PutChar16bit((uint16_t)Mx,(uint16_t)My,'M');
		LCD9341_PutChar16bit((uint16_t)Ix,(uint16_t)Iy,'I');
		LCD9341_PutChar16bit((uint16_t)Nx,(uint16_t)Ny,'N');
		LCD9341_PutChar16bit((uint16_t)Ex,(uint16_t)Ey,'E');
		HAL_Delay(300/t);
		LCD9341_TextModeTextColor(BLACK_COLOR);
		LCD9341_PutChar16bit((uint16_t)Mx,(uint16_t)My,'M');
		LCD9341_PutChar16bit((uint16_t)Ix,(uint16_t)Iy,'I');
		LCD9341_PutChar16bit((uint16_t)Nx,(uint16_t)Ny,'N');
		LCD9341_PutChar16bit((uint16_t)Ex,(uint16_t)Ey,'E');
		Mx=Mx+Md;
		Iy=Iy+Id;
		Ny=Ny-Nd;
		Ex=Ex-Ed;

	}

	LCD9341_TextModeTextColor(WHITE_TEXT_COLOR);
	//	LCD9341_PutChar16bit(128,91,'M');
	//	LCD9341_PutChar16bit(145,91,'I');
	//	LCD9341_PutChar16bit(162,91,'N');
	//	LCD9341_PutChar16bit(179,91,'E');
	LCD9341_PutString16bit(128,91,"MINE");

	for(i=0; i<32; i++){
		LCD9341_TextModeTextColor(LCD9341_Make16bitRGBColor(i, i*2, i));
		LCD9341_TextModeSetPosition(6, 5);
		LCD9341_TextModePutString("SWEEPER");
		HAL_Delay(300/t);
	}
	LCD9341_TextModeTextColor(WHITE_TEXT_COLOR);
	LCD9341_TextModeSetPosition(6, 5);
	LCD9341_TextModePutString("SWEEPER");

	LCD9341_TextModeInit(Font8, DGREY_COLOR, BLACK_COLOR, 0);
	LCD9341_TextModeSetPosition(17, 30);
	char* c = "by Eugenijus Margalikas. 2021";
	LCD9341_TextModePutString(c);

	char* c1=c;
	char* c2=c;
	char* c3=c;
	char* c4=c;

	uint16_t c1x=17,c2x=17,c3x=17,c4x=17;

	while(*c4){
		if(c3!=c && *c4){
			LCD9341_TextModeSetPosition(c4x,30);
			LCD9341_TextModeTextColor(DGREY_COLOR);
			LCD9341_TextModePutChar(*c4);
			c4++;
			c4x++;
		}
		if(c2!=c && *c3){
			LCD9341_TextModeSetPosition(c3x,30);
			LCD9341_TextModeTextColor(LGREY_COLOR);
			LCD9341_TextModePutChar(*c3);
			c3++;
			c3x++;
		}
		if(c1!=c && *c2){
			LCD9341_TextModeSetPosition(c2x,30);
			LCD9341_TextModeTextColor(WHITE_TEXT_COLOR);
			LCD9341_TextModePutChar(*c2);
			c2++;
			c2x++;
		}
		if(*c1){
			LCD9341_TextModeSetPosition(c1x,30);
			LCD9341_TextModeTextColor(LGREY_COLOR);
			LCD9341_TextModePutChar(*c1);
			c1++;
			c1x++;
		}
		HAL_Delay(20);
	}
	LCD9341_TextModeTextColor(DGREY_COLOR);
	LCD9341_TextModeSetPosition(17, 30);
	LCD9341_TextModePutString(c);


}

void DrawTile(uint16_t x, uint16_t y){
	tile t = get_tile(x,y);
	LCD9341_TextModeInit(Font16, WHITE_TEXT_COLOR, BLACK_COLOR, 0);
	x=TILE_GAP_X+Field.x1+(x*(TILE_SIZE_X+TILE_GAP_X));
	y=1+Field.y1+(y*(TILE_SIZE_Y+TILE_GAP_Y));
	if(t.status==OPEN){
		LCD9341_FillRect16bit(x, y, x+TILE_SIZE_X-1,y+TILE_SIZE_Y-1 , GREY_COLOR);
		char c;
		switch(t.value){
		case ONE:
			LCD9341_TextModeTextColor(BLUE_COLOR);
			c='1';
			break;
		case TWO:
			LCD9341_TextModeTextColor(GREEN_COLOR);
			c='2';
			break;
		case THREE:
			LCD9341_TextModeTextColor(RED_COLOR);
			c='3';
			break;
		case FOUR:
			LCD9341_TextModeTextColor(DBLUE_COLOR);
			c='4';
			break;
		case FIVE:
			LCD9341_TextModeTextColor(DRED_COLOR);
			c='5';
			break;
		case SIX:
			LCD9341_TextModeTextColor(PURPLE_COLOR);
			c='6';
			break;
		case SEVEN:
			LCD9341_TextModeTextColor(DGREEN_COLOR);
			c='7';
			break;
		case EIGHT:
			LCD9341_TextModeTextColor(DPURPLE_COLOR);
			c='8';
			break;
		case MINE:
			LCD9341_TextModeTextColor(BLACK_COLOR);
			c='*';
			LCD9341_FillRect16bit(x, y, x+TILE_SIZE_X-1,y+TILE_SIZE_Y-1 , RED_COLOR);
			break;
		default:
			c=' ';
			break;
		}
		LCD9341_PutChar16bitOverlay(x+TILE_CHAR_OFFSET_X,y+TILE_CHAR_OFFSET_Y,c);
	}else{
		//LCD9341_FillRect16bit(x, y, x+TILE_SIZE_X-1,y+TILE_SIZE_Y-1 , LLGREY_COLOR);

		DrawButton(x,y,TILE_SIZE_X,TILE_SIZE_Y,1);
		if(game_state.status==LOST){
			if(t.status == MARKED && t.value != MINE){
				char c='X';
				LCD9341_TextModeTextColor(RED_COLOR);
				LCD9341_PutChar16bitOverlay(x+TILE_CHAR_OFFSET_X,y+TILE_CHAR_OFFSET_Y,c);
			}
			if((t.status == CLOSED || t.status == UNMARKED) && t.value == MINE){
				char c='*';
				LCD9341_TextModeTextColor(BLACK_COLOR);
				LCD9341_PutChar16bitOverlay(x+TILE_CHAR_OFFSET_X,y+TILE_CHAR_OFFSET_Y,c);
			}
		}else{

			char c=' ';
			if(t.status == MARKED) c='M';
			if(t.status == UNMARKED) c='?';
			LCD9341_TextModeTextColor(BLACK_COLOR);
			LCD9341_PutChar16bitOverlay(x+TILE_CHAR_OFFSET_X,y+TILE_CHAR_OFFSET_Y,c);
		}
	}
}

void DrawField(){
	if(!field) return;
	Field.x1 = 90;
	Field.y1 = 11;
	Field.x2 =  Field.x1+(field_width*(TILE_SIZE_X+TILE_GAP_X));
	Field.y2 = Field.y1+(field_height*(TILE_SIZE_Y+TILE_GAP_Y));
	LCD9341_FillRect16bit(Field.x1,Field.y1,Field.x2,Field.y2 , DGREY_COLOR);

	DrawTiles();
}
void DrawTiles(){

	for(uint16_t i = 0; i < field_width; i++){
			for(uint16_t j = 0; j < field_height; j++){
				//DrawTile(TILE_GAP_X+UIFieldX1+(i*(TILE_SIZE_X+TILE_GAP_X)),1+UIFieldY1+(j*(TILE_SIZE_Y+TILE_GAP_Y)),get_tile(i,j));
				DrawTile(i,j);
			}
		}
}

void DrawButton(uint16_t posX, uint16_t posY, uint16_t sizeX, uint16_t sizeY, uint8_t state){
	if(state){
		LCD9341_FillRect16bit(posX+sizeX-1-BUTTON_BEWEL, posY, 			posX+sizeX-1, 	posY+sizeY-1, DGREY_COLOR);
		LCD9341_FillRect16bit(posX, 		posY, 			posX+BUTTON_BEWEL, 	  	posY+sizeY-1, LGREY_COLOR);
		LCD9341_FillRect16bit(posX, 		posY+sizeY-1-BUTTON_BEWEL, 	posX+sizeX-1, 	posY+sizeY-1, DDGREY_COLOR);
		LCD9341_FillRect16bit(posX,			posY,			posX+sizeX-1, 	posY+BUTTON_BEWEL, 	  LLGREY_COLOR);
	}else{
		LCD9341_FillRect16bit(posX+sizeX-1-BUTTON_BEWEL, posY, 			posX+sizeX-1, 	posY+sizeY-1, LGREY_COLOR);
		LCD9341_FillRect16bit(posX, 		posY, 			posX+BUTTON_BEWEL, 	  	posY+sizeY-1, DGREY_COLOR);
		LCD9341_FillRect16bit(posX, 		posY+sizeY-1-BUTTON_BEWEL, 	posX+sizeX-1, 	posY+sizeY-1, LLGREY_COLOR);
		LCD9341_FillRect16bit(posX,			posY,			posX+sizeX-1, 	posY+BUTTON_BEWEL, 	  DDGREY_COLOR);
	}

	LCD9341_FillRect16bit(posX+1+BUTTON_BEWEL, 		posY+1+BUTTON_BEWEL, 		posX+sizeX-2-BUTTON_BEWEL, posY+sizeY-2-BUTTON_BEWEL, GREY_COLOR);

}
void DrawButtonWithText(uint16_t posX, uint16_t posY, uint16_t sizeX, uint16_t sizeY, uint8_t state,char* c){
	DrawButton(posX,posY,sizeX,sizeY,state);
	LCD9341_TextModeInit(BUTTON_FONT, BLACK_COLOR, BLACK_COLOR, 0);
	if(state){
		LCD9341_PutString16bitOverlay(posX+BUTTON_TEXT_OFFSET_X, posY+BUTTON_TEXT_OFFSET_Y, c);
	}else{
		LCD9341_PutString16bitOverlay(posX+BUTTON_TEXT_OFFSET_X+1, posY+BUTTON_TEXT_OFFSET_Y+1, c);
	}
}
void MakeResetButton(uint16_t x, uint16_t y){
	ResetButton.x=x;
	ResetButton.y=y;
	ResetButton.sizeX = 70;
	ResetButton.sizeY = 40;
	ResetButton.state = 1;
	UpdateResetButton();
}
void MakeMarkButton(uint16_t x, uint16_t y){
	MarkButton.x=x;
	MarkButton.y=y;
	MarkButton.sizeX = 70;
	MarkButton.sizeY = 40;
	MarkButton.mb = LCLICK;
	UpdateMarkButton();
}
void MakeTimer(uint16_t x, uint16_t y){
	Timer.x=x;
	Timer.y=y;
	Timer.sizeX=70;
	Timer.sizeY=50;
	Timer.time=0;
	Timer.update=1;

	LCD9341_FillRect16bit(Timer.x, Timer.y,Timer.x+Timer.sizeX-1, Timer.y+Timer.sizeY-1, GREY_COLOR);
	LCD9341_FillRect16bit(Timer.x+3, Timer.y+5,Timer.x+Timer.sizeX-4, Timer.y+Timer.sizeY-4, LGREY_COLOR);
	LCD9341_FillRect16bit(Timer.x+4, Timer.y+6,Timer.x+Timer.sizeX-5, Timer.y+Timer.sizeY-5, GREY_COLOR);
	LCD9341_TextModeInit(Font12,BLACK_COLOR, GREY_COLOR, 0);
	LCD9341_PutString16bit(Timer.x+6, Timer.y+1, "Time:");
	LCD9341_FillRect16bit(Timer.x+6, Timer.y+11,Timer.x+Timer.sizeX-7, Timer.y+Timer.sizeY-7, DGREY_COLOR);
	LCD9341_FillRect16bit(Timer.x+7, Timer.y+12,Timer.x+Timer.sizeX-8, Timer.y+Timer.sizeY-8, BLACK_COLOR);
	UpdateTimer();
}

void MakeCounter(uint16_t x, uint16_t y){
	Counter.x=x;
	Counter.y=y;
	Counter.sizeX=70;
	Counter.sizeY=50;
	Counter.count=game_state.mines_count;

	LCD9341_FillRect16bit(Counter.x, Counter.y,Counter.x+Counter.sizeX-1, Counter.y+Counter.sizeY-1, GREY_COLOR);
	LCD9341_FillRect16bit(Counter.x+3, Counter.y+5,Counter.x+Counter.sizeX-4, Counter.y+Counter.sizeY-4, LGREY_COLOR);
	LCD9341_FillRect16bit(Counter.x+4, Counter.y+6,Counter.x+Counter.sizeX-5, Counter.y+Counter.sizeY-5, GREY_COLOR);
	LCD9341_TextModeInit(Font12,BLACK_COLOR, GREY_COLOR, 0);
	LCD9341_PutString16bit(Counter.x+6, Counter.y+1, "Mines:");
	LCD9341_FillRect16bit(Counter.x+6, Counter.y+11,Counter.x+Counter.sizeX-7, Counter.y+Counter.sizeY-7, DGREY_COLOR);
	LCD9341_FillRect16bit(Counter.x+7, Counter.y+12,Counter.x+Counter.sizeX-8, Counter.y+Counter.sizeY-8, BLACK_COLOR);
	UpdateCounter();
}

void UpdateTimer(){
	if(!Timer.update) return;
	uint16_t time1,time2,time3;
	char str[4];
	if(Timer.time<999){
		time1=Timer.time%10;
		time2=(Timer.time%100)/10;
		time3=(Timer.time%1000)/100;
		str[0]= (char)(48+time3);
		str[1]= (char)(48+time2);
		str[2]=	(char)(48+time1);
		str[3]= 0;
	}
	else{
		str[0]= '9';
		str[1]= '9';
		str[2]=	'9';
		str[3]= 0;
	}
	Timer.update=0;
	LCD9341_TextModeInit(Font24, RED_COLOR, BLACK_COLOR, 0);
	LCD9341_PutString16bit(Timer.x+10, Timer.y+18, str);


}
void UpdateCounter(){

	char str[3];
	if(Counter.count>=0){
		uint16_t time1,time2;
		time1=Counter.count%10;
		time2=(Counter.count%100)/10;

		str[0]= (char)(48+time2);
		str[1]=	(char)(48+time1);
		str[2]= 0;
		LCD9341_TextModeInit(Font24, GREEN_COLOR, BLACK_COLOR, 0);
		LCD9341_PutString16bit(Counter.x+20, Counter.y+18, str);
	}else{
		str[0]= '-';
		str[1]=	'-';
		str[2]= 0;
		LCD9341_TextModeInit(Font24, DRED_COLOR, BLACK_COLOR, 0);
		LCD9341_PutString16bit(Counter.x+20, Counter.y+18, str);

	}

}

void SetTimer(uint16_t t){
	if(t==Timer.time) return;

	Timer.time=t;
	Timer.update=1;
	//UpdateTimer();
}

void NextTimer(){
	if(game_state.status!=RUNNING) return;
	Timer.time=Timer.time+1;
	Timer.update=1;
}

void SetCounter(uint16_t t){
	if(t==Counter.count) return;
	Counter.count=t;
	UpdateCounter();
}
void DownCounter(){
	SetCounter(Counter.count-1);
	UpdateCounter();
}
void UpCounter(){
	SetCounter(Counter.count+1);
	UpdateCounter();
}

void UpdateResetButton(){
	DrawButtonWithText(ResetButton.x,ResetButton.y,ResetButton.sizeX,ResetButton.sizeY,ResetButton.state,"Reset");
}

void UpdateMarkButton(){
	uint16_t state = (MarkButton.mb==LCLICK)?1:0;
	DrawButtonWithText(MarkButton.x,MarkButton.y,MarkButton.sizeX,MarkButton.sizeY,state,"Mark");
}

void ClickOnMark(){
	MarkButton.mb = (MarkButton.mb==LCLICK)?RCLICK:LCLICK;
	UpdateMarkButton();
}

void ClickOnReset(){
	ResetButton.state=0;
	UpdateResetButton();
	MarkButton.mb=LCLICK;
	UpdateMarkButton();
	new_game(FIELD_SIZE_X,FIELD_SIZE_Y,FIELD_MINES);
	DrawField();
	SetTimer(0);
	SetCounter(game_state.mines_count);
	ResetButton.state=1;
	UpdateResetButton();
}

void UIClick(uint16_t x, uint16_t y){
	if(x>=Field.x1 && x<=Field.x2 && y>=Field.y1 && y<=Field.y2){
		ClickOnField(x-Field.x1,y-Field.y1, MarkButton.mb);
	}else if(x>=ResetButton.x && x<ResetButton.x+ResetButton.sizeX && y>=ResetButton.y && y<ResetButton.y+ResetButton.sizeY){
		ClickOnReset();
	}else if(x>=MarkButton.x && x<MarkButton.x+MarkButton.sizeX && y>=MarkButton.y && y<=MarkButton.y+MarkButton.sizeY){
		ClickOnMark();
	}
}

void ClickOnField(uint16_t x, uint16_t y, mouse_button mb){
	if(game_state.status==LOST || game_state.status==WIN) return;
	if(game_state.status==PAUSE) start_game();
	x = (x-TILE_GAP_X) / (TILE_SIZE_X+TILE_GAP_X);
	y = (y-TILE_GAP_Y) / (TILE_SIZE_Y+TILE_GAP_Y);
	click(x,y,mb);
}

void no(void){

}

void Lost(){
	DrawTiles();
	LCD9341_TextModeInit(Font24, RED_COLOR, BLACK_COLOR, 0);
	LCD9341_PutString16bitOverlay(Field.x1+40, Field.y1+95, "YOU LOST!");
}
void Win(){
	DrawTiles();
	LCD9341_TextModeInit(Font24, YELLOW_COLOR, BLACK_COLOR, 0);
	LCD9341_PutString16bitOverlay(Field.x1+45, Field.y1+95, "YOU WIN!");
}

void UIStart(){

	init_game(DrawTile,Win,Lost,DownCounter,UpCounter);
	new_game(FIELD_SIZE_X,FIELD_SIZE_Y,FIELD_MINES);
	MakeResetButton(10,11);
	MakeTimer(10,66);
	MakeCounter(10,121);
	MakeMarkButton(10,186);
	DrawField();
}

void TEST_DrawField(){
	init_game(DrawTile,no,no,DownCounter,UpCounter);
	new_game(FIELD_SIZE_X,FIELD_SIZE_Y,FIELD_MINES);
	DrawField();
	MakeResetButton(10,11);
	MakeTimer(10,66);
	MakeCounter(10,121);
	MakeMarkButton(10,186);
	click(3,3,LCLICK);
	click(4,4,RCLICK);
	click(5,5,LCLICK);
	click(6,6,LCLICK);
	ClickOnReset();
	click(3,3,LCLICK);
	click(4,4,RCLICK);
	click(5,5,LCLICK);
	click(6,6,LCLICK);
	SetTimer(123);
	SetCounter(10);


}




