#include <stdio.h>
#include <dirent.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "utils.c"

#define MAX_WEIGHT_LENGHT 7

struct tris* report_line_to_tris(char * line){
  struct tris *t  = malloc(sizeof(struct tris));
  size_t line_length = strlen(line);
  size_t line_cursor=0;
  // copy cell_id
  for (;line_cursor<NUM_CELLS; line_cursor++) {
    t->id[line_cursor] =line[line_cursor]; 
  }
  t->id[NUM_CELLS] = '\0'; //termination for cell_id

  // weigths are slightly more complicated
  int weight_index=0;
  char num[MAX_WEIGHT_LENGHT]; // the ascii part of '123'->123
  int num_cursor = 0; //cursor to the ascii repres. of the num
  // scroll the line past the id
  for (;weight_index < NUM_CELLS && line_cursor<line_length ;line_cursor++){
    if (line[line_cursor] ==','){ // the number representation is complted
      t->weights[weight_index++] = atoi(num);
      for (int i=0;i < MAX_WEIGHT_LENGHT; i++) num[i]=' '; //clean num and restart
      num_cursor =0;
    }else if (line[line_cursor] >= '0' && line[line_cursor]<= '9') { //new digit
      num[num_cursor++]  = line[line_cursor];
    }
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
    current_scenario = current_scenario->next;
    i++;
  }
  root_scenario = remove_duplicates(root_scenario);
  print_linked_scenario(root_scenario, 100);
  printf("load completed"); fflush(stdout);
  return 0;
}
