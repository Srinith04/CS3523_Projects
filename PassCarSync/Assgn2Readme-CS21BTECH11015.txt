-->> CS21BTECH11015 , DASARI SRINITH <<--
            -->> README <<--

->  This is a README for my assignment2 code , where the objective of this assignment is to
    check if given sudoku is valid or not by using Pthreads and OpenMP in C++.
->  The folder has a input.txt file where the numbers of K and N and the values of sudoku
    (input) can be changed.
->  Do not change the name of the input.txt file.
->  If you wish to take input from another file, change the name of text file to input.txt , or 
    change the name of file in code to needed file name.
->  The folder has two src .cpp files , one done using Pthreads and one using OpenMP.
->  The folder also has a sub-folder "Extra_Credit" , which has the .cpp file for extra credit,
    i.e early termination .
    NOTE : MOVE THE "input.txt" file to "Extra_Credit" before executing the extra_credit file.
->  For executing the code , follow the following instructions in the console/terminal in the same 
    directory as the src .cpp file and input.txt file:
        -> FOR "Pthread" file
            ->  g++ -std=c++11 -lpthread Assgn2Srcpthread-CS21BTECH11015.cpp -lm -o pthread
            ->   ./pthread
        -> FOR "OpenMP" file
            ->  g++-12 -fopenmp Assgn2SrcOpenMp-CS21BTECH11015.cpp -lm -o openmp
            ->  ./openmp
        -> FOR "Extra_Credit Pthread" file
            ->  g++ -std=c++11 -lpthread Assgn2Srcpthread-CS21BTECH11015_extracredit.cpp -lm -o pthread_extra
            ->   ./pthread_extra
->  After executing you will find the "Outmain.txt" file which contains the time taken , log ,
    and if sudoku is valid or not.