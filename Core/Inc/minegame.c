//
//  minegame.c
//  minesweeper1
//
//  Created by Eugenijus Margalikas on 25.10.12.
//  Copyright (c) 2012 NASA. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include "minegame.h"
#include "field.h"

int click(uint16_t height, uint16_t width, mouse_button mb){

	//extern uint16_t field_height;
	//extern uint16_t field_width;

	int return_value=0;

	if (height>=game_state.field_height || width>=game_state.field_width) return 0;
	if(mb == LCLICK){
		switch (get_tile_status(height, width)) {
		case CLOSED:
		case UNMARKED:
			switch (mark_tile(height, width, OPEN).value) {
			case EMPTY:


				//callback - otrisovka (void)(uint16_t x, uint16_t y)
				return_value=1+
				click(height-1, width-1, mb)+
				click(height-1, width, mb)+
				click(height-1, width+1, mb)+
				click(height, width-1, mb)+
				click(height, width+1, mb)+
				click(height+1, width-1, mb)+
				click(height+1, width, mb)+
				click(height+1, width+1, mb);//tut obrabotka
				--game_state.tiles_opened_countdown;
				break;
			case MINE:
				//finnish_game(LOST);   // tut obrabotka

				game_state.status=LOST;
				// callback - LOST
				return_value=-1;
				break;
			case ONE:
			case TWO:
			case THREE:
			case FOUR:
			case FIVE:
			case SIX:
			case SEVEN:
			case EIGHT:
			default: // 1 2 3 4 5 6 7 8
				--game_state.tiles_opened_countdown;
				return_value=1;
				//callback - otrisovka (void)(uint16_t x, uint16_t y)
				break;
			}
			if(!game_state.tiles_opened_countdown){ //finnish_game(WIN);
				//callback - WIN
				game_state.status=WIN;
				return_value=-1;}
			break;

		default:    // OPEN, MARKED
			break;
		}
	}else{

		switch (get_tile_status(height, width)) {
		case CLOSED:
			mark_tile(height, width, MARKED);
			game_state.down_counter_callback();
			return_value=1;
			break;
		case MARKED:
			mark_tile(height, width, UNMARKED);
			game_state.up_counter_callback();
			return_value=1;
			break;
		case UNMARKED:
			mark_tile(height, width, CLOSED);
			return_value=1;
			break;
		default:    // OPEN
			break;
		}

	}
	if(return_value>0){
		game_state.draw_callback(height,width);
	}
	if(return_value == -1 && game_state.status==WIN) game_state.win_callback();
	if(return_value == -1 && game_state.status==LOST) game_state.lost_callback();

	return return_value; // -1 game finished, 0 unchanged, >0 opened
}

int new_game(uint16_t height, uint16_t width , uint16_t mines){



	if(create_field(height,width,mines)) return 1;

	fill_field();

	close_field();

	game_state.field_height=height;
	game_state.field_width=width;
	game_state.mines_count=mines;
	//game_state.mines_countdown=mines;

	game_state.tiles_opened_countdown=game_state.field_height*game_state.field_width-game_state.mines_count; //tiles to go

	game_state.status=PAUSE;

	return 0;


}

int start_game(void){
	game_state.status=RUNNING;
	//game_state.start_time=time(NULL);
	return 0;
}

int init_game(void (_draw)(uint16_t height, uint16_t width), void (_win)(void), void (_lost)(void),void(_down)(void),void(_up)(void)){
	game_state.draw_callback = _draw;
	game_state.win_callback = _win;
	game_state.lost_callback = _lost;
	game_state.down_counter_callback = _down;
	game_state.up_counter_callback = _up;
	return 0;
}


/*
int finnish_game(game_status result){
    status=result;
    return 0;
}

int pause_game(void){
    status=PAUSE;
    return 0;
}
 */




