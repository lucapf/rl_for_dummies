#pragma  once
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "models.c"

#define  DEFAUL_WEIGHT 2
#define  NO_WEIGHT 0 
#define  PRINT_CELL_ID 0
#define  PRINT_PLACEHOLDER 1

/* because rand doesn't really work */
int bounded_rand(int range){
  int divisor = RAND_MAX / range;  
  int limit   = range * divisor;  
  int r;
  do { r = rand(); } while (r >= limit);
  return r / divisor;
}

/* format a string in the form "something %d something else", 10
 * into "something 10 something else"
 */
char * string_decimal_format(char* string , int decimal){
  int size =  snprintf(NULL,0, string, decimal);
  char *result =  malloc(size+1);
  snprintf(result, size+1, string, decimal);
  return result;
}
struct tris *build_scenario(char *id){
  struct tris *scenario = malloc(sizeof(struct tris));
  strcpy(scenario->id,id );
  for (int i=0; i< NUM_CELLS; i++){
    scenario->weights[i] = id[i] == ' '?DEFAUL_WEIGHT:NO_WEIGHT;
  };
  return scenario;
};

char* build_cell_id(char* original_cell, unsigned char pos, char xo){
  char *new_str_id = malloc((size_t) NUM_CELLS+1);
  for (int i=0;i<NUM_CELLS; i++) new_str_id[i]=' ';
  for (int i = 0; i<NUM_CELLS; i++){
    if (original_cell[i]== '\0') break;
    new_str_id[i] = original_cell[i];
  }
  new_str_id[pos] = xo;
  new_str_id[NUM_CELLS] ='\0';
  return new_str_id;
};


struct linked_scenarios * build_root_scenario(struct linked_scenarios *all_scenarios){
    all_scenarios = malloc(sizeof(struct linked_scenarios));
    char  *root_id = "        ";
    char *id = build_cell_id(root_id, 0, ' '); //root node
    struct tris *root_scenario = build_scenario(id);
    all_scenarios->s = root_scenario;
    all_scenarios->next = NULL;
    all_scenarios->incremented = 0;
    all_scenarios->next_move = -1;
    return all_scenarios;
}



struct linked_scenarios *init_linked_scenarios(){

  struct linked_scenarios *new_scenario = malloc(sizeof(struct linked_scenarios));
  char  root_id = ' ';
  char *id = build_cell_id(&root_id,0, root_id); //root node
  struct tris *root_scenario = build_scenario(id);
  new_scenario->s = root_scenario;
  new_scenario->next = NULL;
  return new_scenario;

};


struct linked_scenarios *add_game_to_scenario( struct tris* scenario, 
                                               struct linked_scenarios *g){

  struct linked_scenarios *new_item = malloc(sizeof(struct linked_scenarios)); 
  new_item->s = scenario;
  new_item->player = g->player;
  new_item->next = g;
  return new_item;

};


/*return list of tree starting by the offset*/
int tris_lookup(struct tris** list_of_tris,char * id ,int offset, int size){
 for (int i=offset;i<size;i++){
   if (strcmp(list_of_tris[i]->id,id) == 0) return i;
 }
 return -1;

}
struct tris *get_scenario_by_id(struct linked_scenarios * all_scenarios, char *cell_id){
  struct linked_scenarios *cursor = all_scenarios;
  while (cursor->next != NULL){
    if (strcmp(cursor->s->id, cell_id) ==0){
       return cursor->s; 
    }
    cursor = cursor->next;
  }
  return NULL;
};


