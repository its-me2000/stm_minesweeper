/*
 * gameUI.h
 *
 *  Created on: Jan 26, 2021
 *      Author: Eugenijus
 */

#ifndef __GAMEUI_H

#include <LCD9341_SPI.h>
#include "field.h"
#include "minegame.h"



void SplashScreen();
void DrawTile(uint16_t x, uint16_t y);
void DrawTiles();
void DrawButton(uint16_t posX, uint16_t posY, uint16_t sizeX, uint16_t sizeY, uint8_t state);
void ClickOnField(uint16_t x, uint16_t y, mouse_button mb);
void UIClick(uint16_t x, uint16_t y);
void ClickOnReset();
void UpdateResetButton();
void MakeResetButton(uint16_t x, uint16_t y);
void MakeTimer(uint16_t x, uint16_t y);
void UpdateTimer();
void SetTimer(uint16_t t);

void MakeCounter(uint16_t x, uint16_t y);
void UpdateCounter();
void SetTimer(uint16_t t);
void NextTimer();

void MakeMarkButton(uint16_t x, uint16_t y);
void ClickOnMark();
void UpdateMarkButton();

void UIStart();

void TEST_DrawTile();
void TEST_DrawField();

#endif /* __GAMEUI_H */

