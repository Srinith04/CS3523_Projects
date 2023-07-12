/*------------------------------------------------------------------------------*/
/* Libraries */
/*------------------------------------------------------------------------------*/

#include <iostream>

#include <vector>

#include <algorithm>

#include <chrono>

#include <math.h>

#include <fstream>

#include <omp.h>

using namespace std;

/*------------------------------------------------------------------------------*/
/* Globally defining the grid to store the values of the sudoko */
/*------------------------------------------------------------------------------*/

vector<int> grid; /* Vector to store the values of the sudoko */

/*------------------------------------------------------------------------------*/
/* Function to check if the row or column or grid is valid */
/*------------------------------------------------------------------------------*/

int check_sudoko(int o, int N)
{
    int type = o / N; /* For which type to check */

    int number = o % N; /* Which number in order of that type to check */

    int n = sqrt(N);

    int k = 0;

    vector<int> elements(N); /*  Vector to store elements for row/column/grid */

    switch (type)
    {
    case 0: /* Type row */
        for (int i = 0; i < N; i++)
        {
            elements[i] = grid[number * N + i];
        }

        break;

    case 1: /* Type column */
        for (int i = 0; i < N; i++)
        {
            elements[i] = grid[i * N + number];
        }

        break;

    case 2: /* Type grid */
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                elements[k] = grid[N * (n * (number / n) + i) + n * (number % n) + j];

                k++;
            }
        }

    default:
        break;
    }

    sort(elements.begin(), elements.end()); /* Sorting the elements array */

    for (int i = 0; i < N - 1; i++)
    {
        if (elements[i] == elements[i + 1]) /* If this is the case then the row/column/grid doesnt have all elements from 0 to 9 */
        {
            return 0;
        }
    }

    return 1; /* Correct row/column/grid */
}

int main()
{
    int N, K;

    ifstream pointer("input.txt"); /* File pointer to read the input */

    pointer >> K; /* Number of threads */

    pointer >> N; /* Size of sudoko grid */

    int value = 0;

    grid.resize(N * N);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            pointer >> grid[i * N + j];
        }
    }

    pointer.close(); /* Closing the file pointer */

    int checked = 0;

    int u = (3 * N) / K;

    int r = (3 * N) % K;

    int *arr = (int *)calloc(3 * N, sizeof(int)); /* Array to store the outputs checked by each thread */

    srand(time(0)); /* Seeding rand function */

    auto started = chrono::high_resolution_clock::now(); /* Start time */

    omp_set_num_threads(K); /* Setting the number of threads to K*/

#pragma omp parallel /* Parallel block of code */
    {
        int id = omp_get_thread_num();

        int count = 0;

        for (int o = id * u; o < (id + 1) * u; o++)
        {
            count = check_sudoko(o, N);

            if (count == 1)
            {
                arr[o] = 1;
            }
            else
            {
                arr[o] = 0;
            }
        }

        if (r > 0)
        {
            count = check_sudoko(K * u + r - 1, N);

            if (count == 1)
            {
                arr[K * u + r - 1] = 1;
            }
            else
            {
                arr[K * u + r - 1] = 0;
            }
        }

        r--;
    }

    auto done = chrono::high_resolution_clock::now(); /* End time after calculating the value of pi */

    auto time = chrono::duration_cast<chrono::microseconds>(done - started);

    /* PRINTING THE LOG/OUTMAIN FILE BASE ON ARRAY 'arr' --> value 0 => checked and is invalid row/columnm/grid
                                                     --> value  1 => checked and is valid row/column/grid   */

    ofstream outmain("Outmain.txt", ios_base::app);

    u = 3 * N / K;

    r = 3 * N % K;

    int type = 0;

    int number = 0;

    for (int id = 0; id < K; id++)
    {
        for (int o = id * u; o < (id + 1) * u; o++)
        {
            type = o / N;

            number = o % N;

            switch (type)
            {
            case 0:
                outmain << "Thread " << id + 1 << " checks row " << number + 1 << " and is ";

                if (arr[o] == 1)
                {
                    outmain << "valid" << endl;
                }
                else
                {
                    outmain << "invalid" << endl;
                }
                break;

            case 1:
                outmain << "Thread " << id + 1 << " checks column " << number + 1 << " and is ";

                if (arr[o] == 1)
                {
                    outmain << "valid" << endl;
                }
                else
                {
                    outmain << "invalid" << endl;
                }
                break;

            case 2:
                outmain << "Thread " << id + 1 << " checks grid " << number + 1 << " and is ";

                if (arr[o] == 1)
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

        if (r > 0)
        {
            type = ((K * u + r - 1)) / N;

            number = ((K * u + r - 1)) % N;

            switch (type)
            {
            case 0:
                outmain << "Thread " << id + 1 << " checks row " << number + 1 << " and is ";

                if (arr[K * u + r - 1] == 1)
                {
                    outmain << "valid" << endl;
                }
                else
                {
                    outmain << "invalid" << endl;
                }
                break;

            case 1:
                outmain << "Thread " << id + 1 << " checks column " << number + 1 << " and is ";

                if (arr[K * u + r - 1] == 1)
                {
                    outmain << "valid" << endl;
                }
                else
                {
                    outmain << "invalid" << endl;
                }
                break;

            case 2:
                outmain << "Thread " << id + 1 << " checks grid " << number + 1 << " and is ";

                if (arr[K * u + r - 1] == 1)
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

        r--;
    }

    u = 0;

    for (int i = 0; i < 3 * N; i++)
    {
        if (arr[i] == 0)
        {
            outmain << "Sudoko is invalid" << endl;

            u = 1;

            break;
        }
    }

    if (u == 0)
    {
        outmain << "Sudoko is valid" << endl;
    }

    outmain << "The total time taken is " << (double)(time.count()) << "\xC2\xB5s" << endl;

    /*------------------------------------------------------------------------------*/
    /* Freeing the memory allocated */
    /*------------------------------------------------------------------------------*/

    free(arr);
}