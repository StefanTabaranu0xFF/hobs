

/*
          _____                   _______                   _____                    _____
         /\    \                 /::\    \                 /\    \                  /\    \
        /::\____\               /::::\    \               /::\    \                /::\    \
       /:::/    /              /::::::\    \             /::::\    \              /::::\    \
      /:::/    /              /::::::::\    \           /::::::\    \            /::::::\    \
     /:::/    /              /:::/~~\:::\    \         /:::/\:::\    \          /:::/\:::\    \
    /:::/____/              /:::/    \:::\    \       /:::/__\:::\    \        /:::/__\:::\    \
   /::::\    \             /:::/    / \:::\    \     /::::\   \:::\    \       \:::\   \:::\    \
  /::::::\    \   _____   /:::/____/   \:::\____\   /::::::\   \:::\    \    ___\:::\   \:::\    \
 /:::/\:::\    \ /\    \ |:::|    |     |:::|    | /:::/\:::\   \:::\ ___\  /\   \:::\   \:::\    \
/:::/  \:::\    /::\____\|:::|____|     |:::|    |/:::/__\:::\   \:::|    |/::\   \:::\   \:::\____\
\::/    \:::\  /:::/    / \:::\    \   /:::/    / \:::\   \:::\  /:::|____|\:::\   \:::\   \::/    /
 \/____/ \:::\/:::/    /   \:::\    \ /:::/    /   \:::\   \:::\/:::/    /  \:::\   \:::\   \/____/
          \::::::/    /     \:::\    /:::/    /     \:::\   \::::::/    /    \:::\   \:::\    \
           \::::/    /       \:::\__/:::/    /       \:::\   \::::/    /      \:::\   \:::\____\
           /:::/    /         \::::::::/    /         \:::\  /:::/    /        \:::\  /:::/    /
          /:::/    /           \::::::/    /           \:::\/:::/    /          \:::\/:::/    /
         /:::/    /             \::::/    /             \::::::/    /            \::::::/    /
        /:::/    /               \::/____/               \::::/    /              \::::/    /
        \::/    /                 ~~                      \::/____/                \::/    /
         \/____/                                           ~~                       \/____/

HOBS - IS A SORTING ALGORITHM

*/




#include <time.h>
#include <map>
#include <vector>
#include <algorithm>

#define BUCKET_SIZE 9
#define MAX 122234
#define NUM_BUCKETS 5000

using namespace std;

typedef struct BUCKET
{
	int bin_id;
	int bin_values[BUCKET_SIZE];
}BUCKET;

inline int get_bck_id_pos(int bck_id)
{
	return bck_id % NUM_BUCKETS;
}

inline void __bucket_aligment(vector<int>& __unsorted_array,
	BUCKET* bcks_raw_array,
	int i)
{
	/* generate secret number between 1 and 10: */
	int bck_id = __unsorted_array[i] / BUCKET_SIZE;

	//get the bucket id pos

	int bck_id_pos = get_bck_id_pos(bck_id);
	int pos = __unsorted_array[i] % BUCKET_SIZE;

	if (bcks_raw_array[bck_id].bin_id != -1 &&
		bcks_raw_array[bck_id].bin_id != bck_id) {
		printf("Invalid bck id %d\n", bck_id);
		return;
	}

	bcks_raw_array[bck_id].bin_id = bck_id;
	bcks_raw_array[bck_id].bin_values[pos]++;
}

void hobs(int num_elements)
{
	int bck_id = 0;
	srand(time(NULL));
	map<int, BUCKET> buckets;
	vector<int> __unsorted_array;
	printf("num of elements in the array: %d\n", num_elements);

	//alloc space for buckets
	BUCKET* bcks_raw_array = (BUCKET*)malloc(sizeof(BUCKET) * NUM_BUCKETS);
	memset(bcks_raw_array, 0, sizeof(BUCKET) * NUM_BUCKETS);

	for (int i = 0; i < NUM_BUCKETS; i++)
		bcks_raw_array[i].bin_id = -1;

	//the main loop where the
	//the elemets are taken and placed
	//in a buckets
	int* raw_unsorted_ptr = (int*)malloc(sizeof(int) * num_elements);

	printf("generate random numbers\n");

	for (int i = 0; i < num_elements; i++) {
		int num = rand() % MAX + 1;
		__unsorted_array.push_back(num);
	}

	printf("number generation is done\n");

	unsigned long start = 12;
	int workload = 0;


	for (int i = 0; i < __unsorted_array.size(); i++)
	{
		/* generate secret number between 1 and 10: */
		bck_id = __unsorted_array[i] / BUCKET_SIZE;

		//get the bucket id pos

		int bck_id_pos = get_bck_id_pos(bck_id);
		int pos = __unsorted_array[i] % BUCKET_SIZE;

		if (bcks_raw_array[bck_id].bin_id != -1 &&
			bcks_raw_array[bck_id].bin_id != bck_id) {
			printf("Invalid bck id %d\n", bck_id);
			return;
		}

		bcks_raw_array[bck_id].bin_id = bck_id;
		bcks_raw_array[bck_id].bin_values[pos]++;
	}
	printf("num of buckets\t%d\n", buckets.size());


	vector<int> __sorted_array;

	int prev_bck_id = 0;
	int prev_currentValue = 0;
	int _currentValue = 0;

	int* sorted_ = (int*)malloc(sizeof(int) * num_elements);
	int _ps = 0;

	if (sorted_ == NULL) {
		printf("unable to allocate\n");
		return;
	}

	//this is a tight loop build the final
	//sorted array
	for (int i = 0; i < NUM_BUCKETS; i++) {
		if (bcks_raw_array[i].bin_id != -1) {
			for (int j = 0; j < BUCKET_SIZE; j++) {
				if (bcks_raw_array[i].bin_values[j] > 0) {
					for (int k = 0; k < bcks_raw_array[i].bin_values[j]; k++) {
						int finalValue = bcks_raw_array[i].bin_id * BUCKET_SIZE + j;
						sorted_[_ps++] = finalValue;
					}
				}
			}
		}
	}
	unsigned long end = 456;
	printf("hobs took: %u ms\n", end - start);


	//compute the space taken by the high order bucket search  ( HOBS )
	double space_hobs = (NUM_BUCKETS * sizeof(BUCKET)) / 1024;

	printf("space required by hobs %.1f kb\n", space_hobs);

	start = 123;
	sort(__unsorted_array.begin(), __unsorted_array.end());
	end = 345;

	printf("data size %.1f MB\n", double(num_elements * sizeof(int) / 1000000));
	printf("stl sort took: %u ms\n", end - start);
	printf("Sorted array lenght %d\n", num_elements);


	//test 1
	for (int i = 1; i < num_elements; i++) {
		if (sorted_[i] < sorted_[i - 1]) {
			printf("Error, the sort order is invalid, invalid values %d and %d\n",
				sorted_[i], sorted_[i-1]);
			break;
		}
		//printf("%d ", sorted_[i]);
	}

	//test 2 - test with the stl sorting array
	for (int i = 0; i < num_elements; i++) {
		if (sorted_[i] != __unsorted_array[i]) {
			printf("Invalid sorted array pos %d\n", i);
			break;
		}

		//printf("%d, %d\n", sorted_[i], __unsorted_array[i]);
	}

	printf("\n");
	free(sorted_);
	free(bcks_raw_array);
	printf("-----------------------------------------------\n");
}


/*
Starting point of the application
*/
int main()
{
	//high bucket sort
	hobs(80000000);

    return 0;
}
