##### Parallel Design Patterns - Semester 2
##### Student Exam Number: B159973
##### University of Edinburgh @ 2019- 2020
# Squirrel Parapoxvirus

## Assignment Introduction

The program simulates the squirrel parapoxvirus for 24 months. The squirrels can take a number of steps during the month and communicate with cells and update
theirselfs. A month lasts about 2 seconds and after that the cells stop communication with the squirrels and update theirselfs.
When that is finished, the communication starts again. The squirrels can catch a disease, give birth to other squirrels and die. 
When we reach 24 months in the simulation, the cells communicate with the Master and ask him to terminate the program. 

Assignment pdf file can be found here [here](ext/PDP_assignment.pdf).

## Dependencies

On @Cirrus it runs properly by only loading the module:
1. module load mpt

If you want the latest MPI library by intel you can load:
1. module  load intel-compilers-17
2. module  load intel-mpi-17 

The program needs gcc/intel compiler in order to compile with mpi 0.
#### GCC Compiler
 Please install the gcc compiler [GNU](https://gcc.gnu.org/).
The following version can run the program.
```sh
$ gcc -v
 gcc version 4.8.5(GCC) (Red Hat 4.8.5-11)
```
#### Mpirun
Please install the mpirun if needed. [MPIRUN](https://www.open-mpi.org/doc/v4.0/man1/mpirun.1.php)
```sh
mpirun --version
MPT: libxmpi.so 'HPE MPT 2.18  10/02/18 04:03:22'
```
#### Doxygen:
The header files and all the program are well commented. To generate a documentation in other formats rather than plain text might come handy. 
The project includes a doxygen configuration file. 
Install & Download [Doxygen](http://www.doxygen.org/download.html) if you want a good documentation in html files.
Find the  `index.html` and open it with a modern browser:

```sh
$ doxygen configDoxygen
$ cd doc
$ cd html
$ cat index.html (open with browser)
```

## Install

#### Makefile:
The program is builded  completely via make. First it compiles the`*.c` files found in `/src` folder and then moves the object files under the `/obj` folder. After than it links the files together and produces the binary executable  in `/bin` folder under the name `parapoxvirus.exe`. 
There are several commands that makefile can take. All the commands must run inside the **project root directory** where the `Makefile `is located.
To **build** the program run the following:
```sh
$ make all
```
To **clean** the  `*.o` object files and the executable under `/obj` and the executable along with its output, issue:
```sh
$ make clean
```
To **print**  variables of the makefile issue:
```sh
$ make print
```

## Run

#### Run Parameters
The program intially starts with these parameters:

1. **_MAX_SQUIRRELS** : 200

2. **_NUM_INIT_SQUIRRELS**: 34

3. **_MAX_MONTHS_SIMULATION**: 24

4. **UNHEALTHY_SQUIRRELS**: 4

5. **_NUM_CELLS**: 16

6. **MONTH_DURATION**:  2

7. **MAX_SQUIRRELS_LAZINESS**: 20 

These constants are defined in `main.h`. It is very important to mention that `getCellFromPosition()` which is given by the examiner limits the cells to be only 16. The program however can work if that function is changed to support more cells. Also, a month lasts about 2 seconds (2000ms). The maximum squirrel delay is 20 milliseconds. 

#### Run executable
After the make command, the program can run via `parapoxvirus` which is located in`\bin`folder:

Submission Script:
```sh
$ qsub submission.pbs
$ cat months.out (after execution)
```

Locally:
```sh
$ cd bin
$ chmod +x  parapoxvirus
$ mpirun -n 4 ./parapoxvirus
```

## Results 
The results are printed on `stdout` by running locally. If you run using the submission script, a file called `months.out` will be created. 
