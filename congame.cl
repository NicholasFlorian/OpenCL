__kernal void CGM_update(__global int** virtualMap, __global int** updateMap){

    // index variables for OpenCL assignment
    int x;
    int y;

    // variables 
    int count; 
    int matrix[8][2];
    
    
    // assign
    x = get_global_id(0);
    y = get_global_id(1);

    count = 0;

    // create the checking matrix
    matrix = {{-1,-1}, { 0,-1}, { 1,-1}, {-1, 0}, { 1, 0}, {-1, 1}, { 0, 1}, { 1, 1}};


    // count how many neighbors are populated
    for(int i = 0; i < 8; i++){
        
        // variables
        int flag;
        int xNew;
        int yNew;


        // assume true for now
        flag = 1;
    
        // assign new x and y values
        xNew = x + matrix[i][0];
        yNew = y + matrix[i][1];


        // prove false
        if(0 > xNew || xNew >= 24)
            flag = 0;

        if(0 > yNew || yNew >= 24)
            flag = 0;

        // count up
        if(flag)
            count+=matrix[xNew][yNew];
    }

    // update the value based on the count
    if(virtualMap[x][y] == CGM_OCCUPIED && 2 <= count && count <= 3)
        updateMap[x][y] = CGM_OCCUPIED;
    else if(virtualMap[x][y] == CGM_EMPTIED && count == 3)
        updateMap[x][y] = CGM_OCCUPIED;
    else
        updateMap[x][y] = CGM_EMPTIED;
}