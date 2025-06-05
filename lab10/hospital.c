#define _GNU_SOURCE
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define PATIENTS_PER_VISIT 3
#define DRUGS_CAPACITY 6
#define DRUGS_REPLENISH 3
#define PATIENT_SLEEP_TIME 15


pthread_cond_t info_change = PTHREAD_COND_INITIALIZER;
pthread_mutex_t doctor_info_lock = PTHREAD_MUTEX_INITIALIZER;

int queue_full = 0;
int flag_ready_to_deliver = 0;
int patients_waiting = 0;
int patients_waiting_ids[PATIENTS_PER_VISIT];
int drugs = 0;

int total_patients_served = 0;

void* patient(int* patient_id){
    
    int arrival_time = rand() % 4 + 2;

    printf("%d Pacjent %d: ide do szpitala, bede za %d s\n", time(NULL), *patient_id, arrival_time);
    sleep(arrival_time);

    while(1){
        // check if queue is full
        pthread_mutex_lock(&doctor_info_lock);
        if(patients_waiting == PATIENTS_PER_VISIT){
            printf("%d Pacjent %d: Za duzo pacjentow, wracam pozniej za %d s\n", time(NULL), *patient_id, PATIENT_SLEEP_TIME);
            pthread_mutex_unlock(&doctor_info_lock);
            sleep(PATIENT_SLEEP_TIME);
        } else {
            // join queue
            patients_waiting_ids[patients_waiting++] = *patient_id;
            printf("%d Pacjent %d: Na lekarza czeka %d pacjentow \n", time(NULL), *patient_id, patients_waiting);
            break;
        }
    }

    // here we still have locked mutex

    if(patients_waiting == PATIENTS_PER_VISIT){
        printf("%d Pacjent %d: Budze lekarza\n", time(NULL), *patient_id);
        // inform the doctor that there is required number of patients waiting
        pthread_cond_broadcast(&info_change);
    }

    // still locked mutex

    // wait until all current patients were served
    while(patients_waiting != 0){
        pthread_cond_wait(&info_change, &doctor_info_lock);
    }

    pthread_mutex_unlock(&doctor_info_lock);


    printf("%d Pacjent %d: Koncze wizyte\n", time(NULL), *patient_id);
    return NULL;
}

void* pharmacist(int* pharmacist_id){
    int arrival_time = rand() % 11 + 5;
    printf("%d Farmaceuta %d: ide do szpitala, bede za %d s\n", time(NULL), *pharmacist_id, arrival_time);
    sleep(arrival_time);

    pthread_mutex_lock(&doctor_info_lock);
    if(drugs > DRUGS_CAPACITY - PATIENTS_PER_VISIT){
        printf("%d Farmaceuta %d: Czekam na oproznienie apteczki\n", time(NULL), *pharmacist_id);
    }
    // wait for when there's not enough drugs and no other pharmacist is ready
    while(drugs < PATIENTS_PER_VISIT && !flag_ready_to_deliver){
        pthread_cond_wait(&info_change, &doctor_info_lock);
    }

    printf("%d Farmaceuta %d: Budze lekarza\n", time(NULL), *pharmacist_id);

    // doctor will replenish drugs
    flag_ready_to_deliver = 1;
    pthread_mutex_unlock(&doctor_info_lock);
    pthread_cond_broadcast(&info_change);

    // not synchronised with doctor accepting them
    printf("%d Farmaceuta %d: Dostarczam leki\n", time(NULL), *pharmacist_id);

    printf("%d Farmaceuta %d: Koncze prace\n", time(NULL), *pharmacist_id);

    return NULL;
}

void* doctor(int* total_patients){
    pthread_mutex_lock(&doctor_info_lock);

    // doctor work as long as there are not served patients
    while(total_patients_served < *total_patients){
        while(!(patients_waiting == 3 && drugs >= PATIENTS_PER_VISIT) && !flag_ready_to_deliver){
            pthread_cond_wait(&info_change, &doctor_info_lock);
        }
        printf("%d Lekarz: Budze sie\n", time(NULL));
        if(flag_ready_to_deliver){
            // Accepting drug delivery
            sleep(rand() % 3 + 1);
            printf("%d Lekarz: przyjmuje dostawe lekow\n", time(NULL));
            
            drugs += DRUGS_REPLENISH;
            flag_ready_to_deliver = 0;
        } else {
            // Serving patients
            sleep(rand() % 3 + 2);
            printf("%d Lekarz: konsultuje pacjentow", time(NULL));

            for(int i = 0; i<patients_waiting; i++){
                printf(" %d", patients_waiting_ids[i]);
                patients_waiting_ids[i] = -1;
            }
            printf("\n");
            patients_waiting = 0;
            total_patients_served += PATIENTS_PER_VISIT;
        }

        pthread_cond_broadcast(&info_change);
        printf("%d Lekarz: Ide spac\n", time(NULL));
    }

    return NULL;
}


int main(int argc, char* argv[]){
    if(argc != 3){
        printf("Give exactly 2 arguments!\n");
        return 1;
    }
    int patients = atoi(argv[1]);
    int pharmacists = atoi(argv[2]);

    if(patients <= 0 || pharmacists <= 0){
        printf("Invalid argument!\n");
        return 1;
    }

    if(patients % PATIENTS_PER_VISIT != 0){
        printf("Number of patients must be divisible by %d\n!", PATIENTS_PER_VISIT);
        return 1;
    }

    // Assuming every pharmacist delivers PATIENTS_PER_VISIT drugs
    if(patients > pharmacists*DRUGS_CAPACITY){
        printf("There isn't enough drugs for everyone!\n");
        return 1;
    }

    // initialize patients ids array to easier detect errors (ids and >=0)
    for(int i=0; i<PATIENTS_PER_VISIT; patients_waiting_ids[i++]=-1);
    

    // create doctor's thread
    pthread_t doctor_thread;
    pthread_create(&doctor_thread, NULL, doctor, &patients);

    // create patients thread
    pthread_t *patient_threads = malloc(sizeof(pthread_t) * patients);
    int *patient_ids = malloc(sizeof(int) * patients);

    for(int i=0; i<patients; i++){
        patient_ids[i] = i;
        pthread_create(&patient_threads[i], NULL, patient, &patient_ids[i]);
    }

    // create pharmacists threads
    pthread_t *pharmacists_threads = malloc(sizeof(pthread_t) * pharmacists);
    int *pharmacist_ids = malloc(sizeof(int) * pharmacists);

    for(int i=0; i<pharmacists; i++){
        pharmacist_ids[i] = i;
        pthread_create(&pharmacists_threads[i], NULL, pharmacist, &pharmacist_ids[i]);
    }


    pthread_join(doctor_thread, NULL);
    return 0;
}