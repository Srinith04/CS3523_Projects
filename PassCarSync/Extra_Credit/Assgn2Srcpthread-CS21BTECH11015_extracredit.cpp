/*------------------------------------------------------------------------------*/
/* Libraries */
/*------------------------------------------------------------------------------*/

#include <iostream>

#include <vector>

#include <algorithm>

#include <chrono>

#include <math.h>

#include <fstream>

using namespace std;

/*------------------------------------------------------------------------------*/
/* Globally defining the grid to store the values of the sudoko */
/*------------------------------------------------------------------------------*/

vector<int> grid;   /* Vector to store the values of the sudoko */

int* exi;   /* Pointer to an int array of size 1 used for exiting all threads if one invalid row/column/grid is found */

/*------------------------------------------------------------------------------*/
/* Struct containing things related to thread_id */
/*------------------------------------------------------------------------------*/

struct thread_id
{
    int id;

    int K;  /* Number of threads */

    int N;  /* The number of numbers */

    int r;

    int* arr;   /* Array for threads to store their outputs for their checks */
};

typedef struct thread_id *Thread_id;

/*------------------------------------------------------------------------------*/
/* Function to check if row/column/grid is valid or invalid */
/*------------------------------------------------------------------------------*/

int check_sudoko(int o , int N)
{
    int type = o/N ;    /* For which type to check */

    int number = o%N ;  /* Which number in order of that type to check */

    int n = sqrt(N);

    int k = 0;

    vector<int> elements(N);

    switch (type)
    {
        case 0: // Row
            for(int i=0;i<N;i++)
            {
                elements[i] = grid[number*N + i];
            }

            break;

        case 1: // Column
            for(int i=0;i<N;i++)
            {
                elements[i] = grid[i*N + number];
            }

            break;

        case 2: // Grid
            for(int i=0;i<n;i++)
            {
                for(int j =0;j<n;j++)
                {
                    elements[k] = grid[N*(n*(number/n) + i) + n*(number%n) +j];

                    k++;
                }
            }
    
            default:
                break;
    } 

    sort(elements.begin(),elements.end());  /* Sorting in ascending order */

    for(int i=0;i<N-1;i++)
    {
        if(elements[i] == elements[i+1])    /* Invalid row/column/grid */
        {
            return 0;   
        }
    }
    
    return 1;
}

/*------------------------------------------------------------------------------*/
/* Function which is run on each of K threads */
/*------------------------------------------------------------------------------*/

void *thread_function(void *arg)
{
    Thread_id data = (Thread_id)arg;

    int id = data->id;

    int N = data->N;

    int K = data->K;

    int u = N/K;

    int r = data->r;

    int* arr = data->arr;

    int count = 0;

    for (int o = id * u ; o < (id + 1) * u; o++)
    {
        if(exi[0] == 1) /* Meaning one thread has found an invalid row/column/grid , so exiting all threads */
        {
            pthread_exit(0);
        }

        count = check_sudoko(o,(N/3));

        if(count == 1)
        {
            arr[o] = 1;
        }
        else
        {
            arr[o] = -1;

            exi[0] = 1; /* Setting counter to one */

            pthread_exit(0);
        }
    }
    
    if(r > 0)
    {
        if(exi[0] == 1)
        {
            pthread_exit(0);
        }

        count = check_sudoko(K*u+r-1,(N/3));

        if(count == 1)
        {
            arr[K*u + r - 1] = 1;
        }
        else
        {
            arr[K*u + r - 1] = -1;

            exi[0] = 1; /* Setting counter to one */

            pthread_exit(0);
        }
    }

    pthread_exit(0);
}

