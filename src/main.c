
#include <stdio.h>
#include<time.h>
#include <unistd.h>
#include <locale.h>
#include "game.c"
#include "loader.c"
#include "utils.c"

#define MAX_GAMES 100000
/*
 * the game object stores all moves of the current game.
 * the scenarios list 
 * A scenario represent the last status of the game
 */
void print_help(char* app_name){
  printf(
"usage:  \n"
"%s [--run] | [--interactive]  \n"
" -r | --run x [y]: re-run from scratch the leanring path x time.\n" 
"                     optionally you can also specify the iteration inside the epic ( %d by default)\n"
"                     Save all scenarios in a game-<epic_id>.txt file.\n"
"                     Generate the consolidated version (consolidated.txt)  \n"
" -i | --interactive: load from the consolidated learning (consolidated.txt) and ask you to play \n"
" -l | --load       : load the consolidated leaning and print it\n"
" \n", app_name , MAX_GAMES
);
}

void interactive(){
  all_scenarios = build_root_scenario(all_scenarios);
  struct tris* root_node = all_scenarios->s;
  all_scenarios = load_file("game-100.txt", all_scenarios);
  char c = 'Y';
  while (c == 'Y' || c == 'y'){
    play_interacting_game(root_node);
    c = another_game(c);
  }
}

int play_epic(int games, int epic, int total_epic){
    srand(time(NULL));
    all_scenarios = build_root_scenario(all_scenarios);
    struct tris * root_scenario  = all_scenarios->s;
    progress_bar *pb =  progress_bar_init(games,"content [", "]");
    /* here start the game */
      
    int i =0;
    for (i =0 ; i< games; i++ ) {
      print_progress(i,  pb);
      play_a_game(root_scenario);
    }
    printf("\n\n");
    printf("\033[KSaving epic %d/%d", epic, total_epic);
    fflush(stdout);
    print_linked_scenario(all_scenarios, epic);
    free_scenarios(all_scenarios);
    printf("..Done!\033[2A");
    fflush(stdout);
    return 0;
};

void run_epics(int epics, int games){
  for(int i=0;i<7;i++) printf("\n");
  printf("\033[6A");
  for(int i=0; i< 20; i++) putchar('-');
  fflush(stdout);
  printf("\033[B\rLearning: epics: %'d | iteration per epics: %'d | total games to play %'d!", epics, games, epics*games);
  printf("\033[5B");
  fflush(stdout);
  progress_bar *p = progress_bar_init(epics, "Total [", "]");
  for (int e=1;e<=epics; e++){
    if (e>0) printf("\033[1B");   // cursor DOWN 1 row
    print_progress(e, p );
    printf("\033[A");
    int result = play_epic(games, e, epics);

    if (result<0) exit(result);
  }
  printf("\n\n\n\n");
};

int main(int argc, char *argv[]){
  setlocale(LC_NUMERIC, "");     // adopt the environment's locale
  if (argc == 1 ){
    print_help(argv[0]);
  }else{
    if (strcmp(argv[1],"--run") == 0  || strcmp(argv[1], "-r") == 0 ){
      int epics, games;
      if (argc <=3 ){
        printf("epics is required!\n");
        return -1;
      }
      sscanf(argv[2], "%d",&epics);
      if(argc == 4){ sscanf(argv[3], "%d",&games); }else{ games = MAX_GAMES; }
      run_epics(epics,games);
    };
    if (strcmp(argv[1],"--interactive")==0 || strcmp(argv[1], "-i")==0){
      interactive();
    };
    if (strcmp(argv[1],"--load")==0 || strcmp(argv[1], "-l")==0){
      int result = load();
      if (result <  0){
        printf("load failed  error code  %d\n", result);
      }else{
        printf("success!\n");
      }
    };
  }
  return 0;
}


