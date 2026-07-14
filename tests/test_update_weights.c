#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../src/loader.c"
#include "../src/game.c"
 
char *lines[] = {
  "     OX  ;2.025,1.967,2.018,2.029,2.050,0.000,0.000,1.977,2.049,incremented 1;",
  "   O OX  ;1.995,1.994,1.997,0.000,2.012,0.000,0.000,2.001,2.000,incremented 0;",
  " X O OX  ;1.990,0.000,2.006,0.000,2.004,0.000,0.000,1.999,2.001,incremented 0;",
  "XX O OX X;0.000,0.000,2.005,0.000,2.000,0.000,0.000,1.999,0.000,incremented 0;"
};


/*
 * builds a game list the way play_a_game does: newest move first,
 * the head being the terminal board (no move made from it).
 * chronological order of the moves: ls0 (X at 3), ls1 (O at 1), ls2 (X at 0).
 */
struct linked_scenarios * build_linked_list(){
  struct linked_scenarios *ls0 = malloc(sizeof(struct linked_scenarios));
  ls0->s = report_line_to_tris(lines[0]);
  ls0->player = MOVE_X;
  ls0->next_move = 3;
  ls0->incremented = 0;

  struct linked_scenarios *ls1 = malloc(sizeof(struct linked_scenarios));
  ls1->s = report_line_to_tris(lines[1]);
  ls1->player = MOVE_O;
  ls1->next_move = 1;
  ls1->incremented = 0;

  struct linked_scenarios *ls2 = malloc(sizeof(struct linked_scenarios));
  ls2->s = report_line_to_tris(lines[2]);
  ls2->player = MOVE_X;
  ls2->next_move = 0;
  ls2->incremented = 0;

  struct linked_scenarios *ls3 = malloc(sizeof(struct linked_scenarios));
  ls3->s = report_line_to_tris(lines[3]);
  ls3->player = MOVE_O;
  ls3->next_move = -3;
  ls3->incremented = 0;

  ls3->next = ls2;
  ls2->next = ls1;
  ls1->next = ls0;
  ls0->next = NULL;

  return ls3;
}

void verify_weights_match(float *actual, float *expected ){
  for (int i =0; i<NUM_CELLS;i++) 
    assert(actual[i] == expected[i]);
}

int test_load(){

 struct tris *l1 =  report_line_to_tris(lines[0]);
 // struct tris *l3 =  report_line_to_tris(lines[2]);
 struct tris *l4 = report_line_to_tris(lines[3]);
 assert (strcmp(l1->id, "     OX  " ) ==0);
 assert (strcmp(l4->id, "XX O OX X" ) ==0);
 printf("check weights ");fflush(stdout);
 verify_weights_match(l1->weights,(float[]){2.025,1.967,2.018,2.029,2.050,0.000,0.000,1.977,2.049});
 printf("..done\n");

 free(l1);
 free(l4);
 return 0;

}
void assert_close(float actual, float expected){
  if (fabsf(actual - expected) > 0.001f){
    printf("expected %.3f got %.3f\n", expected, actual);
    assert(0);
  }
}

/*
 * X wins. update_weights receives the list starting from the winning move
 * (the terminal-board head is skipped, as play_a_game does).
 * expected: winning move +3, then the reward shrinks by 0.8 per step back;
 * O's move is punished and clipped at the weight floor.
 */
int test_reward_win(){
  struct linked_scenarios *game = build_linked_list();
  int r = update_weights(game->next, 'W', MOVE_X);
  assert(r == 0);

  struct linked_scenarios *ls2 = game->next;      /* X at 0: winning move */
  assert_close(ls2->s->weights[0], 1.990f + 3.0f);
  assert(ls2->incremented == 1);

  struct linked_scenarios *ls1 = ls2->next;       /* O at 1: punished */
  assert_close(ls1->s->weights[1], WEIGHT_FLOOR); /* 1.994 - 2.4 clips to floor */
  assert(ls1->incremented == 0);

  struct linked_scenarios *ls0 = ls1->next;       /* X at 3: earlier move */
  assert_close(ls0->s->weights[3], 2.029f + 3.0f * 0.8f * 0.8f);
  assert(ls0->incremented == 1);

  free_scenarios(game, TRUE);
  return 0;
}

/*
 * pair: every move gets the pair reward, discounted going back in time.
 */
int test_reward_pair(){
  struct linked_scenarios *game = build_linked_list();
  int r = update_weights(game->next, 'P', MOVE_X);
  assert(r == 0);

  struct linked_scenarios *ls2 = game->next;
  assert_close(ls2->s->weights[0], 1.990f + 1.0f);

  struct linked_scenarios *ls1 = ls2->next;
  assert_close(ls1->s->weights[1], 1.994f + 0.8f);
  assert(ls1->incremented == 1);

  struct linked_scenarios *ls0 = ls1->next;
  assert_close(ls0->s->weights[3], 2.029f + 0.64f);

  free_scenarios(game, TRUE);
  return 0;
}

/*
 * the deterministic choice must pick the empty cell with the highest
 * weight, ignoring the (zero-weighted) occupied cells.
 */
int test_deterministic_score(){
  struct tris *s = report_line_to_tris(lines[0]); /* best empty cell: 4 (2.050) */
  assert(cell_by_deterministic_score(s) == 4);
  free(s);
  return 0;
}

int main(void){
  if (    test_load()   == 0
       && test_reward_win() == 0
       && test_reward_pair() == 0
       && test_deterministic_score() == 0
      )
    return 0;
  else
    return 1;
}
