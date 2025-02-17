

/*
          _____                   _______                   _____ _____
         /\    \                 /::\    \                 /\    \ /\    \
        /::\____\               /::::\    \               /::\    \ /::\    \
       /:::/    /              /::::::\    \             /::::\    \ /::::\    \
      /:::/    /              /::::::::\    \           /::::::\    \
/::::::\    \
     /:::/    /              /:::/~~\:::\    \         /:::/\:::\    \
/:::/\:::\    \
    /:::/____/              /:::/    \:::\    \       /:::/__\:::\    \
/:::/__\:::\    \
   /::::\    \             /:::/    / \:::\    \     /::::\   \:::\    \
\:::\   \:::\    \
  /::::::\    \   _____   /:::/____/   \:::\____\   /::::::\   \:::\    \
___\:::\   \:::\    \
 /:::/\:::\    \ /\    \ |:::|    |     |:::|    | /:::/\:::\   \:::\ ___\  /\
\:::\   \:::\    \
/:::/  \:::\    /::\____\|:::|____|     |:::|    |/:::/__\:::\   \:::|    |/::\
\:::\   \:::\____\
\::/    \:::\  /:::/    / \:::\    \   /:::/    / \:::\   \:::\  /:::|____|\:::\
\:::\   \::/    /
 \/____/ \:::\/:::/    /   \:::\    \ /:::/    /   \:::\   \:::\/:::/    /
\:::\   \:::\   \/____/
          \::::::/    /     \:::\    /:::/    /     \:::\   \::::::/    /
\:::\   \:::\    \
           \::::/    /       \:::\__/:::/    /       \:::\   \::::/    /
\:::\   \:::\____\
           /:::/    /         \::::::::/    /         \:::\  /:::/    /
\:::\  /:::/    /
          /:::/    /           \::::::/    /           \:::\/:::/    /
\:::\/:::/    /
         /:::/    /             \::::/    /             \::::::/    /
\::::::/    /
        /:::/    /               \::/____/               \::::/    / \::::/    /
        \::/    /                 ~~                      \::/____/ \::/    /
         \/____/                                           ~~ \/____/



                                        HOBS is a sorting algorithm based on the
Bitmap sort, with HOBS the space problem
                                        of the Bitmap sort is solved and the
space overhead is around 140 KB. HOBS is spliting
                                        the space into multiple sub-spaces
called buckets.


*/

#include <stdio.h>
#include <time.h>
#include <map>
#include <vector>
#include <algorithm>
#include <memory.h>
#include <time.h>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

// 125 KB of data per bucket
// make this as a sparse vector
#define BUCKET_SIZE 125000
#define MAX 910114774
#define NUM_BUCKETS 224
#define NUM_THREADS 2
#define MB 1000000
#define MIN_PRINT_CTR 1200
#define VOIDER -1

pthread_t tid[NUM_THREADS];

typedef struct BUCKET {
  int bin_id;
  int bin_values[BUCKET_SIZE];
} BUCKET;

typedef struct thread_info {
  int id;
  int size;
  int *sorted_;
  BUCKET *bcks_raw_array;
} thread_info;

typedef struct ELEMENT {
  int index;
  int value;
  ELEMENT* next;
} ELEMENT;

double hobs_time, stl_sort_time;
int print_sort_flg = 0;
int bck_list[NUM_BUCKETS];

int** bck_refs;

inline int get_bck_id_pos(int bck_id) { return bck_id % NUM_BUCKETS; }

inline int get_list_bck_pos(int bck_id) {
  return (0x99A * bck_id) % NUM_BUCKETS;
}

void list_bck_ids() {
  for (int i = 0; i < NUM_BUCKETS; i++) {
    if (bck_list[i] != VOIDER)
      printf("%d ", bck_list[i]);
  }
  printf("\n");
}

void init_bck_list() {
  if (bck_list != NULL)
    memset(bck_list, VOIDER, sizeof(int) * NUM_BUCKETS);
}


map<int,int> bck_map_list;

