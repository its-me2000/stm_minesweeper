//
//  game.h
//  minesweeper1
//
//  Created by Eugenijus Margalikas on 25.09.12.
//  Copyright (c) 2012 NASA. All rights reserved.
//

#ifndef minesweeper1_field_h
#define minesweeper1_field_h


typedef enum {
    EMPTY   =   0,
    ONE     =   1,
    TWO     =   2,
    THREE   =   3,
    FOUR    =   4,
    FIVE    =   5,
    SIX     =   6,
    SEVEN   =   7,
    EIGHT   =   8,
    MINE    =   9,
    BAD_VALUE} tile_value;

typedef enum {
    CLOSED      =   0,
    MARKED      =   1,
    UNMARKED    =   2,
    OPEN        =   3,
    BAD_STATUS} tile_status;

/*

 EMPTY   =   1,         0x01    0000 0000 0000 0001b
 ONE     =   2,         0x02    0000 0000 0000 0010b
 TWO     =   4,         0x04    0000 0000 0000 0100b
 THREE   =   8,         0x08    0000 0000 0000 1000b
 FOUR    =   16,        0x10    0000 0000 0001 0000b
 FIVE    =   32,        0x20    0000 0000 0010 0000b
 SIX     =   64,        0x40    0000 0000 0100 0000b
 SEVEN   =   128,       0x80    0000 0000 1000 0000b
 EIGHT   =   256,       0x0100  0000 0001 0000 0000b
 MINE    =   512,       0x0200  0000 0010 0000 0000b
 
 CLOSED      =   ,  0x1000  0001 0000 0000 0000b
 MARKED      =   ,  0x2000  0010 0000 0000 0000b
 UNMARKED    =   ,  0x4000  0100 0000 0000 0000b
 OPEN        =      0x8000  1000 0000 0000 0000b
 
*/

typedef struct {
    tile_value value;
    tile_status status;
} tile;

int create_field(int unsigned height,unsigned int width,unsigned int mines);   //sozdanie polia s razmerami i kolichestvom min
int fill_field(void);  //zapolnenie polia znachenijami
int reset_field(void);
int add_mines(void);
int destroy_field(void);                                     //ochischenie pamiati
tile_value get_tile_value(unsigned int height,unsigned int width);                //otkrytie kletki polia
tile_status get_tile_status(unsigned int height,unsigned int width);                //otkrytie kletki polia
void close_field(void);  //zakrytie polia
tile mark_tile(unsigned int height,unsigned int width,tile_status status); //izmenenie statusa kletki polia
tile get_tile(unsigned int height, unsigned int width);


#endif
