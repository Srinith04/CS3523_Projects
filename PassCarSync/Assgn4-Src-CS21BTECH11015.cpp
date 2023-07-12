/*------------------------------------------------------------------------------*/
/* Libraries */
/*------------------------------------------------------------------------------*/

#include <iostream>

#include <sys/time.h>

#include <atomic>

#include <cstdlib>

#include <fstream>

#include <chrono>

#include <ctime>

#include <unistd.h>

#include <random>

#include <vector>

#include <algorithm>

#include <queue>

#include <pthread.h>

#include <semaphore.h>

using namespace std;

/*------------------------------------------------------------------------------*/
/* Globally defining the variables */
/*------------------------------------------------------------------------------*/

// auto started = chrono::high_resolution_clock::now();

int P, C, lam1, lam2, k;

default_random_engine generator;

sem_t *semavaicars;

sem_t *semavaipass;

queue<int> avaicars;

queue<int> avaipass;

mutex carqueuemutex;

mutex passqueuemutex;

sem_t **personalmutex;

int exit_VAR = 0;

mutex forout;

/*------------------------------------------------------------------------------*/
/* Struct containing things related to thread_id */
/*------------------------------------------------------------------------------*/

struct thread_id
{
    int id; /*Thread id*/

    vector<struct timeval> *times; /* Vector to store the TIME INSTANCES */

    vector<pair<int, int> > *pairs; /* Vector to store the pairs of car and passengers / viceversa*/

    long double *ele; /* To store total ride time of a car*/
};

typedef struct thread_id *Thread_id;

/*------------------------------------------------------------------------------*/
/* Function which is run on each on each passsenger thread */
/*------------------------------------------------------------------------------*/

void *Passenger(void *arg)
{
    exponential_distribution<double> distribution1((1 / (double)lam1)); /*Exponential distribution for sleep pairs*/

    Thread_id data = (Thread_id)arg;

    int id = data->id;

    vector<struct timeval> *times = data->times;

    vector<pair<int, int> > *pairs = data->pairs;

    int temp_car;

    struct timeval timenow;

    gettimeofday(&timenow, NULL);

    times->at(0) = timenow; //------------enter time

    for (int i = 0; i < k; i++)
    {
        usleep((int)(distribution1(generator) * 1000)); // Wander time

        gettimeofday(&timenow, NULL);

        times->at(3 * i + 1) = timenow; //------------request time

        passqueuemutex.lock();

        sem_post(semavaipass);

        avaipass.push(id); // Adding into the available passengers queue

        passqueuemutex.unlock();

        sem_wait(semavaicars); // Waiting for cars

        gettimeofday(&timenow, NULL);

        times->at(3 * i + 2) = timenow; //------------accept time

        carqueuemutex.lock();

        if (!avaicars.empty())
        {
            temp_car = avaicars.front();

            avaicars.pop(); // Poping the top car

            pairs->at(i) = make_pair(id, temp_car); //------------ pairs

            sem_wait(personalmutex[temp_car]); // Waiting untill the car completes its ride

            carqueuemutex.unlock();

            gettimeofday(&timenow, NULL);

            times->at(3 * i + 3) = timenow; //------------end time

            sem_post(personalmutex[temp_car]);
        }
        else
        {
            carqueuemutex.unlock();
        }
    }

    gettimeofday(&timenow, NULL);

    times->at(3 * k + 1) = timenow; //------------exit time

    pthread_exit(0);
}

/*------------------------------------------------------------------------------*/
/* Function which is run on each on each car thread */
/*------------------------------------------------------------------------------*/

void *Car(void *arg)
{
    exponential_distribution<double> distribution2((1 / (double)lam2)); /*Exponential distribution for sleep pairs*/

    Thread_id data = (Thread_id)arg;

    int id = data->id;

    vector<struct timeval> *times = data->times;

    vector<pair<int, int> > *pairs = data->pairs;

    long double *ele = data->ele;

    int temp_pass;

    struct tm *timeinfo;

    struct timeval timenow1;

    struct timeval timenow2;

    time_t now;

    long double time_taken = 0;

    do
    {
        carqueuemutex.lock();

        sem_post(semavaicars);

        avaicars.push(id); // Adding into the available cars queue

        carqueuemutex.unlock();

        sem_wait(semavaipass); // Waiting for passengers

        passqueuemutex.lock();

        if (!avaipass.empty())
        {
            sem_wait(personalmutex[id]); // lOCKING THE PERSONAL MUTEX

            temp_pass = avaipass.front();

            avaipass.pop(); // Poping the top passenger

            pairs->push_back(make_pair(id, temp_pass));

            passqueuemutex.unlock();

            gettimeofday(&timenow1, NULL);

            usleep((int)(distribution2(generator) * 1000)); // SIMULATING RIDE TIME

            gettimeofday(&timenow2, NULL);

            sem_post(personalmutex[id]);

            time_taken = time_taken + (timenow2.tv_sec - timenow1.tv_sec) * 1000000L + timenow2.tv_usec - timenow1.tv_usec;

            ele[0] = time_taken; // ADD TIME TAKEN BY CAR EACH TIME
        }
        else
        {
            passqueuemutex.unlock();
        }

        if (exit_VAR == -1)
        {
            pthread_exit(0);
        }

    } while (true);

    pthread_exit(0);
}