// Read and write in the matrix holder
// this function should get the prefather
// system of the cpu
inline void __compute_density_mat_pos(vector<int> &__unsorted,
                                      int *density_matrix, int &i) {

  // this read get with a high cost
  // best case scenarious as read as part
  // of a cache line
  int val = __unsorted[i];
  // printf("val: %d  ", val);
  /* generate secret number between 1 and 10: */
  // int bck_id = val / BUCKET_SIZE;
  int bck_id = val / BUCKET_SIZE;
  // get the bucket id pos
  // int bck_id_pos = bck_id % NUM_BUCKETS;
  int pos = val % BUCKET_SIZE;
  int slot_id = get_bck_id_pos(bck_id);
  int pos_in_bck_holder = slot_id * BUCKET_SIZE + pos;
  // bck_list[pos_in_bck_list] = bck_id;

  //if (bck_list[pos_in_bck_list] == 0)
      //bck_list[pos_in_bck_list] = bck_id;
  //else if (bck_list[pos_in_bck_list] != bck_id)
    //printf("error");
  //density_matrix[pos_in_bck_list] = bck_id;
  std::map<int,int>::iterator f = bck_map_list.find(bck_id);

  if(f == bck_map_list.end())
  {
      //this is new
      bck_map_list.insert(std::make_pair<int, int>(bck_id, bck_id));
  }

  //if (bck_list[slot_id] == VOIDER)
    //bck_list[slot_id] = bck_id;
  //else if (bck_list[slot_id] != bck_id)
    //printf("error on slot id %d with bck id %d existing %d\n", slot_id, bck_id,
    //       bck_list[slot_id]);

  // printf("slot id %d  bck_id %d pos in density matrix %d val %d pos %d offset
  // "
  //     "%d\n",
  //     slot_id, bck_id, pos_in_bck_holder, val, pos, slot_id * BUCKET_SIZE);

  density_matrix[pos_in_bck_holder]++;
}

void _compose_sorted_array(int *density_matrix, int *sorted_) {
  int _ps = 0;
  // this is a tight loop build the final
  // sorted array
  int size_bck_holder = BUCKET_SIZE * NUM_BUCKETS;
  for (int i = 0; i < size_bck_holder; i++) {
    int dens_val = density_matrix[i];
    if (dens_val > 0) {
      // there have a value in this bin
      // need to compute the bucket and the final value
      for (int k = 0; k < dens_val; k++) {
        int bck_id = i / BUCKET_SIZE;
        int pos = i % BUCKET_SIZE;
        int value = bck_id * BUCKET_SIZE + pos;
        sorted_[_ps++] = value;
      }
    }
  }
}

void _compose_sorted_array_bcks(int *density_matrix, int *sorted_) {
  printf("Compose sorted array bck\n");
  int _ps = 0;
  for (int i = 0; i < NUM_BUCKETS; i++) {
    if (bck_list[i] != VOIDER) {
      // there is something in this bucket
      // get the slot id
      int slot_id = get_bck_id_pos(bck_list[i]);
      int bck_id = bck_list[slot_id];
      int offset = slot_id * BUCKET_SIZE;

      for (int k = 0; k < BUCKET_SIZE; k++) {
        if (density_matrix[offset + k] > 0) {

          for (int v = 0; v < density_matrix[offset + k]; v++) {
            int value = bck_id * BUCKET_SIZE + k;
            //printf("slot id %d bck id %d pos int density matrix %d pos %d "
            //       "value %d offset %d\n",
            //       slot_id, bck_id, offset + k, v, value, offset);

            sorted_[_ps++] = value;
          }
        }
      }
    }
  }
  printf("\n");
}

void generate_random_numbers(vector<int> &__unsorted_array, int num_elements) {
  for (int i = 0; i < num_elements; i++) {
    int num = rand() % MAX + 1;
    __unsorted_array.push_back(num);
  }
}

void __hobs_scan_(vector<int> &__unsorted_array, int *sorted_,
                  int *density_matrix, int num_elements) {

  clock_t begin, end;
  double time_spent;

  for (int i = 0; i < num_elements / 4; i++) {
    __compute_density_mat_pos(__unsorted_array, density_matrix, i);
  }

  begin = clock();
  // this will composed the final array
  //_compose_sorted_array(density_matrix, sorted_);
  _compose_sorted_array_bcks(density_matrix, sorted_);
  end = clock();
  hobs_time = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("composition took: %.3f s\n", hobs_time);
}

void __run_tests(int *sorted_, vector<int> &__unsorted_array,
                 int num_elements) {
  int is_sor_ok = true;
  // test 1
  for (int i = 1; i < num_elements; i++) {
    if (sorted_[i] < sorted_[i - 1]) {
      printf("Error, the sort order is invalid, invalid values %d and %d\n",
             sorted_[i], sorted_[i - 1]);
      break;
    }
  }
  // test 2 - test with the stl sorting array
  for (int i = 0; i < num_elements; i++) {
    if (sorted_[i] != __unsorted_array[i]) {
      printf("Invalid sorted array pos %d\n", i);
      is_sor_ok = false;
      break;
    }

    if (num_elements < MIN_PRINT_CTR) {
      printf("hobs: %d:\tstl: %d\n", sorted_[i], __unsorted_array[i]);
    }
  }
  printf("Hobs sorting test is: %d\n", is_sor_ok);
}

