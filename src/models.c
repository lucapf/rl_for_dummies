#include<stddef.h>
#define NUM_CELLS 9
#define CELL_PER_ROW 3
#define SCORE_MAX_VALUE 100
#define SCORE_MIN_VALUE 0
#define PROGRESS_BAR_SIZE 30
#define TRUE 1
#define FALSE 0


typedef struct {
  size_t progress_bar_size;
  float items_per_block;
  int current_index;
  int items;
  char *content;
  unsigned char is_valid; /* positive = is valid, 0 is not valid*/

}progress_bar;

struct tris{
 char id[NUM_CELLS+1];
 float weights[NUM_CELLS];
};

/*
 * stores a list of scenario
 */
struct linked_scenarios{
  struct tris *s; //current scenario
  struct linked_scenarios * next; //pointer to tne next scenario
  char player; //who made the move
  short next_move;
  short incremented;
};

