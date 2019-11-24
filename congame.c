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


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// rendering data
// global constants
#define CGM_DELAY           50000
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
                /***TODO for(int i = 0; i < MATRIX_SIZE; i++) {
    
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
                }*/
            }
        }
    }
}

void CGM_drawMap(int* map, int xMax, int yMax){

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
    sleep(CGM_DELAY);
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

cl_device_id create_device() {

        
    cl_platform_id platform;
    cl_device_id dev;
    int err;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if(err < 0) {
        perror("Couldn't identify a platform");
        exit(1);
    } 

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
    if(err == CL_DEVICE_NOT_FOUND) {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
    }
    if(err < 0) {
        perror("Couldn't access any devices");
        exit(1);   
    }

    return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// main function
int main(int argc, char *argv[]) {

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

    doOutput = 1;
    kernalSize = 1;




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
    device = create_device();
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);

    if(err < 0) {

        // return error code
        perror("Couldn't create a context");
        exit(1);
    }


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // build program
    program = build_program(context, device, "congame.cl");


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
    for(int i = 0; i < 1000; i++){
        
        // clear the map on each use
        CGM_clearMap(updateMap, xMax, yMax);

        /* Enqueue kernel */
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

        /* Read the kernel's output */
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

        // display
        for(int x = 0; x < xMax; x++)
            for(int y = 0; y < yMax; y++)
                virtualMap[(x * 24) + y] = updateMap[(x * 24) + y];


        // output
       
        CGM_drawMap(virtualMap, xMax, yMax);
        
        //    CGM_drawArray(virtualMap);
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
