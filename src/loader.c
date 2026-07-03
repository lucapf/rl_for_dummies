#include <stdio.h>
#include <dirent.h>
#include <regex.h>
#include <stdlib.h>
#include "utils.c"


struct tris* report_line_to_tris(char * line){
        // cell_id
        struct tris * t = malloc(sizeof(struct tris));
        int i=0;
        for (;i<NUM_CELLS; i++){ t->id[i] =line[i]; }
        i++; //';'
        //weights
        char sw[10];
        for (int j=0;j<10;j++) sw[j] =' ';
        int swi = 0; int wc = 0; 
        while (wc < NUM_CELLS) {
          if (line[i]==','){
            t->weights[wc++] = atoi(sw); swi =0;
            for (int j=0;j<10;j++) sw[j] = ' ';
          }else{
            sw[swi++]=line[i];
          }
          i++;
        }
        return t;
}

struct linked_scenarios * load_file(char * file_name, struct linked_scenarios *root_scenario){
  FILE *f = fopen(file_name, "r");
  char line[100];
  while (1){
    char *res = fgets(line, 100,f);
    if (res == NULL) break;
    struct tris *t = report_line_to_tris(line);
    struct linked_scenarios *f_scenario = 
          malloc(sizeof(struct linked_scenarios));
    f_scenario->s = t;
    f_scenario->next = root_scenario;
    root_scenario = f_scenario;
  }
  return  root_scenario;
}

char** read_current_dir(void){
  char ** file_list = malloc(100*sizeof(char *)); // 100 string max
  char *pattern= "game-[0-9]+.txt";
  regex_t regex;
  int rc;
  rc = regcomp(&regex,pattern, REG_EXTENDED) ;
  if (rc != 0){
    printf("pattern %s", pattern);
    return NULL;
  }
  DIR *current_dir = opendir(".");
  if (current_dir == NULL) return NULL;
  struct dirent *entry;
  int i=0;
  while ((entry = readdir(current_dir))!=NULL){
    if  (entry->d_type == DT_REG){
      rc = regexec(&regex, entry->d_name, 0, NULL, 0);
      if (rc == 0) {
        file_list[i++]= entry->d_name;
      } 
    }
  }
  regfree(&regex);
  return file_list;
};

int linked_scenario_size(struct linked_scenarios * root){
  int i =0;
  struct linked_scenarios *cursor = root;
  while (cursor->next != NULL){
    i++; 
    cursor = cursor->next;
  }
  return i;
};

struct tris ** build_list(struct linked_scenarios * l, int size){
  struct tris ** list;
  struct linked_scenarios * cursor;
  cursor = l;
  list = malloc(size * sizeof(struct tris));
  int i=0;
  while(cursor->next != NULL){
    list[i++] = cursor->s;
    cursor = cursor->next;
  }
  return list;
}
 
int sum_weight(struct tris * t){
  int w =0;
  for (int i=0;i<NUM_CELLS;i++) w +=t->weights[i];
  return w; 
}

struct linked_scenarios * remove_duplicates( struct linked_scenarios * root_scenario){
  struct  linked_scenarios *normalized, *root_cursor;
  struct tris **list_of_tris;
  normalized = malloc(sizeof(struct linked_scenarios));
  normalized = build_root_scenario(normalized);
  int size = linked_scenario_size(root_scenario);
  list_of_tris = build_list(root_scenario, size);
  int index = 0;
  while (index < size-1) {
    if (index % 100 ==0) { 
      printf("\r\033[K normalized: %d/%d", index, size);
      fflush(stdout);
    }
    index++;
    struct tris * item = list_of_tris[index];
    if (item == NULL){
      printf("unexpected! at index %d got null", index);
    }
    if (get_scenario_by_id(normalized, item->id) !=NULL){
      continue;
    }
     
    int offset = index +1;
    while(offset >=0){
      offset = tris_lookup(list_of_tris, item->id, offset, size);
      if(offset >=0){
        if (sum_weight(item) < sum_weight(list_of_tris[offset]) ){
          item = list_of_tris[offset];
        }
        offset++;
      }
    }
    struct linked_scenarios *new_item = malloc(sizeof(struct linked_scenarios));
    new_item->s = item;
    new_item->next = normalized;
    normalized = new_item;
    root_cursor = root_cursor->next;
  }
  printf("normalized\n");
  return normalized;
}

int  load(){
  printf("start loading scenarios"); fflush(stdout);
  char ** l = read_current_dir();
  struct linked_scenarios *root_scenario;
  root_scenario = malloc(sizeof(struct linked_scenarios));
  root_scenario = build_root_scenario(root_scenario);
  int i = 0;
  while(l[i] != NULL)
    root_scenario = load_file(l[i++], root_scenario);
  struct linked_scenarios * current_scenario = root_scenario;
  i=0;
  while(current_scenario->next != NULL) {
    // print_game(current_scenario->s, PRINT_PLACEHOLDER );
    current_scenario = current_scenario->next;
    i++;
  }
  printf("...copleted"); fflush(stdout);
  root_scenario = remove_duplicates(root_scenario);
  print_linked_scenario(root_scenario, 100);
  printf("load completed"); fflush(stdout);
  return 0;
  // int n =0;
  // while(root_scenario->next != NULL) {
  //   print_game(root_scenario->s, PRINT_PLACEHOLDER );
  //   n++;
  //   root_scenario = root_scenario->next;
  // }

  // printf("loaded %d scenarios after normalization %d", i, n);
}
