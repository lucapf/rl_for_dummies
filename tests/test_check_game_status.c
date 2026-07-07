#include <assert.h>
#include <stdio.h>
#include "../src/game.c"


int verify(char *g[], int winners[], int combinations){
  for (int i=0;i<combinations;i++){ 
    char result = check_game_status(g[i]); 
    if ( result != winners[i]){ 
      printf("failed -%s- expected %c got %c\n", g[i], result, winners[i]);
      return 1; 
    }
    printf("."); fflush(stdout);
  } 
  printf(".ok\n");
  return 0;
}

int game_same_row(){
 printf("same row");
 char *g[] = {
               "OOO      ",
               " OOO     ",
               "  OOO    ",
               "   OOO   ",
               "    OOO  ",
               "     OOO ",
               "      OOO"
             };
 int winners[] = {'W','C','C','W','C','C','W'};
 return verify(g,  winners,7);
}

int game_same_column(){
 printf("same column");
 char *g[] = {
               "O  O  O  ",
               " O  O  O ",
               "  O  O  O",
             };
 int winners[] = {'W','W','W'};
 return verify(g,  winners,3);
}

int game_crossed(){
 printf("game crossed");
 char *g[] = {
               "O   O   O",
               "  O O O  ",
             };
 int winners[] = {'W','W'};
 return verify(g,  winners,2);
}

int pair(){
 printf("pair");
  return verify((char*[]){ "OOOOOOOOO" },(int[]){'P'}, 1);
}

int main(void){
  if ( (game_same_row() == 0)  
       && (game_crossed() == 0)
       && (pair() == 0)
      ) return 1;
  return 0;
}
