#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "mapreduce.h"

typedef struct key_val{  // this is the structure for storing key-value pair
    char* key;
    char* val;  //SXY
} kv;

typedef struct key_vallist{  //SXY: this is the structure for storing key-valuelist for reducer
    char* key;
    char** val_list;
    int list_size;
} kvlist;

kv** intermediate;
int NUM_reducers;
sem_t *mutexes;  // lock list, every lock for every
int *partition_size; // SXY: this is used to store the patition size (double it when it is full)
int *curr_index;  // SXY: this is used to store the current index for every patition, pointing to the current avaliable index (or == size, meaning this partition is full)
// SXY: this is also used as the size of every partition after mapping! REALLY useful variable!!!
int *curr_getnext_index;
Partitioner local_partitioner;  //function pointer to use the partitioner passed into MR_Run()

typedef struct map_arg{
    int file_num; // number of files mapped to the mapper function
    char** file_list; // names of files mapped to the mapper function
    Mapper map;  // mapper function
} marg;

typedef struct reduce_arg{
    Reducer reduce;
    int partition_number;
} rarg;

// SXY: compare helper function for using qsort()
int compare (const void * a, const void * b)
{
    kv *A = (kv *)a;
    kv *B = (kv *)b;
    if(strcmp(A->key, B->key)<0) {
        return -1;
    }
    else if(strcmp(A->key, B->key)>0) {
        return 1;
    }
    else  //A->key == B->key
    {
        if(strcmp(A->val, B->val)<0)
            return -1;
        else
            return 1;
    }
}

char* get_next(char* key, int partition_number)
{
//    sem_wait(&mutexes[partition_number]);
    int p = partition_number;  //short for partition_number
//    sem_post(&mutexes[partition_number]);
    if(curr_getnext_index[p] >= curr_index[p] || strcmp(key,intermediate[p][curr_getnext_index[p]].key) != 0) {
        return NULL;
    }
    else {
        return intermediate[p][curr_getnext_index[p]++].val;
    }
}
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void MR_Emit(char *key, char *value) {
    //TODO need to free all the allocations!!
    int partition_index = (int)((*local_partitioner)(key, NUM_reducers));
    sem_wait(&mutexes[partition_index]); // SXY
    // SXY: implementation of new structure (Key_val pair rather than key_vallist)
    kv kv_instance; // create a kv instance
    kv_instance.key = malloc(sizeof(char) * (strlen(key) + 1));
    kv_instance.val = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(kv_instance.key, key);
    strcpy(kv_instance.val, value);
    if(partition_size[partition_index] == 0) {  // if this partition is empty
        partition_size[partition_index] = 2;   // initialize the size to 2
        free(intermediate[partition_index]); ///
        intermediate[partition_index] = malloc(sizeof(kv)*2);
        memcpy(&intermediate[partition_index][0], &kv_instance, sizeof(kv));
        curr_index[partition_index]++;
    }
    else if(curr_index[partition_index] == partition_size[partition_index]) {  // if this partition is full
        // reconstruct partition (realloc() function doesn't work!)
        kv* temp = malloc(sizeof(kv) *2*partition_size[partition_index]);
        for (int i = 0; i < partition_size[partition_index]; ++i) {
            memcpy(&temp[i], &intermediate[partition_index][i], sizeof(kv));
        }

        free(intermediate[partition_index]);
        intermediate[partition_index] = temp;
        partition_size[partition_index] = 2*partition_size[partition_index];
        memcpy(&intermediate[partition_index][curr_index[partition_index]], &kv_instance, sizeof(kv));
        curr_index[partition_index]++;
    }
    else {  // not full and not empty, directly add to the partition
        memcpy(&intermediate[partition_index][curr_index[partition_index]], &kv_instance, sizeof(kv));
        curr_index[partition_index]++;
    }
    sem_post(&mutexes[partition_index]); // SXY
}

void* Map_wrapper(void* arg) {
    for (int i = 0; i < ((marg*)arg)->file_num; ++i) {
        (((marg*)arg)->map)(((marg*)arg)->file_list[i]);
    }
    pthread_exit(NULL);
}