void hobs(int num_elements) {
  int bck_id = 0;
  srand(time(NULL));
  vector<int> __unsorted_array;
  clock_t begin, end;
  double time_spent;
  int sz_ds_mtx = sizeof(int) * BUCKET_SIZE * NUM_BUCKETS;
  double speedFactor = 1;
  init_bck_list();
  printf("size holder %d\n", sz_ds_mtx);

  int *sorted_ = (int *)malloc(sizeof(int) * num_elements);
  if (sorted_ == NULL) {
    printf("unable to allocate\n");
    return;
  }

  int *density_matrix = (int *)malloc(sz_ds_mtx);
  memset(density_matrix, 0, sz_ds_mtx);
  generate_random_numbers(__unsorted_array, num_elements);

  //__unsorted_array.push_back(1223);
  //__unsorted_array.push_back(12234234);
  //__unsorted_array.push_back(12234239);
  //__unsorted_array.push_back(1223423412);

  // get the initial start time
  // bellow is the main chunk of code that
  // does the processing and sorting
  begin = clock();
  __hobs_scan_(__unsorted_array, sorted_, density_matrix, num_elements);
  end = clock();
  hobs_time = (double)(end - begin) / CLOCKS_PER_SEC;

  printf("hobs took: %.3f s\n", hobs_time);
  printf("num of bcks %d\n", (int)bck_map_list.size() );
  printf("num of bcks: %d\n", (int)bck_map_list.size());

  //list_bck_ids();

  // compute the space taken by the high order bucket search  ( HOBS )
  double space_hobs = (NUM_BUCKETS * sizeof(BUCKET)) / 1024;
  printf("space required by hobs %.1f kb\n", space_hobs);

  begin = clock();
  sort(__unsorted_array.begin(), __unsorted_array.end());
  end = clock();
  stl_sort_time = (double)(end - begin) / CLOCKS_PER_SEC;

  if (hobs_time != 0 && stl_sort_time != 0)
    speedFactor = stl_sort_time / hobs_time;

  printf("data size %.1f MB\n", double(num_elements * sizeof(int) / MB));
  printf("stl sort took: %.3f s\n", stl_sort_time);
  printf("Sorted array lenght %d\n", num_elements);
  printf("Speed factor (hobs vs stl sort) %.2f x\n", speedFactor);

  // test the output
  __run_tests(sorted_, __unsorted_array, num_elements);

  free(sorted_);
  free(density_matrix);
}

int* field;
int len_field = 12000000;
void test_bitfield_mapper()
{
    field = (int*)malloc(sizeof(int) * len_field);
    if (field == NULL) {
        printf("unable to allocate\n");
        return;
    }

    for (int i = 0; i < len_field; i++) {
        field[i] = i;
    }

    clock_t begin, end;
    double time_spent;

    begin = clock();
    int bits_set = 0;
    for (int i = 0; i < len_field; i++) {
        int v = field[i];
        for (int j = 0; j < 32; j++) {
            int bits = v & (1 << j);
            if(bits) bits_set++;
        }
    }
    end = clock();

    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("check bit values time: %.3f s\n", time_spent);
    printf("num of bits set %d\n", bits_set);
}

inline ELEMENT* create_element(int value, int index){
    ELEMENT* element = (ELEMENT*) malloc (sizeof(ELEMENT));
    element->index = index;
    element->value = value;
    return element;
}

void __test_linked_list()
{
    int total_elements = 200000000;
    clock_t begin, end;
    double time_spent;
    begin = clock();
    ELEMENT* head = create_element(0,0);
    ELEMENT* prev = head;
    for(int i=1;i<total_elements;i++){
        ELEMENT* current = create_element(i,i);
        prev->next = current;
        prev = current;
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("element id %d\n", head->index);
    printf("time: %.3f s\n", time_spent);

    unsigned int tt = 0;
    begin = clock();
    ELEMENT* current = head;
    for(int i = 1;i < total_elements;i++){
        if(current == NULL) {
            printf("break here\n");
            break;
        }
        tt = current->index;
        current = current->next;
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time: %.3f s\n", time_spent);
    printf("index %d\n", tt);
}

/*
Starting point of the application
*/
int main(int argc, char** argv)
{

    int num_elements = 50;

    //test_bitfield_mapper();
    __test_linked_list();
    return 1;

    int flag;
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "n:")) != -1) {
        switch (c) {
        case 'n':
            num_elements = atoi(optarg);
            break;
        default:
            abort();
        }
    }

    printf("num of elements %d\n", num_elements);

    // high order bucket sort
    hobs(num_elements);

    return 0;
}