int main()
{
    int N, K;

    ifstream pointer("input.txt"); /* File pointer to read the input */

    pointer >> K;   /* Number of threads */
    
    pointer >> N;   /* Size of sudoko grid */

    int value = 0;

    grid.resize(N*N);  

    for(int i=0;i<N;i++)
    {
        for(int j=0;j<N;j++)
        {
            pointer >> grid[i*N + j];   /* Populating the grid */
        }
    }

    pointer.close();    /* Closing the file pointer */

    int checked = 0;

    pthread_t *threads;

    threads = (pthread_t *)calloc(K, sizeof(pthread_t));

    Thread_id td;

    td = (Thread_id)calloc(K, sizeof(struct thread_id));  

    int r = (3*N)%K;

    int* arr = (int*)calloc(3*N , sizeof(int));

    exi = (int*)calloc(1,sizeof(int));

    exi[0] = 0;

    srand(time(0)); /* Seeding rand function */

    auto started = chrono::high_resolution_clock::now(); /* Start time */

    for (int id = 0; id < K; id++)
    {
        td[id].K = K;

        td[id].N = 3*N;

        td[id].id = id;

        td[id].r = r;

        td[id].arr = arr;

        pthread_create(&threads[id], NULL, thread_function, (void *)&td[id]);

        r--;
    }

    pthread_t tid = threads[0];

    int p = 0;

    while (p < K)
    {
        tid = threads[p];

        pthread_join(tid,NULL);  /* Waiting for all threads to join */

        p++;
    };

    auto done = chrono::high_resolution_clock::now(); /* End time */

    auto time = chrono::duration_cast<chrono::microseconds>(done-started);

    /* PRINTING THE LOG/OUTMAIN FILE BASE ON ARRAY 'arr' --> value  0 => not checked by any thread
                                                         --> value  1 => checked and is valid row/column/grid
                                                         --> value -1 => checked and is invalid row/columnm/grid */

    ofstream outmain("Outmain.txt",ios_base::app);

    int u = 3*N/K;

    r = 3*N%K;

    int type = 0;

    int number = 0;

    for(int id=0;id<K;id++)
    {
        for (int o = id * u ; o < (id + 1) * u; o++)
        {
            if(arr[o] != 0)
            {
                type = o/N ;

                number = o%N ;

                switch (type)
                {
                    case 0:
                        outmain << "Thread " << id+1 << " checks row " << number+1 << " and is ";

                        if(arr[o] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;

                    case 1:
                        outmain << "Thread "<< id +1<< " checks column " << number+ 1 << " and is ";

                        if(arr[o] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;

                    case 2:
                        outmain << "Thread " << id +1<< " checks grid " << number+ 1 << " and is ";

                        if(arr[o] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;
                
                    default:
                        break;
                } 
            }
        }
        
        if(r > 0)
        {
            if(arr[K*u + r - 1] != 0)
            {
                type = ((K*u + r - 1))/N;

                number = ((K*u + r - 1))%N ;

                switch (type)
                {
                    case 0:
                        outmain << "Thread "<<id + 1<<" checks row "<<number+1<<" and is ";

                        if(arr[K*u + r - 1] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;

                    case 1:
                        outmain << "Thread "<< id+1 << " checks column " << number+ 1 << " and is ";

                        if(arr[K*u + r - 1] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;

                    case 2:
                        outmain << "Thread " << id+1 << " checks grid " << number+ 1 << " and is ";

                        if(arr[K*u + r - 1] == 1)
                        {
                            outmain << "valid" << endl;
                        }
                        else
                        {
                            outmain << "invalid" << endl;
                        }
                        break;
                    
                    default:
                        break;
                } 
            }
        }

        r--;
    }

    if(exi[0] == 1)
    {
        outmain << "Sudoko is invalid" << endl;
    }
    else
    {
        outmain << "Sudoko is valid" << endl;
    }

    outmain << "The total time taken is " << (double)(time.count()) << "\xC2\xB5s"  << endl;

    /*------------------------------------------------------------------------------*/
    /* Freeing the memory allocated */
    /*------------------------------------------------------------------------------*/

    free(threads);

    free(td);

    free(exi);

    free(arr);
}