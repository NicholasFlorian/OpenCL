#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>

// provide version of OpenCL to use
#define CL_TARGET_OPENCL_VERSION 220

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

// Using fake c namespace 'CGM'

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// OpenCL
#define FILE_NAME           "congame.cl"
#define KERNEL_FUNC         "CGM_update"
#define OFFSET              0


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// rendering data
// global constants
#define CGM_DELAY           5
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
int CGM_checkPosition(int* map, int xMax, int yMax, int xPos, int yPos){

    if(xPos >= xMax || xPos < 0)
        return 0;
    
    if(yPos >= yMax || yPos < 0)
        return 0;

    return 1;
}

int CGM_pullPosition(int* map, int xMax, int yMax, int xPos, int yPos){

    if(xPos >= xMax || xPos < 0)
        return CGM_EMPTIED;
    
    if(yPos >= yMax || yPos < 0)
        return CGM_EMPTIED;

    return map[(xPos * 24) + yPos];
}

void CGM_clearMap(int* map, int xMax, int yMax){

    // go through the entire map and assign all values to 0;
    for(int x = 0; x < xMax; x++)
        for(int y = 0; y < yMax; y++)
            map[(x * 24) + y] = CGM_EMPTIED;  // assign to 0
}

void CGM_randomizeMap(int* map, int xMax, int yMax){

    // go through the entire map and randomly assign values
    for(int x = 0; x < xMax; x++){

        for(int y = 0; y < yMax; y++) {

            // 17% chance the result is a 1
            map[(x * 24) + y] = (rand() % 100) < 17; 
            
            // if it is a 1, let there be a 35% that the neighbor is a 1
            if(map[(x * 24) + y]) {

                // go through each neighbor
                for(int i = 0; i < MATRIX_SIZE; i++) {
    
                    // variables
                    int xNew;
                    int16_t yNew;


                    // assign
                    xNew = x + MATRIX[i][0];
                    yNew = y + MATRIX[i][1];

                    // verify the location to prevent segfault...
                    if(CGM_checkPosition(
                        map,
                        xMax,
                        yMax,
                        xNew, 
                        yNew)){

                        // ...then apply the 35% chance
                        map[(xNew * 24) + yNew] = 
                            (rand() % 100) < 35;
                    }
                }
            }
        }
    }
}

int CGM_drawMap(int* map, int xMax, int yMax){

    // variables
    int c;

    // update max screen size
    getmaxyx(stdscr, CurrentY, CurrentX);


    // clear the ncurses
    clear();

    // go through the entire map and print evey
    for(int x = 0; x < xMax; x++)
        for(int y = 0; y < yMax; y++)
            if(map[(x * 24) + y] == CGM_OCCUPIED) // print out populations
                mvprintw(y, x * 2, CGM_STR_OCCUPIED);
            else
                mvprintw(y, x * 2, CGM_STR_EMPTIED);


    // refresh and delay
    refresh();
    sleep(1);

    // read keyboard and exit if 'q' pressed
    c = getch();
    if (c == 'q')
        return 0;
    else
        return 1;
}

