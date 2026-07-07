#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/loader.c"
#include "../src/game.c"
 
char *lines[] = {
  "     OX  ;2,2,2,2,2,0,0,2,2,incremented 1;",
  "   O OX  ;2,2,2,0,2,0,0,2,2,incremented 1;",
  " X O OX  ;2,0,2,0,2,0,0,2,2,incremented 1;",
  "XX O OX X;0,0,2,0,2,0,0,2,0,incremented 1;"
};


struct linked_scenarios * build_linked_list(){

  struct linked_scenarios *ls0 = malloc(sizeof(struct linked_scenarios));
  ls0->s = report_line_to_tris(lines[0]);
  ls0->player = MOVE_X;
  ls0->next_move = 3;

  struct linked_scenarios *ls1 = malloc(sizeof(struct linked_scenarios));
  ls1->s = report_line_to_tris(lines[1]);
  ls1->player = MOVE_O;
  ls1->next_move = 1;
  

  struct linked_scenarios *ls2 = malloc(sizeof(struct linked_scenarios));
  ls2->s = report_line_to_tris(lines[2]);
  ls2->player = MOVE_X;
  ls2->next_move = 0;

  struct linked_scenarios *ls3 = malloc(sizeof(struct linked_scenarios));
  ls3->s = report_line_to_tris(lines[3]);
  ls3->player = MOVE_O;
  ls3->next_move = -3;

  ls3->next = ls2;
  ls2->next = ls1;
  ls1->next = ls0;
  ls0->next = NULL;

  return ls3;
}

void verify_weights_match(int *actual, int *expected ){
  for (int i =0; i<NUM_CELLS;i++) 
    assert(actual[i] == expected[i]);
}
int test_reward(){

  struct linked_scenarios *root_scenario= build_linked_list();
  struct linked_scenarios *curr_scenario = root_scenario;
  int r = update_weights(curr_scenario, 'P', MOVE_X);
  assert(r ==0);
  verify_weights_match(curr_scenario->s->weights,(int[]) {0,0,2,0,2,0,0,2,0});
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {3,0,2,0,2,0,0,2,2});
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {2,3,2,0,2,0,0,2,2});
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {2,2,2,3,2,0,0,2,2});
  free_scenarios(curr_scenario);
  free(root_scenario);
  root_scenario = build_linked_list();
  curr_scenario = root_scenario;
  r = update_weights(curr_scenario, 'W', MOVE_X);
  assert(r ==0);

  verify_weights_match(curr_scenario->s->weights,(int[]) {0,0,2,0,2,0,0,2,0}); //O
  printf("l\n");
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {5,0,2,0,2,0,0,2,2}); //X
  printf("l\n");
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {2,2,2,0,2,0,0,2,2}); //O
  printf("l\n");
  curr_scenario = curr_scenario->next;
  verify_weights_match(curr_scenario->s->weights,(int[]) {2,2,2,5,2,0,0,2,2}); //X
  printf("l\n");
  return 0;
}

int test_load(){

 struct tris *l1 =  report_line_to_tris(lines[0]);
 // struct tris *l3 =  report_line_to_tris(lines[2]);
 struct tris *l4 = report_line_to_tris(lines[3]);
 assert (strcmp(l1->id, "     OX  " ) ==0);
 assert (strcmp(l4->id, "XX O OX X" ) ==0);
 printf("check weights ");fflush(stdout);
 verify_weights_match(l1->weights,(int[]){2,2,2,2,2,0,0,2,2});
 printf("..done\n");

 free(l1);
 free(l4);
 return 0;

}

int main(void){
  if (    test_load()   ==0
       && test_reward() == 0
      ) 
    return 0; 
  else 
    return 1;
}