void print_game(struct tris *s, short print_mode ){
  printf("|------||------------------------||\n");
  printf("| moves|| weights                ||\n");
  printf("|------||------------------------||\n|");

  int offset  = 0;
  for (int i=0;i< 2*NUM_CELLS; i++){
    int weight_index = i - CELL_PER_ROW - offset;
    if( i % (2*CELL_PER_ROW) < CELL_PER_ROW){
      char placeholder[13]; 
      if(print_mode == PRINT_CELL_ID){
          snprintf(placeholder, sizeof placeholder ,"\033[32m%c\033[0m",i-offset+'0' );
      }else{
        placeholder[0]='~'; placeholder[1]='\0';
      }
      char index_value[2];
      index_value[0]=s->id[i - offset ];
      index_value[1]='\0';
      printf("%s ", s->id[i - offset] ==' '?placeholder:index_value);
    }else{
      printf("%8d",s->weights[weight_index]);
    }
    if ((i+1) % CELL_PER_ROW == 0) printf("||");
    if ((i+1) % ( 2*CELL_PER_ROW) == 0) printf("\n");
    if (((i+1) % (2* CELL_PER_ROW) == 0) 
      && (i < 2*NUM_CELLS -1) ) printf("|");
    if ((i+1) % (2*CELL_PER_ROW)==0) offset += CELL_PER_ROW;
  }
};

progress_bar *progress_bar_init(int items, char* prefix, char* postfix){
  progress_bar *p= malloc(sizeof(progress_bar));
  size_t prefix_size = strlen(prefix);
  size_t postfix_size = strlen(postfix);
  size_t postfix_start_index = PROGRESS_BAR_SIZE - postfix_size;
  if (prefix_size >= PROGRESS_BAR_SIZE-1){
    p->is_valid = 0;
    printf("prefix (%s) size exceed bar size: %d",prefix, PROGRESS_BAR_SIZE );
    return p;
  }

  if ((postfix_start_index - prefix_size) <= 0){
    p->is_valid = 0;
    printf("postfix (%s) size exceed bar size: %lu",postfix, 
        (PROGRESS_BAR_SIZE - prefix_size));
    return p;
  }
  p->progress_bar_size = PROGRESS_BAR_SIZE- prefix_size - postfix_size;
  p->items_per_block = (float) items / p->progress_bar_size;
  p->current_index = prefix_size;
  p->items = items;
  p->content = malloc((PROGRESS_BAR_SIZE+1)*sizeof(char));
  size_t i = 0;
  for (;i<prefix_size; i++) p->content[i] = prefix[i];
  for (;i<postfix_start_index; i++) p->content[i] = ' ';
  
  for (i=0;i < postfix_size ;i++) p->content[i + postfix_start_index] = postfix[i];
  p->content[p->progress_bar_size] = ' ';
  return p ;
}

void print_progress(int current_iteration, progress_bar *p){
    int block_to_add = 1;
    if (p->items_per_block < 1)
      block_to_add = floorf(1 / p->items_per_block);
    if (current_iteration == p->items) 
      block_to_add = p->items - p->progress_bar_size;

    // printf("current iteration %d - bs: %d - indx: %d - -%s-\n", current_iteration,p->items_per_block,  p->current_index, p->content);
    if ((p->items_per_block < 1) || (current_iteration % (int)floor(p->items_per_block)) ==0) {
      for (int i =0;i<block_to_add;i++) p->content[p->current_index++] ='#';
       printf("\r%s%%\033[K", p->content);
    }
    fflush(stdout);
} 

void print_linked_scenario(struct linked_scenarios *ls, int e){
  struct linked_scenarios* current_scenario = ls;
  char*  filename;
  filename = string_decimal_format("game-%d.txt",e );
  
  FILE *f = fopen(filename,"w");
  while (current_scenario->next != NULL){
    for (int i = 0; i< NUM_CELLS;i++)
      fprintf(f,"%c" ,current_scenario->s->id[i]);
    fprintf(f,";");
    for (int i = 0; i< NUM_CELLS;i++)
      fprintf(f,"%5d," ,current_scenario->s->weights[i]);
    
    fprintf(f,"incremented %d;\n", current_scenario->incremented);
    current_scenario = current_scenario->next;
  }
  fclose(f);
  free(filename);
};

char another_game(char c){
  do{
        printf("\rwanna play again [Y/N]?");
        fflush(stdout);
        int ret = scanf("%c",&c );
        if  (ret < 0){
          printf("error reading input. Abort");
          return -1;
          
        }
      }while (c !='Y' && c != 'y' && c != 'N' && c!='n');
  return c; 
};
