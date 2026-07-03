#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.c"


#define MAX_ITERATION 9 
#define MOVE_X 'X'
#define MOVE_O 'O' 

struct linked_scenarios *all_scenarios; 

short  next_move(struct tris *s ){
  int score = 0;
  for (int i = 0 ; i < NUM_CELLS; i++) score +=s->weights[i];
  if (score == 0) return -1;
  int randomized_score = rand() % score;
  if (randomized_score < 2) randomized_score = 2;
  short next_move_index = 0;
  for (int i =0; i<NUM_CELLS; i++){
    randomized_score -= s->weights[next_move_index];
    if (randomized_score <= 0 ) break;
    next_move_index++;
  }
  if (randomized_score > 0) return -2;
  if (s->id[next_move_index]!= ' ') return -3;
  return next_move_index;
};

/*
 * checks if the game has ended and how.
 * possible exit codes:
 * 'C': Continue - game has not ended
 * 'P': Pair - all cells filled nobody win
 * 'W': player that did the last move, win
 */
char game_ended(struct tris * t){
  int score = 0;
  for (size_t i = 0; i< NUM_CELLS ; i++) 
    score += t->weights[i];
  if (score == 0 ) return 'P';
  char *board = t->id;

  /* looking horizontally and vertically*/
  for (int offset = 0; 
      offset<(int)NUM_CELLS/CELL_PER_ROW; offset++){
    int row_offset = offset * CELL_PER_ROW;
    if (board[row_offset]==board[row_offset+1]
        && board[row_offset]==board[row_offset+2]
        && board[row_offset] != ' ')
      return 'W';
    if (board[offset]==board[offset+CELL_PER_ROW] 
        && board[offset]==board[offset+2*CELL_PER_ROW]
        && board[offset] != ' ')
      return 'W';
  }
  /* cross */
  if (board[0] == board[4] && board[0] ==board[8] && board[0]!= ' ') 
    return 'W';
  if ((board[2] == board[4]) && (board[2] ==board[6]) && board[2] != ' ')
    return 'W';
  return 'C';
};

void update_weights(struct linked_scenarios *game, short reward, char player){
  struct linked_scenarios *s = game;
  while (s->next != NULL ){
    if (s->s->id[s->next_move] !=' '){
      printf("error! -%s- next move index: -%d- is last? %s weights:",
          s->s->id, s->next_move, (s->next->s->id) );
      for (int i =0;i< NUM_CELLS;i++){
        printf("%d", s->s->weights[i]);
      }
      printf("\n");
    }else{
      if (s->player == player){
        s->s->weights[s->next_move] += reward;
        s->incremented += 1;
      }
    }
    s = s->next;
  }
};


void  free_scenarios(struct linked_scenarios * g){
  while (g->next != NULL){
    struct linked_scenarios * current_linked_scenario  = g;
    current_linked_scenario = g->next;
    free(g);
    g = current_linked_scenario;
  }
  free(g);
}

int is_next_move_valid(short next_move_index, struct  linked_scenarios *game){
  if (next_move_index == -1){
      printf(" undetected pair. =%s=", game->s->id);
    }
    if (next_move_index < 0 ) {
      printf(" NON Valid next move index. Abort %d\n", next_move_index);
      free_scenarios(game);
      return -1;
    }
    return 0;
}


struct linked_scenarios * build_and_link_scenario(char* new_cell_id, struct linked_scenarios *all_scenarios){
  struct linked_scenarios *new_linked_scenario_item = malloc(sizeof(struct linked_scenarios));
    new_linked_scenario_item->s = build_scenario(new_cell_id);
    new_linked_scenario_item->next = all_scenarios;
    return new_linked_scenario_item;
}

short user_input(struct tris *scenario, short iteration){
  short next_move_index =0;
  short is_valid_input = 1;
  while (is_valid_input != 0){
    (iteration == 0)
      ? printf("you start put a nuber between 0 and 8: ")
      :printf ("your tourn (you are player %c): ", MOVE_X);
    int ret = scanf("%hd", &next_move_index);
    if (ret <0) {
      printf("error reading user input. Abort %d", ret);
      return -1;
    }
    if (next_move_index < 0 && next_move_index <= NUM_CELLS){
      printf("invalid input %hd. Use numbers between 0 and 8\n", next_move_index);
      is_valid_input = 1;
    } else if (scenario->id[next_move_index] != ' '){
      printf("Invalid input %hd. Cell already taken! %c\n", next_move_index,scenario->id[next_move_index] );
      is_valid_input = 1;
    }else { 
      is_valid_input = 0;
    }
    printf(" you choose %hd", next_move_index);
  }
  return next_move_index;
}

void play_interacting_game(struct tris *scenario){
  char player = rand()%2 ==0 ? MOVE_O:MOVE_X;
  short next_move_index=0; 
  printf ("\033[2J\033[H \n Wonderful! now that I understood how tris works, let's start a new game!\n");
  for ( short i=0; i< MAX_ITERATION;i++){
    if (player == MOVE_O){
      next_move_index = next_move(scenario);
    }else{
      next_move_index = user_input(scenario, i);
    }
    char * cell_id =build_cell_id(scenario->id, next_move_index, player);
    printf(" cell id : %s\n", cell_id);
    scenario = get_scenario_by_id(all_scenarios,cell_id );
    if (scenario == NULL) scenario = build_scenario(cell_id);
    printf("\033[2J\033[H");
    print_game(scenario, PRINT_CELL_ID);
    free(cell_id);
    char game_state =game_ended(scenario);
    if (game_state == 'P'){
      printf("game ended PAIR\n");
      break;
    }else if (game_state == 'W'){
      printf ("player %c WIN!\n", player);
      break;
    }
    player = player == MOVE_O?MOVE_X:MOVE_O;
  }
};


struct tris *play_a_game(struct tris *root_scenario){
  struct linked_scenarios *game = malloc(sizeof(struct linked_scenarios));
  game->player = ' ';
  game->s = root_scenario;
  game->next = NULL;
  
  char game_state ='C';
  struct tris *new_tris;
  char player = rand()%2 ==0 ? MOVE_O:MOVE_X; //first move
  for (int i =0; i< MAX_ITERATION; i++){
    short next_move_index = next_move(game->s);
    if (is_next_move_valid(next_move_index, game)) {return NULL;}
    char *new_cell_id = build_cell_id(game->s->id, next_move_index, player);
    new_tris = get_scenario_by_id(all_scenarios, new_cell_id);
// scenario not found --> let's add it to the static all_sceanrios structure
    if (new_tris == NULL){
      all_scenarios = build_and_link_scenario(new_cell_id,all_scenarios );
      new_tris = all_scenarios->s;
    }
    game_state = game_ended(new_tris);
    game->next_move = next_move_index;
    game->player = player;
    game = add_game_to_scenario(new_tris, game);
    if (game_state == 'P' || game_state == 'W' )  break; 
    next_move_index = -1;
    player = player==MOVE_X?MOVE_O:MOVE_X;
    free(new_cell_id);
  }
  if (game_state == 'P' || game_state == 'W' ){
    update_weights(game->next, game_state =='P'?1:3, player);
  }
  free_scenarios(game);
  return  new_tris;

}


