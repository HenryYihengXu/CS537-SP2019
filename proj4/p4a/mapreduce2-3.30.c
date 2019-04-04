#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "mapreduce.h"

typedef struct key_val{  // this is the structure for storing key-value pair
    char* key;
    int size;
    char** val_list;
    int current_val_index; // XYH
} kv;

kv*** intermediate;
//int NUM_mappers;
int NUM_reducers;
int *key_num;
int *current_key_index; // XYH
sem_t *mutexes;  // lock list, every lock for every

//TODO need to be implemented
typedef struct map_arg{
    int file_num; // number of files mapped to the mapper function
    char** file_list; // names of files mapped to the mapper function
    Mapper map;  // mapper function
} marg;


//TODO need to be implemented
typedef struct reduce_arg{
    //   Getter get_next;
    Reducer reduce;
    int partition_number;
} rarg;


char* get_next(char* key, int partition_number) //declarition for testing, TODO need to be implemented in the reduce part
{
    sem_wait(&mutexes[partition_number]);
    int curr = intermediate[partition_number][current_key_index[partition_number]]->current_val_index;
    if (curr >= intermediate[partition_number][current_key_index[partition_number]]->size) {
        current_key_index[partition_number]++;
        sem_post(&mutexes[partition_number]);
        return NULL;
    } else {
        intermediate[partition_number][current_key_index[partition_number]]->current_val_index++;
        sem_post(&mutexes[partition_number]);
        return intermediate[partition_number][current_key_index[partition_number]]->val_list[curr];
    }
    /*sem_wait(&mutexes[partition_number]);
    for (int i = 0; i < key_num[partition_number]; i++) {
        if (strcmp(intermediate[partition_number][i]->key, key) < 0) {
            continue;
        } else if (strcmp(intermediate[partition_number][i]->key, key) == 0) {
            kv* kv_instance = intermediate[partition_number][i];
            int curr = kv_instance->current_val_index;
            if (curr >= kv_instance->size) {
                sem_post(&mutexes[partition_number]);
                return NULL;
            } else {
                kv_instance->current_val_index++;
                sem_post(&mutexes[partition_number]);
                printf("%d\n", curr);
                printf("%s\n", kv_instance->val_list[curr]);
                return kv_instance->val_list[curr];
            }
        } else {
            printf("Error: the key \"%s\" does not exist, something wrong in Emit or sort.\n", key);
            sem_post(&mutexes[partition_number]);
            return NULL;
        }
    }
    printf("Error: the key \"%s\" does not exist, something wrong in Emit or sort.\n", key);
    sem_post(&mutexes[partition_number]);
    return NULL;*/
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
    int partition_index = (int)MR_DefaultHashPartition(key, NUM_reducers);
    sem_wait(&mutexes[partition_index]); // SXY
    if (key_num[partition_index] == 0) { // if nothing in this partition, then initialize it
        intermediate[partition_index] = (kv**)malloc(sizeof(kv*)); // malloc a kv array with size 1 for this partition
        key_num[partition_index]++; // now the number of key is 1
        kv *kv_instance = malloc(sizeof(kv)); // create a kv instance

        // set the key of this kv instance
        kv_instance->key = malloc(sizeof(char) * (strlen(key) + 1));
        strcpy(kv_instance->key, key);

        kv_instance->size = 1; // now this kv instance has 1 value in its value list
        kv_instance->val_list = (char**)malloc(sizeof(char*)); //malloc the value list with size 1
        kv_instance->current_val_index = 0; // XYH

        // set the first value
        kv_instance->val_list[0] = malloc(sizeof(char) * (strlen(value) + 1));
        strcpy(kv_instance->val_list[0], value);

        intermediate[partition_index][0] = kv_instance; // add this kv instance to this partition
    } else { // already has kv in the partition
        // check if the key is already in the partition
        for (int i = 0; i < key_num[partition_index]; ++i) {
            kv *kv_instance = intermediate[partition_index][i];
            if (strcmp(kv_instance->key, key) == 0) { // if the key is already in
                kv_instance->size++; // now this kv has one more value in its value list
                // realloc kv_instance->val_list (realloc() function doesn't work!)
                char** temp = malloc(sizeof(char*) * kv_instance->size);
                for (int i = 0; i < kv_instance->size - 1; ++i) {
                    temp[i] = kv_instance->val_list[i];
                }
                free(kv_instance->val_list);
                kv_instance->val_list = temp;

                /*if(realloc(kv_instance->val_list, sizeof(char*) * kv_instance->size) == NULL) // realloc value list to append the new value.
                {
                    printf("realloc failed\n");
                    exit(1);
                }*/
                // append value
                kv_instance->val_list[kv_instance->size - 1] = malloc(sizeof(char) * (strlen(value) + 1));
                strcpy(kv_instance->val_list[kv_instance->size - 1], value);
                sem_post(&mutexes[partition_index]);
                return;
            }
        }
        // if the key is not in the partition:
        key_num[partition_index]++; // now the partition has one more key
        // realloc partition (realloc() function doesn't work!)
        kv** temp = malloc(sizeof(kv*) * key_num[partition_index]);
        for (int i = 0; i < key_num[partition_index] - 1; ++i) {
            temp[i] = intermediate[partition_index][i];
        }
        free(intermediate[partition_index]);
        intermediate[partition_index] = temp;

        /*if(realloc(intermediate[partition_index], sizeof(kv*) * key_num[partition_index] + 1) == NULL) // realloc kv array to append the new kv
        {
            printf("realloc failed\n");
            exit(1);
        }*/
        kv *kv_instance = malloc(sizeof(kv)); // create a new kv

        // set key of this kv
        kv_instance->key = malloc(sizeof(char) * (strlen(key) + 1));
        kv_instance->current_val_index = 0;
        strcpy(kv_instance->key, key);

        kv_instance->size = 1; // it has 1 value
        kv_instance->val_list = (char**)malloc(sizeof(char*)); // alloc value list

        // set the first value
        kv_instance->val_list[0] = malloc(sizeof(char) * (strlen(value) + 1));
        strcpy(kv_instance->val_list[0], value);

        intermediate[partition_index][key_num[partition_index] - 1] = kv_instance; // append this new kv
    }

    sem_post(&mutexes[partition_index]); // SXY
}

