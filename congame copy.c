#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <ncurses.h>

// Using fake c namespace 'CGM'

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

int CGM_checkPosition(int** map, int xMax, int yMax, int xPos, int yPos){

    if(xPos >= xMax || xPos < 0)
        return 0;
    
    if(yPos >= yMax || yPos < 0)
        return 0;

    return 1;
}

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

void CGM_update(int** map, int xMax, int yMax){

    // variables
    int** updateMap;


    // assign
    updateMap = malloc(sizeof(int*) * xMax);
    for(int i = 0; i < xMax; i++)
        updateMap[i] = malloc(sizeof(int) * yMax);

    // clear the map to start
    CGM_clearMap(updateMap, xMax, yMax);

    // run through each location
    for(int x = 0; x < xMax; x++) {
        
        for(int y = 0; y < yMax; y++) {
            
            // variables
            int count;

            //printf("{%d, %d}\n", x, y);
            // count how many neighbors are populated
            count = 0;
            for(int i = 0; i < MATRIX_SIZE; i++){

                count+= CGM_pullPosition(
                    map,
                    xMax,
                    yMax,
                    x + MATRIX[i][0], 
                    y + MATRIX[i][1]);

               // printf("    [%d, %d]\n", x + MATRIX[i][0], y + MATRIX[i][1]);
            }
        
            if(map[x][y] == CGM_OCCUPIED && 2 <= count && count <= 3)
                updateMap[x][y] = CGM_OCCUPIED;
            else if(map[x][y] == CGM_EMPTIED && count == 3)
                updateMap[x][y] = CGM_OCCUPIED;
            else
                updateMap[x][y] = CGM_EMPTIED;
        }
    }

    for(int x = 0; x < xMax; x++) {
        
        for(int y = 0; y < yMax; y++) {
            
            map[x][y] = updateMap[x][y];
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


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// main function
int main(int argc, char *argv[]) {

    // variables
    int** virtualMap;
    int** updateMap;
    int xMax;
    int yMax;


    // assign
    xMax = MAX_X;
    yMax = MAX_Y;

    virtualMap = malloc(sizeof(int*) * xMax);
    for(int i = 0; i < xMax; i++)
        virtualMap[i] = malloc(sizeof(int) * yMax);

    // initialize ncurses
    initscr();
    noecho();
    cbreak();
    timeout(0);
    curs_set(FALSE);
    getmaxyx(stdscr, CurrentY, CurrentX); // global var `stdscr` is created by the call to `initscr()`


    // create an empty map and randomly fill it up
    CGM_clearMap(virtualMap, xMax, yMax);
    CGM_randomizeMap(virtualMap, xMax, yMax);


    // prefrom 2000 iterations
    for(int i = 0; i < 2000; i++){
        
        CGM_update(virtualMap, xMax, yMax);

        CGM_drawMap(virtualMap, xMax, yMax);
    }

    // end ncurses
    endwin();
}
