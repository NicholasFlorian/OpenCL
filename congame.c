#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>

#include <OpenCL/cl.h>
#include <CL/cl.h>

// Using fake c namespace 'CGM'

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// OpenCL
#define FILE_NAME           "congame.cl"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// rendering data
// global constants
#define CGM_DELAY           50000 * 4
#define CGM_STR_OCCUPIED    "x"
#define CGM_STR_EMPTIED     " "

// constants
#define MAX_Y               24  // 
#define MAX_X               24  // assign to 24 for assignment

// global variables
int CurrentY = 0;               // store the current size of the screen globally
int CurrentX = 0;               // assign to 0 for now


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// simulation data

// matrix for comparing data
// simulating population
#define CGM_OCCUPIED        1
#define CGM_EMPTIED         0

// matrix demensions
#define MATRIX_SIZE         8
#define MATRIX_DEPTH        2

// matric of values 
int MATRIX[MATRIX_SIZE][MATRIX_DEPTH] = {
    {-1,-1},
    { 0,-1},
    { 1,-1},
    {-1, 0},
    { 1, 0},
    {-1, 1}, 
    { 0, 1},
    { 1, 1}
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// utility code
int CGM_pullPosition(int** map, int xMax, int yMax, int xPos, int yPos){

    if(xPos >= xMax || xPos < 0)
        return CGM_EMPTIED;
    
    if(yPos >= yMax || yPos < 0)
        return CGM_EMPTIED;

    return map[xPos][yPos];
}

void CGM_clearMap(int** map, int xMax, int yMax){

    // go through the entire map and assign all values to 0;
    for(int x = 0; x < xMax; x++)
        for(int y = 0; y < yMax; y++)
            map[x][y] = CGM_EMPTIED;  // assign to 0
}

void CGM_randomizeMap(int** map, int xMax, int yMax){

    // go through the entire map and randomly assign values
    for(int x = 0; x < xMax; x++){

        for(int y = 0; y < yMax; y++) {

            // 17% chance the result is a 1
            map[x][y] = (rand() % 100) < 17; 
            
            // if it is a 1, let there be a 35% that the neighbor is a 1
            if(map[x][y]) {

                // go through each neighbor
                for(int i = 0; i < MATRIX_SIZE; i++) {
    
                    // verify the location to prevent segfault...
                    if(CGM_checkPosition(
                        map,
                        xMax,
                        yMax,
                        x + MATRIX[i][0], 
                        y + MATRIX[i][1])){

                        // ...then apply the 35% chance
                        map[x + MATRIX[i][0]][y + MATRIX[i][1]] = 
                            (rand() % 100) < 35;
                    }
                }
            }
        }
    }
}

void CGM_drawMap(int** map, int xMax, int yMax){

    // update max screen size
    getmaxyx(stdscr, CurrentY, CurrentX);


    // go through the entire map and print evey
    for(int x = 0; x < xMax; x++)
        for(int y = 0; y < yMax; y++)
            if(map[x][y] == CGM_OCCUPIED) // print out populations
                mvprintw(y, x * 2, CGM_STR_OCCUPIED);
            else
                mvprintw(y, x * 2, CGM_STR_EMPTIED);


    // refresh and delay
    refresh();
    usleep(CGM_DELAY);
}

char* CGM_readCode(char* fileName){

    // variables
    FILE* file;
    char* code;
    int fileLength;


    program_handle = fopen(fileName);


    




    return code;

}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// main function
int main(int argc, char *argv[]) {

    // OpenCL structures
    cl_platform_id      platform;
    cl_device_id        device;
    cl_context          context;
    char*               code;
    cl_program          program;
    cl_kernel           kernel;
    cl_command_queue    queue;
    cl_int              err;
    

    // OpenCL buffers
    cl_mem              virtualMapBuffer; 
    cl_mem              updateMapBuffer;

    // data variables
    int**               virtualMap;
    int**               updateMap;
    int                 xMax;
    int                 yMax;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // create platform and check for an error
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);

    if(err < 0) {

        // return error code
        perror("Couldn't create a context");
        exit(1);
    }


    // create device and context, check for an error
    clGetDeviceIds(NULL, CL_DEVICE_TYPE_GPU, 1, &device);
    context =  clCreateContext(NULL, 1, &device, NULL, NULL, &err);

    if(err < 0) {

        // return error code
        perror("Couldn't create a context");
        exit(1);
    }


    // read file




    // build program
    program = build_program(context, device, PROGRAM_FILE);
    


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // assign
    // use max x and max y
    xMax = MAX_X; 
    yMax = MAX_Y;

    // allocate memory for our arrays
    virtualMap = malloc(sizeof(int*) * xMax);
    for(int i = 0; i < xMax; i++)
        virtualMap[i] = malloc(sizeof(int) * yMax);

    updateMap = malloc(sizeof(int*) * xMax);
    for(int i = 0; i < xMax; i++)
        updateMap[i] = malloc(sizeof(int) * yMax);

    // create an empty map and randomly fill it up
    CGM_clearMap(virtualMap, xMax, yMax);
    CGM_randomizeMap(virtualMap, xMax, yMax);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


    // initialize ncurses
    // global var `stdscr` is created by the call to `initscr()`
    initscr();
    noecho();
    cbreak();
    timeout(0);
    curs_set(FALSE);
    getmaxyx(stdscr, CurrentY, CurrentX); 
    

    // prefrom 2000 iterations
    for(int i = 0; i < 2000; i++){
        
        // clear the map
        CGM_clearMap(updateMap, xMax, yMax);




        // clear the map to start
        CGM_drawMap(virtualMap, xMax, yMax);
    }


    // end ncurses
    endwin();

    // free 
}