int main()
{
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count()); /*Seeding the generator*/

    ifstream pointer("inp.txt"); /* Input file*/

    pointer >> P >> C >> lam1 >> lam2 >> k;

    pointer.close();

    semavaicars = sem_open("/semaphore_cars", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0); // SEMAPHORES

    semavaipass = sem_open("/semaphore_pass", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);

    assert(semavaicars != SEM_FAILED);

    assert(semavaipass != SEM_FAILED);

    personalmutex = (sem_t **)(calloc(C, sizeof(sem_t *)));

    string s = "d";

    for (int i = 0; i < C; i++)
    {
        s = s + string(i, 1);

        personalmutex[i] = sem_open(s.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1); // PERSONAL MUTEX
    }

    /* Allocating memory for Passenger threads */

    pthread_t *threads_pass;

    threads_pass = (pthread_t *)calloc(P, sizeof(pthread_t));

    Thread_id td_pass;

    td_pass = (Thread_id)calloc(P, sizeof(struct thread_id));

    /* Allocating memory for Car threads */

    pthread_t *threads_car;

    threads_car = (pthread_t *)calloc(C, sizeof(pthread_t));

    Thread_id td_car;

    td_car = (Thread_id)calloc(C, sizeof(struct thread_id));

    /*-----------------------------------*/

    for (int id = 0; id < C; id++)
    {
        td_car[id].id = id;

        td_car[id].pairs = new vector<pair<int, int> >;

        td_car[id].times = new vector<struct timeval>;

        td_car[id].ele = (long double *)calloc(1, sizeof(long double));

        pthread_create(&threads_car[id], NULL, Car, (void *)&td_car[id]);
    }

    for (int id = 0; id < P; id++)
    {
        td_pass[id].id = id;

        td_pass[id].pairs = new vector<pair<int, int> >(k);

        td_pass[id].times = new vector<struct timeval>(3 * k + 2);

        pthread_create(&threads_pass[id], NULL, Passenger, (void *)&td_pass[id]);
    }

    pthread_t tid = threads_pass[0];

    int p = 0;

    while (p < P)
    {
        tid = threads_pass[p];

        pthread_join(tid, NULL); /* Waiting for all threads to end */

        p++;
    }

    exit_VAR = -1;

    p = 0;

    while (p < C)
    {
        sem_post(semavaipass);

        p++;
    }

    p = 0;

    while (p < C)
    {
        tid = threads_car[p];

        pthread_join(tid, NULL);

        p++;
    }

    vector<pair<int, int> > *pairs;

    vector<struct timeval> *times;

    ofstream pointer1("output.txt"); /* ouTPUT FILE POINTER*/

    struct tm *timeinfo1;

    struct tm *timeinfo2;

    time_t now1;

    time_t now2;

    long double average_pass = 0;

    long double average_ride = 0;

    long double *ele;

    for (int j = 0; j < P; j++)
    {
        times = td_pass[j].times;

        pairs = td_pass[j].pairs;

        now1 = times->at(0).tv_sec;

        now2 = times->at(3 * k + 1).tv_sec;

        timeinfo1 = localtime(&now1);

        timeinfo2 = localtime(&now2);

        average_pass = average_pass + (times->at(3 * k + 1).tv_sec - times->at(0).tv_sec) * 1000000L + times->at(3 * k + 1).tv_usec - times->at(0).tv_usec;

        pointer1 << "Passenger " << j << " enter the museum at " << timeinfo1->tm_hour << ":" << timeinfo1->tm_min << ":" << timeinfo1->tm_sec << ":" << times->at(0).tv_usec << endl;

        for (int u = 0; u < k; u++)
        {
            now1 = times->at(3 * u + 1).tv_sec;

            timeinfo1 = localtime(&now1);

            pointer1 << "Passenger " << j << " made ride request at " << timeinfo1->tm_hour << ":" << timeinfo1->tm_min << ":" << timeinfo1->tm_sec << ":" << times->at(3 * u + 1).tv_usec << endl;

            pointer1 << "Car " << pairs->at(u).second << " accepted " << pairs->at(u).first << " 's request" << endl;

            now1 = times->at(3 * u + 2).tv_sec;

            timeinfo1 = localtime(&now1);

            pointer1 << "Passenger " << j << " started riding at " << timeinfo1->tm_hour << ":" << timeinfo1->tm_min << ":" << timeinfo1->tm_sec << ":" << times->at(3 * u + 2).tv_usec << endl;

            now1 = times->at(3 * u + 3).tv_sec;

            timeinfo1 = localtime(&now1);

            pointer1 << "Passenger " << j << " finished riding at " << timeinfo1->tm_hour << ":" << timeinfo1->tm_min << ":" << timeinfo1->tm_sec << ":" << times->at(3 * u + 3).tv_usec << endl;
        }

        pointer1 << "Passenger " << j << " exit the museum at " << timeinfo2->tm_hour << ":" << timeinfo2->tm_min << ":" << timeinfo2->tm_sec << ":" << times->at(3 * k + 1).tv_usec << endl;

        pointer1 << endl;
    }

    average_pass = average_pass / P;

    for (int i = 0; i < C; i++)
    {
        ele = td_car[i].ele;

        average_ride = average_ride + ele[0];
    }

    average_ride = average_ride / C;

    cout << "The average time is : " << average_pass << endl;

    cout << "The average time is : " << average_ride << endl;

    pointer1.close();

    sem_close(semavaicars);

    sem_close(semavaipass);

    for (int i = 0; i < C; i++)
    {
        sem_close(personalmutex[i]);
    }

    sem_unlink("/semaphore_pass");

    sem_unlink("/semaphore_cars");

    string s1 = "d";

    for (int i = 0; i < C; i++)
    {
        s1 = s1 + string(i, 3);

        sem_unlink(s1.c_str());
    }
    /*fREEING MEMORY*/

    free(personalmutex);

    free(threads_car);

    free(threads_pass);

    free(td_car);

    free(td_pass);
}