void* Map_wrapper(void* arg) {
    for (int i = 0; i < ((marg*)arg)->file_num; ++i) {
        printf("this is before loop %d in mapper\n", i);
        (((marg*)arg)->map)(((marg*)arg)->file_list[i]);
        printf("this is after loop %d in mapper\n", i);
    }
    pthread_exit(NULL);
}

// need to get all the keys in the specific partition
void* Reduce_wrapper(void* arg) {
    printf("partition_number = %d\n", key_num[((rarg*)arg)->partition_number]);
    for (int i = 0; i < key_num[((rarg*)arg)->partition_number]; ++i) {
        printf("this is before loop %d in reducer\n", i);
        (((rarg*)arg)->reduce)(intermediate[((rarg*)arg)->partition_number][i]->key, get_next, ((rarg*)arg)->partition_number);
        printf("this is after loop %d in reducer\n", i);
    }
    pthread_exit(NULL);
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition) {
    printf("num_mapper: %d\n", num_mappers);
    printf("num_reducer: %d\n", num_reducers);
    pthread_t map_threads[num_mappers];
    pthread_t reduce_threads[num_reducers];
    key_num = malloc(sizeof(int)*num_reducers);
    current_key_index = malloc(sizeof(int)*num_reducers); // XYH
    NUM_reducers = num_reducers;

    // initialize mutexes
    mutexes = malloc(sizeof(sem_t) * num_reducers);
    for (int i = 0; i < num_reducers; i++) {
        sem_init(&mutexes[i], 0, 1);
    }

    // initialize intermediate
    intermediate = (kv***)malloc(sizeof(kv**)*num_reducers);

    for(int i = 0; i < num_reducers; i++)
    {
        key_num[i] = 0;
        current_key_index[i] = 0; // XYH
    }

// initialize and alloc memory for arguments list for Map_wrapper
    char** file_lists[num_mappers];
    int file_nums[num_mappers];

    for (int i = 0; i < num_mappers; ++i) {
        file_nums[i] = 0;
        file_lists[i] = malloc(sizeof(char**));
        *file_lists[i] = malloc(sizeof(char*));
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
/*
        if( realloc(file_lists[index], sizeof(char*) * file_nums[index]) == NULL)
        {
            printf("realloc failed\n");
            exit(1);
        }
*/
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
    }

    //sort intermediate files
    for(int i = 0; i < num_reducers; i++) {
        // sort every partition on key in ascending order
        for (int j = 0; j < key_num[i]; j++) {
            char* min = intermediate[i][j]->key;
            int minindex = j;
            for (int k = j; k < key_num[i]; k++) {
                if(strcmp(intermediate[i][k]->key, min) < 0){
                    min = intermediate[i][k]->key;
                    minindex = k;
                }
            }
            kv* temp;
            temp = intermediate[i][j];
            intermediate[i][j] = intermediate[i][minindex];
            intermediate[i][minindex] = temp;
        }
        //something wrong in this sort (Bubble sort)
        /*for (int j = 0; j < key_num[i]; j++) {
            for (int k = j-1; k > -1; k--) {
                if(strcmp(intermediate[i][j]->key, intermediate[i][k]->key) < 0){
                    kv* temp;
                    temp = intermediate[i][j];
                    intermediate[i][j] = intermediate[i][k];
                    intermediate[i][k] = temp;
                } else {
                    break;
                }
            }
        }*/
        // sort every key's value list
        for (int j = 0 ; j < key_num[i]; j++) {
            for (int k = 1; k < intermediate[i][j]->size; k++) {
                for (int k2 = k-1; k2 > -1; k2--) {
                    if(strcmp(intermediate[i][j]->val_list[k], intermediate[i][j]->val_list[k2]) < 0){
                        char* temp;//malloc? free?
                        temp = intermediate[i][j]->val_list[k];
                        intermediate[i][j]->val_list[k] = intermediate[i][j]->val_list[k2];
                        intermediate[i][j]->val_list[k2] = temp;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    //printf("this is AFTER sort!!!!!\n");
// test:
    /*printf("this is the test of intermediate files after mapping\n");
    for (int i = 0; i < num_reducers; i ++) {
        for(int k = 0; k < key_num[i]; k ++) {
            printf("key is: %s\n", intermediate[i][k]->key);
            for (int p = 0; p < intermediate[i][k]->size; p++) {
                printf("value[%d] = %s\n", p, intermediate[i][k]->val_list[p]);
            }
            printf("\n");
        }
    }*/
//

    // TODO get key_lists[partition_number]
    printf("this is before initialize rarg_list\n");
    rarg* rarg_list[num_reducers];
    for (int i = 0; i < num_reducers; i++) {
        rarg_list[i] = malloc(sizeof(rarg));
//      rarg_list[i]->get_next = get_next;
        rarg_list[i]->reduce = reduce;
        rarg_list[i]->partition_number = i;
        printf("this is before pthread_create\n");
        if (pthread_create(&reduce_threads[i], NULL, Reduce_wrapper, (void*)rarg_list[i]) != 0) {
            printf("thread creation failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < num_reducers; i++) {
        pthread_join(reduce_threads[i], NULL);
    }
    printf("this is after pthread_join\n");

    for (int i = 0; i < num_reducers; i++) {
        free(rarg_list[i]);
    }

    free(key_num);

    for (int i = 0; i < num_reducers; i++) {
        sem_destroy(&mutexes[i]);
    }
    free(mutexes);
}


intermediate[i] = malloc(sizeof(kv));
kv kv_instance;
kv_instance.key =
memcpy()