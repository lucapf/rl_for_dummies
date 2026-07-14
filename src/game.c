#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.c"


#define MAX_ITERATION 9 
#define MOVE_X 'X'
#define MOVE_O 'O' 
#define BOUNDED_NOISE 100000
#define NOISE_RATIO 20  // % total call
#define WEIGHT_TOLERANCE 10
#define LIMIT (BOUNDED_NOISE * NOISE_RATIO / 100)
#define WEIGHT_CAP 200 //cannot go more than this
#define WEIGHT_FLOOR 0.1f //cannot go below this, so no move becomes impossible
#define WIN_REWARD 3.0f
#define PAIR_REWARD 1.0f
#define DISCOUNT_FACTOR 0.8f //each step back in time the reward shrinks by this

struct linked_scenarios *all_scenarios; 

short get_random_cell(char* id){
  int index =-1;
  while(1){
    index = bounded_rand(NUM_CELLS);
    if (id[index] == ' ') 
      return index;
  }
}

short cell_by_deterministic_score(struct tris *s){
  float max = 0;
  short max_index = -1;
  for (short i=0;i<NUM_CELLS;i++){
    if (s->id[i] != ' ') continue;
    if (max_index == -1 || s->weights[i] > max){
      max = s->weights[i];
      max_index = i;
    }
  }
  return max_index;
}

short cell_by_probabilistic_score(struct tris* s){
  if (s == NULL) {
    printf("cell_by_probabilistic_score: tris is NULL");
    return -1;
  }
  float score = 0;
  for (int i = 0 ; i < NUM_CELLS; i++)  score +=s->weights[i];
  if (score == 0) return -1;
  float randomized_score = bounded_rand(score);
  short next_move_index = 0;
  for (; next_move_index <NUM_CELLS; next_move_index ++){
    randomized_score -= s->weights[next_move_index];
    if (randomized_score < 0 ) break;
  }
  if (randomized_score > 0) {
    return -2;
  }
  if (s->id[next_move_index]!= ' ') { 
    printf("\n\nnext move index: %d , char: %c\n",
            next_move_index,s->id[next_move_index]);
    return -3;
  }
  return next_move_index;
}

short  next_move(struct tris *s ){

 int  noise = bounded_rand(BOUNDED_NOISE);
  if (noise <= LIMIT){
    return get_random_cell(s->id);
  }
  return cell_by_probabilistic_score(s);
};

/*
 * checks if the game has ended and how.
 * possible exit codes:
 * 'C': Continue - game has not ended
 * 'P': Pair - all cells filled nobody win
 * 'W': player that did the last move, win
 */
char check_game_status(char *board){
  int score = 0;
  for (size_t i = 0; i< NUM_CELLS ; i++) 
    if (board[i] == ' ') score +=1;
  if (score == 0 ) return 'P';

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


void dump_struct_error(struct linked_scenarios *s){
  printf("error! -%s- next move index: -%d- is last? %s weights:",
        s->s->id, s->next_move, (s->next->s->id) );
    for (int i =0;i< NUM_CELLS;i++){
      printf("%.2f", s->s->weights[i]);
    }
    printf("\n");
}

/*
 * `game` is newest-first: the head is the move that ended the game,
 * following ->next walks back in time to the opening move.
 * The reward starts at full strength on the final move and shrinks by
 * DISCOUNT_FACTOR at every step back, so the move that ended the game
 * gets the most credit (or blame) and the opening move the least.
 */
int update_weights(struct linked_scenarios *game, char game_state, char player){
  float reward = game_state=='P'?PAIR_REWARD:WIN_REWARD;
  struct linked_scenarios *cursor = game;
  while (cursor!= NULL ){
    if ( (cursor->s->id[cursor->next_move] !=' ')
        || (cursor-> next_move < 0)
        || (cursor-> next_move >= NUM_CELLS) ) {
        dump_struct_error(cursor);
        return -1;
    }

    if ((game_state== 'W' && cursor->player == player) || game_state== 'P'){
      cursor->s->weights[cursor->next_move] += reward;
      cursor->incremented += 1;
    }else if(game_state == 'W' && cursor->player != player){
      cursor->s->weights[cursor->next_move] -= reward;
      if  (cursor->s->weights[cursor->next_move] < WEIGHT_FLOOR )
        cursor->s->weights[cursor->next_move] = WEIGHT_FLOOR;
    }

    if (cursor->s->weights[cursor->next_move] > WEIGHT_CAP)
      cursor->s->weights[cursor->next_move] = WEIGHT_CAP;
    reward *= DISCOUNT_FACTOR;
    cursor= cursor->next;
  }
  return 0;
};




int is_next_move_valid(short next_move_index, struct  linked_scenarios *game){
  if (next_move_index == -1){
      printf(" undetected pair. =%s= \n", game->s->id);
    }
    if (next_move_index < 0 ) {
      printf(" NON Valid next move index. Abort %d\n", next_move_index);
      free_scenarios(game, FALSE);   /* game list ->s are shared with all_scenarios */
      return -1;
    }
    return 0;
}


struct linked_scenarios * build_and_link_scenario(char* new_cell_id, struct linked_scenarios *all_scenarios){
  struct linked_scenarios *new_item = malloc(sizeof(struct linked_scenarios));
    new_item->s = build_scenario(new_cell_id);
    new_item->next = all_scenarios;
    return new_item;
}

short user_input(struct tris *scenario, short iteration){
  short next_move_index =0;
  short is_valid_input = 1;
  while (is_valid_input != 0){
    (iteration == 0)
      ? printf("you start put a nuber between 0 and 8: ")
      :printf ("your tourn (you are player %c): ", MOVE_X);
    int ret = scanf("%hd", &next_move_index);
    if (ret == EOF) {
      printf("error reading user input (EOF). Abort\n");
      return -1;
    }
    if (ret != 1) {
      /* conversion failed: bad chars are still in stdin, discard the line */
      int c;
      while ((c = getchar()) != '\n' && c != EOF);
      printf("invalid input. Use numbers between 0 and 8\n");
      is_valid_input = 1;
      continue;
    }
    if (next_move_index < 0 || next_move_index >= NUM_CELLS){
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
      next_move_index = cell_by_probabilistic_score(scenario);
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
    char game_state =check_game_status(scenario->id);
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

/*
 * plays a single round. Choose max 9 random moves and checks each iteration 
 * looking for a pair or winner.
 * keep two list
 * all_scenarios = linked list with all possible scenarios
 * game = contains single 
 */
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
    if (is_next_move_valid(next_move_index, game)) return NULL;
    char *new_cell_id = build_cell_id(game->s->id, next_move_index, player);
    game_state = check_game_status(new_cell_id);
   
    new_tris = get_scenario_by_id(all_scenarios, new_cell_id);
    // scenario not found --> let's add it to the static all_sceanrios structure
    if (new_tris == NULL){
      struct linked_scenarios * l = malloc(sizeof(struct linked_scenarios));
      l->s = build_scenario(new_cell_id); 
      l->next =all_scenarios;
      all_scenarios = l;
      new_tris =  l->s;
    }

    game->next_move = next_move_index;
    game->player = player;

    game = add_game_to_scenario(new_tris, game);

    free(new_cell_id);   /* already copied by build_scenario; free before the break */
    if (game_state == 'P' || game_state == 'W' )  break;
    player = player==MOVE_X?MOVE_O:MOVE_X;
  }

  if (game_state == 'P' || game_state == 'W' ){
    if (update_weights(game->next, game_state, player)<0) {
      printf("ERROR update weights\n");
      return NULL;
    }
  }
  free_scenarios(game, FALSE);
  return  new_tris;
}