void CGM_drawArray(int* map){


    for(int x = 0; x < 24; x++){
        
        for(int y = 0; y < 24; y++){
            
            if(map[(x * 24) + y])
                printf("x ");
            else
                printf("  ");

        }
        printf("\n");
    }

    printf("\n");
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// open CL functions
cl_device_id CGM_createDevice() {

    // variables
    cl_uint         platformCount;
    cl_platform_id  platform;
    cl_device_id    device;
    int             err;

    // identify a platform
    platfromCount = NULL;
    err = clGetPlatformIDs(1, &platform, &platformCount);
    if(err < 0) {
        perror("Couldn't identify a platform");
        exit(1);
    } 

    // access a device
    err = clGetDeviceIDs(
        platform, 
        CL_DEVICE_TYPE_GPU, 
        1, 
        &device, 
        &platformCount);
    
    if(err == CL_DEVICE_NOT_FOUND) 
        err = clGetDeviceIDs(
            platform, 
            CL_DEVICE_TYPE_CPU, 
            1, 
            &device, 
            &platformCount);

    if(err == CL_DEVICE_NOT_FOUND) 
        err = clGetDeviceIDs(
            platform, 
            CL_DEVICE_TYPE_CPU, 
            1, 
            &device, 
            &platformCount);

    if(err < 0) {
        perror("Couldn't access any devices");
        exit(1);   
    }

    return device;
}

cl_program CGM_buildProgram(cl_context ctx, cl_device_id dev, const char* filename) {

    // variables
    cl_program  program;
    FILE        *file;

    char        *programString; 
    int         programLength;
    
    int         err;
    char        *errorString;
    int         errorLength;


    // open the file
    file = fopen(filename, "r");
    if(file == NULL) {
        perror("Couldn't find the program file");
        exit(1);
    }

    // get the size of the file
    fseek(file, OFFSET, SEEK_END);
    programLength = ftell(file);
    fseek(file, OFFSET, SEEK_SET);

    // read the file,
    programString = (char*)malloc(programLength + 1);
    fread(programString, sizeof(char), programLength, file);
    programString[programLength] = '\0';
    if(programString[0] == '\0'){

        perror("Couldn't read the program file");
        exit(1);
    }


    // create hte program
    program = clCreateProgramWithSource(ctx, 1, 
        (const char**)&programString, 
        &programLength, 
        &err);
    if(err < 0) {
        perror("Couldn't create the program");
        exit(1);
    }

    // build the program
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err < 0) {

        // print out the errors
        clGetProgramBuildInfo(
            program, 
            dev, 
            CL_PROGRAM_BUILD_LOG, 
            0, 
            NULL, 
            &errorLength);

        errorString = malloc(errorLength + 1);
        clGetProgramBuildInfo(
            program, 
            dev, 
            CL_PROGRAM_BUILD_LOG, 
            errorLength + 1, 
            errorString, 
            NULL);
        errorString[errorLength] = '\0';

        perror("COMPILATION FAILED");
        printf("%s\n", errorString);
        free(errorString);
        exit(1);
    }

    // free the string
    free(programString);

    // after reading through the file, close it
    fclose(file);

    return program;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// main function
int main(int argumentSize, char* argumentArray[]) {

    // OpenCL structures
    cl_device_id        device;
    cl_context          context;
    cl_program          program;
    cl_kernel           kernel;
    cl_command_queue    queue;
    cl_int              err;
    size_t              kernalSize;
    size_t              globalSize[2] = {24,24}; // 6 kernals
    size_t              localSize[2] = {24,24};

    // OpenCL buffers
    cl_mem              virtualMapBuffer; 
    cl_mem              updateMapBuffer;

    // data variables
    int*                virtualMap;
    int*                updateMap;
    int                 xMax;
    int                 yMax;

    // ncurses variables
    int                 doOutput;


    
    doOutput = 0;   // false by default
    kernalSize = 1; // one by default

    // handle arguments
    for(int i = 1; i < argumentSize; i++){

        if(strcmp(argumentArray[i], "-n") == 0){

            kernalSize = atoi(argumentArray[++i]);

            localSize[0] = globalSize[0] / kernalSize;
            localSize[1] = globalSize[1] / kernalSize;
        }
        else if(strcmp(argumentArray[i], "-o") == 0){
            
            doOutput = 1;
        }
        else{

            // kill the program if arguments are invalid
            printf("INVALIDE ARGUMENTS");
            exit(1);
        }
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // assign
    // use max x and max y
    xMax = MAX_X; 
    yMax = MAX_Y;

    // allocate memory for our arrays
    virtualMap = malloc(sizeof(int*) * xMax * yMax);
    updateMap = malloc(sizeof(int*) * xMax * yMax);

    // create an empty map and randomly fill it up
    CGM_clearMap(virtualMap, xMax, yMax);
    CGM_randomizeMap(virtualMap, xMax, yMax);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // create device and context, check for an error
    device = CGM_createDevice();
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

    if(err < 0) {

        // return error code
        perror("Couldn't create a context");
        exit(1);
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // build program
    program = CGM_buildProgram(context, device, "congame.cl");


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // create buffers
    virtualMapBuffer = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 
        sizeof(int) * xMax * yMax, 
        virtualMap, 
        &err);
    if(err < 0) {
        perror("Couldn't create a buffer");
        exit(1);   
    }

    updateMapBuffer = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, 
        sizeof(int) * xMax * yMax, 
        updateMap, 
        &err);
    if(err < 0) {
        perror("Couldn't create a buffer");
        exit(1);   
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // create queue
    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
    if(err < 0) {
        
        perror("Couldn't create a command queue");
        exit(1);   
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // create the kernal
    kernel = clCreateKernel(program, KERNEL_FUNC, &err);
    if(err < 0) {
        
        perror("Couldn't create a kernel");
        exit(1);
    }

    err =  clSetKernelArg(kernel, 0, sizeof(cl_mem), &virtualMapBuffer);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &updateMapBuffer);
    if(err < 0) {

        perror("Couldn't create a kernel argument");
        exit(1);
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // initialize ncurses
    // global var `stdscr` is created by the call to `initscr()`
    if(doOutput){

        initscr();
        noecho();
        cbreak();
        timeout(0);
        curs_set(FALSE);
        getmaxyx(stdscr, CurrentY, CurrentX);
    }

    // prefrom 2000 iterations, if if using ncurses until the user hits q
    for(int i = 0; i < 1000 || doOutput; i++){
        
        // variables
        int check;

        check = 1;

        // clear the map on each use
        CGM_clearMap(updateMap, xMax, yMax);

        // queue up the kernal
        err = clEnqueueNDRangeKernel(
            queue, 
            kernel, 
            2, 
            NULL, 
            globalSize, 
            localSize, 
            0, 
            NULL, 
            NULL); 
        if(err < 0) {
            perror("Couldn't enqueue the kernel");
            //endwin();
            exit(1);
        }

        // get the buffer back
        err = clEnqueueReadBuffer(
            queue, 
            updateMapBuffer, 
            CL_TRUE, 
            0, 
            sizeof(int) * xMax * yMax, 
            updateMap, 
            0, 
            NULL, 
            NULL);   

        if(err < 0) {
            perror("Couldn't read the buffer");
            //endwin();
            exit(1);
        }

        clFinish(queue);

        // copy
        for(int x = 0; x < xMax; x++)
            for(int y = 0; y < yMax; y++)
                virtualMap[(x * 24) + y] = updateMap[(x * 24) + y];


        // output
        check = CGM_drawMap(virtualMap, xMax, yMax);
        if(!check)
            break;
    }


    // end ncurses
    if(doOutput)
        endwin();


    // free 
    clReleaseKernel(kernel);
    clReleaseMemObject(virtualMapBuffer);
    clReleaseMemObject(updateMapBuffer);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);
}