// need to get all the keys in the specific partition
void* Reduce_wrapper(void* arg) {
    while(curr_getnext_index[((rarg*)arg)->partition_number]<curr_index[((rarg*)arg)->partition_number]) {
        (((rarg*)arg)->reduce)(intermediate[((rarg*)arg)->partition_number][curr_getnext_index[((rarg*)arg)->partition_number]].key, get_next, ((rarg*)arg)->partition_number);
    }
    pthread_exit(NULL);
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition) {
    if(partition != NULL) {
        local_partitioner = partition;
    }
    else {
        local_partitioner = MR_DefaultHashPartition;
    }

    pthread_t map_threads[num_mappers];
    pthread_t reduce_threads[num_reducers];
    curr_getnext_index = malloc(sizeof(int)*num_reducers);
    NUM_reducers = num_reducers;

    partition_size = malloc(sizeof(int)*num_reducers);  // SXY: this is used to store the patition size (double it when it is full)
    curr_index = malloc(sizeof(int)*num_reducers);  // SXY: this is used to store the current index for every patition

    // initialize intermediate
    intermediate = (kv**)malloc(sizeof(kv*)*num_reducers);
    // initialize partition-related integers:
    for (int i = 0; i < num_reducers; i++) {
        partition_size[i] = 0;
        curr_index[i] = 0;
        curr_getnext_index[i] = 0;
        intermediate[i] = malloc(sizeof(kv)); ///
    }

    // initialize mutexes
    mutexes = malloc(sizeof(sem_t) * num_reducers);
    for (int i = 0; i < num_reducers; i++) {
        sem_init(&mutexes[i], 0, 1);
    }

    // initialize and alloc memory for arguments list for Map_wrapper
    char** file_lists[num_mappers];
    int file_nums[num_mappers];

    for (int i = 0; i < num_mappers; ++i) {
        file_nums[i] = 0;
        file_lists[i] = malloc(sizeof(char**));
        //*file_lists[i] = malloc(sizeof(char*));
    }
    for (int i = 1; i < argc; ++i) {  // get the file_lists
        int index = (i - 1) % num_mappers;
        file_nums[index] += 1;

        char** temp = malloc(sizeof(char*) * file_nums[index]);
        for (int i = 0; i < file_nums[index]-1; ++i) {
            temp[i] = file_lists[index][i];
        }
        free(file_lists[index]);
        file_lists[index] = temp;
        file_lists[index][file_nums[index]-1] = argv[i];
    }
    marg* marg_list[num_mappers]; // map_wrapper argument list
    for(int i = 0; i < num_mappers; i++) {
        // construct arguments for Map_wrapper
        marg_list[i] = malloc(sizeof(marg));
        marg_list[i]-> file_num = file_nums[i];
        marg_list[i]-> file_list = file_lists[i];
        marg_list[i]-> map = map;
        // run threads
        if (pthread_create(&map_threads[i], NULL, Map_wrapper, (void*)marg_list[i]) != 0) {
            printf("thread creation failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < num_mappers; i++) {
        pthread_join(map_threads[i], NULL);
    }
    // free memory of arguments for Map_wrapper
    for(int  i = 0; i < num_mappers; i++){
        free(file_lists[i]);
        free(marg_list[i]);
    }

    //SXY: sort with qsort()
    for(int i = 0; i < num_reducers; i ++){
        qsort(intermediate[i], curr_index[i], sizeof(kv),compare);
    }

    rarg* rarg_list[num_reducers];
    for (int i = 0; i < num_reducers; i++) {
        rarg_list[i] = malloc(sizeof(rarg));
        rarg_list[i]->reduce = reduce;
        rarg_list[i]->partition_number = i;
        if (pthread_create(&reduce_threads[i], NULL, Reduce_wrapper, (void*)rarg_list[i]) != 0) {
            printf("thread creation failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reduce_threads[i], NULL);
    }

    for (int i = 0; i < num_reducers; i++) {
        free(rarg_list[i]);
    }

    for (int i = 0; i < num_reducers; i++) {
        sem_destroy(&mutexes[i]);
    }

    for (int i = 0; i < num_reducers; ++i) {
        for (int j = 0; j < curr_index[i]; ++j) {
            free(intermediate[i][j].key);///
            free(intermediate[i][j].val);///
        }
        free(intermediate[i]);///
    }
    
    free(mutexes);
    free(curr_index);///
    free(curr_getnext_index);///
    free(partition_size);///
    free(intermediate);///
}