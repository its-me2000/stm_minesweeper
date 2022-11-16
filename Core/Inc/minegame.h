//
//  minegame.h
//  minesweeper1
//
//  Created by Eugenijus Margalikas on 25.10.12.
//  Copyright (c) 2012 NASA. All rights reserved.
//

#ifndef minesweeper1_minegame_h
#define minesweeper1_minegame_h

//#include <time.h>
//#include "field.h"
typedef enum{
    LCLICK = 1,
    RCLICK = 0  } mouse_button;


/*
typedef enum {
    STOP    = 0,
    RUNNING = 1,
    PAUSE   = 2,
    WIN     = 3,
    LOST    = 4} game_status;
*/
struct _game_state {
    enum {
                STOP=0,
                RUNNING=1,
                PAUSE=2,
    			LOST,
				WIN}    status;
    //int mines_countdown;
    uint16_t tiles_opened_countdown;
    uint16_t field_height;
    uint16_t field_width;
    int mines_count;
    void (*draw_callback)(uint16_t,uint16_t) ;
    void (*win_callback)(void) ;
    void (*lost_callback)(void) ;
    void (*down_counter_callback)(void);
    void (*up_counter_callback)(void);

    //time_t start_time;
} game_state;



int new_game(uint16_t height,uint16_t width , uint16_t mines);
int start_game(void);
//int finnish_game(struct _game_status result);
//int pause_game(void);
int click(uint16_t height, uint16_t width, mouse_button mb);
//tile simple_click(uint16_t height, uint16_t width, mouse_button mb);
int init_game(void (_draw)(uint16_t height, uint16_t width), void (_win)(void), void (_lost)(void), void (_down)(void),void(_up)(void));


#endif
