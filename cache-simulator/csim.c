#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAXLEN 256

typedef struct set set_t;
typedef struct cache cache_t;
typedef struct cache_line cache_line_t;


//Globals

int hits = 0;
int misses = 0;
int evictions = 0;
int counter;

struct set {

  long int tag;
  cache_line_t **cache_lines;
  int num_lines;

};

struct cache_line {

  long int tag;
  int valid;;
  long int **block;
  int time_accessed;

};

struct cache {

  int num_sets;
  int lines_per_set;
  int block_size;
  set_t **sets;

};

/*   PROTOTYPES   */

void usage();
cache_t *make_cache(int set_bits, int num_lines, int block_bits);
void perform_operation(cache_t *c, char operation, int size, long unsigned int address, int set_bits, int block_bits, int verbose);
int least_recently_used(cache_t *c, set_t *s);
void print_result();
/*               */

void print_result(){

  printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);

}

int least_recently_used(cache_t *c, set_t *s){

  // get the time_accessed of first cache_line_t
  int lru = s -> cache_lines[0] -> time_accessed;
  cache_line_t *cl;
  cache_line_t *rc;
  int i;
  int ret_val = 0;

  for (i = 0; i < c -> lines_per_set; i++){

    // get a cache_line_t
    cl = s -> cache_lines[i];

    if (cl -> time_accessed <= lru){
      lru = cl -> time_accessed;
      rc = cl;
      ret_val = i;

    }
  }

  return ret_val;

}


void perform_operation(cache_t *c, char operation, int size, long unsigned int address, int set_bits, int block_bits, int verbose){

  int longSize = (((int)sizeof(unsigned long int)) * 8);
  int numberOfTagBits = (longSize - (set_bits + block_bits));
  unsigned long int cl_tag = address >> (set_bits + block_bits);
  unsigned long int shiftedAddress = address << numberOfTagBits;
  unsigned long int set_idx = shiftedAddress >> (numberOfTagBits + block_bits);
  int i;
  cache_line_t *cl;
  int cache_idx = -1;
  set_t *set;

  counter++;

  // map to the correct set
  set = c -> sets[set_idx];

  for (i = 0; i < c -> lines_per_set; i ++){

    cl = set -> cache_lines[i];

    //cache hit
    if ((cl -> tag == cl_tag) && (cl -> valid == 1)){
      cl -> time_accessed = counter;
      printf("%c %lx, %d hit", operation, address, size);
      if (operation == 'M'){
	hits += 2;
	printf("hit\n");
      }else {
	printf("\n");
	hits++;
      }
      return;
    }
    }
  // if no hit, then automatic miss
  misses++;
  if (operation == 'M'){
    printf("(from M) hit ");
    hits++;
  }
  printf("%c %lx, %d miss", operation, address, size);

  for (i = 0; i < c -> lines_per_set; i++){

    cl = set -> cache_lines[i];

    if( cl -> valid == 0){

      cache_idx = i;
      break;
    }

  }

  // no vacant spot found
  if (cache_idx < 0) {
    printf(" eviction\n");
    evictions++;
    cache_idx = least_recently_used(c, set);
  }
  printf("\n");

  cl = set -> cache_lines[cache_idx];
  cl -> valid = 1;
  cl -> tag = cl_tag;
  cl -> time_accessed = counter;

  return;
}


void usage(){

  printf("commands should follow the format: \n");
  printf("\n ./executable -v -s #set_index_bits -E #Associativity -b #block_bits -t trace_file_name\n");

}

cache_t *make_cache(int set_bits, int num_lines, int block_bits){

  cache_t *c = (cache_t *) malloc(sizeof(cache_t));
  int i;
  int j;

  c -> num_sets = 1 << set_bits;
  c -> block_size = 1 << block_bits;
  c -> lines_per_set = num_lines;
  set_t **set = (set_t **) malloc(sizeof(set_t *) * c -> num_sets);
  c -> sets = set;

  for (i = 0; i < (c -> num_sets); i++){


    set_t *ts = (set_t *) malloc(sizeof(set_t));
    c -> sets[i] = ts;
    cache_line_t **c_lines = (cache_line_t **) malloc(num_lines * sizeof(cache_line_t *));
    c -> sets[i] -> cache_lines = c_lines;

    for (j = 0; j < num_lines; j++){
      long int **b = (long int **) malloc(c -> block_size * sizeof(long int *));
      cache_line_t *ct = (cache_line_t *) malloc(sizeof(cache_line_t));
      c -> sets[i]-> cache_lines[j] = ct;
      ct -> valid = 0;
      ct -> tag = -1;
      ct -> time_accessed = 0;
      ct -> block = b;
    }

  }

  return c;

}

int main(int argc, char *argv[]) {

  char *filename;
  FILE *in_file;
  int verbose = 0;

  if (strcmp(argv[1],"-v") == 0)
    verbose = 1;

  unsigned int set_bits;
  int num_lines;
  int block_bits;
  char operation;
  long unsigned int address;
  int size;

  if (argc == 0 || argc > 10) {

    usage();
    exit(0);

    }
  if (verbose){

  if (strcmp(argv[2], "-s") != 0 || strcmp(argv[4], "-E") != 0 || strcmp(argv[6], "-b") != 0 || strcmp(argv[8], "-t") != 0) {
  usage();
  exit(0);
  }

  set_bits = atoi(argv[3]);
  num_lines = atoi(argv[5]);
  block_bits = atoi(argv[7]);

  if (strlen(argv[9]) < 1){

    printf("must enter a file to read from\n");
    usage();
    exit(0);

    }
  }
  else {

  if (strcmp(argv[1], "-s") != 0 || strcmp(argv[3], "-E") != 0 || strcmp(argv[5], "-b") != 0 || strcmp(argv[7], "-t") != 0) {

    usage();

  }

  set_bits = atoi(argv[2]);
  num_lines = atoi(argv[4]);
  block_bits = atoi(argv[6]);

  if (strlen(argv[8]) < 1){

    printf("must enter a file to read from\n");
    usage();
    exit(0);

    }
  }


  cache_t *c = make_cache(set_bits, num_lines, block_bits);

    filename = argv[argc- 1];

    if ((in_file = fopen(filename, "r")) == NULL){

      printf("File not Found: %s\n", filename);
      exit(0);

      }

    while(fscanf(in_file, " %c %lx, %d", &operation, &address, &size) > 0){

      if (operation == 'M' || operation == 'S' || operation == 'L')
	perform_operation(c, operation, size, address, set_bits, block_bits, verbose);

    }

    print_result();
    printSummary(hits, misses, evictions);
    return 0;
}
