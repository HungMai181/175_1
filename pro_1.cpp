/*
 * Simple glut demo that can be used as a template for
 * other projects by Garrett Aldrich
 */

/*
 *  Manual:
 *   - This progam assume that the input file is in the correct format.
 *   - To run program use ./pro_1 input_file_name.some_extention.
 *   - Display size is 100 pixels high and 200 pixels wide.
 *   - X-axis goes from 0 to 199.
 *   - Y-axis goes from 0 to 99.
 *   - Press 'q' to quit.
 *   - Press 'l' to toggle between DDA and Bresenham.
 *   - Press 'n' to change to normal mode (default).
 *   - Press 'c' to change to crop mode: left click (and hold) then drag mouse
 *     accross the window to select a rectangular region.
 *   - Press 't' to change to translate mode.
 *   - Press 's' to change to scale mode.
 *   - Press 'r' to change to rotate mode.
 *   - Some transformations require user to click and drag, try to avoid 
 *     changing mode while dragging, unexpected things (glitches) may happen.
 *      
 */

/*

DEBUG: codes to Ctrl+c and Ctrl+v
std::cout<<nPoly<<std::endl;
for(int i=0;i<nPoly;i++){
    std::cout<<nPoint[i]<<std::endl;
    for(int j=0;j<nPoint[i];j++){
        std::cout<<coor[i][j][0]<<" "<<coor[i][j][1]<<std::endl;
    }
}

*/

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>

// ADDED CODES
//     ###
//     ###
//   #######
//    #####
//     ###
//      #
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>



//      #
//     ###
//    #####
//   #######
//     ###
//     ###
// ADDED CODES

/****set in main()****/
//the number of pixels in the grid
int grid_width;
int grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

// ADDED CODES
//     ###
//     ###
//   #######
//    #####
//     ###
//      #
//  number of polygon
int nPoly;
//  array contains number of points for each polygon
int *nPoint;
//  hold coordinates for ppolygons, will be changed when any polygon is transformed
float ***coor;
//  screen buffer, buffer[x-coor][y-coor]
int **buffer;
bool isDDA;
//  used for cropping
int minX, maxX, minY, maxY;
enum Mode
{
    NONE,
    CROP,
    TRAN,
    SCAL,
    ROTA
};
//  transformation mode
Mode mode;
//  previous x, y mouse action coordinates
int preX, preY;


//      #
//     ###
//    #####
//   #######
//     ###
//     ###
// ADDED CODES

void init();
void idle();
void display();
void draw_pix(int x, int y);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

// ADDED CODES
//     ###
//     ###
//   #######
//    #####
//     ###
//      #
void quit();
void setPixel(int x, int y);
void showStatus();
void lineDDA(int x0, int y0, int xEnd, int yEnd);
void lineBres(int x0, int y0, int xEnd, int yEnd);
void resetBound();
void rasterize(int iP);


//      #
//     ###
//    #####
//   #######
//     ###
//     ###
// ADDED CODES


int main(int argc, char **argv)
{

    //the number of pixels in the grid
    grid_width = 200;
    grid_height = 100;
    
    //the size of pixels sets the inital window height and width
    //don't make the pixels too large or the screen size will be larger than
    //your display size
    pixel_size = 5;
    
    /*Window information*/
    win_height = grid_height*pixel_size;
    win_width = grid_width*pixel_size;

    // ADDED CODES
    //     ###
    //     ###
    //   #######
    //    #####
    //     ###
    //      #
    resetBound();
    mode = NONE;
    std::ifstream in(argv[1]);
    in >> nPoly;
    nPoint = new int[nPoly];
    //  allocate space for screen buffer
    buffer = new int*[grid_width];
    for(int x = 0; x < grid_width; x++) {
        buffer[x] = new int[grid_height];
    }
    //  allocate space for each polygon
    coor = new float**[nPoly];
    for(int i = 0; i < nPoly; i++) {
        in >> nPoint[i];
        //  allocate space for points in a polygon
        coor[i] = new float*[nPoint[i]];
        for(int j = 0; j < nPoint[i]; j++) {
            //  allocate space for x, y coordinates and constant 1; (x, y, 1)
            coor[i][j] = new float[3];
            //  read x coordinates
            in >> coor[i][j][0];
            //  read y coordinates
            in >> coor[i][j][1];
            //  constant 1 for the third 'row'
            coor[i][j][2] = 1.0;
        }
    }
    isDDA = true;


    //      #
    //     ###
    //    #####
    //   #######
    //     ###
    //     ###
    // ADDED CODES

	/*Set up glut functions*/
    /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/
    
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    /*initialize variables, allocate memory, create buffers, etc. */
    //create window of size (win_width x win_height
    glutInitWindowSize(win_width,win_height);
    //windown title is "glut demo"
	glutCreateWindow("glut demo");
 
	//defined glut callback functions
	//NOTE: I commented out this function for testing purpose
	glutDisplayFunc(display); //rendering calls here
	glutReshapeFunc(reshape); //update GL on window size change
	glutMouseFunc(mouse);     //mouse button events
	glutMotionFunc(motion);   //mouse movement events
	glutKeyboardFunc(key);    //Keyboard events
	glutIdleFunc(idle);       //Function called while program is sitting "idle"

    //initialize opengl variables
    init();
    //start glut event loop
	glutMainLoop();
	return 0;
}

/*initialize gl stufff*/
void init()
{
    //set clear color (Default background to white)
	glClearColor(0,0,0,1.0);
    //checks for OpenGL errors
	check();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
    //redraw the scene over and over again
	glutPostRedisplay();	
}

//this is where we render the screen
void display()
{
    //clears the screen
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    //clears the opengl Modelview transformation matrix
	glLoadIdentity();

    // ADDED CODES
    //     ###
    //     ###
    //   #######
    //    #####
    //     ###
    //      #
    //  draw all points
    for(int i = 0; i < nPoly; i++) {
        for(int j = 0; j < nPoint[i]; j++) {
            setPixel(round(coor[i][j][0]), round(coor[i][j][1]));
        }
    }
    //  draw lines
    for(int i = 0; i < nPoly; i++) {
        for(int j = 0; j < nPoint[i]-1; j++) {
            if(isDDA) {
                //  draw line between current point and next point, exclude line from first to last point
                lineDDA(coor[i][j][0], coor[i][j][1], coor[i][j+1][0], coor[i][j+1][1]);
            }
            else {
                lineBres(coor[i][j][0], coor[i][j][1], coor[i][j+1][0], coor[i][j+1][1]);
            }
        }
        if(isDDA) {
            //  draw line from first to last point
            lineDDA(coor[i][0][0], coor[i][0][1], coor[i][nPoint[i]-1][0], coor[i][nPoint[i]-1][1]);
        }
        else {
            lineBres(coor[i][0][0], coor[i][0][1], coor[i][nPoint[i]-1][0], coor[i][nPoint[i]-1][1]);
        }
    }

    //  display the pixel that is turned on in screen buffer, any transformation will be acted on screen buffer
	for(int x = minX; x <= maxX; x++) {
        for(int y = minY; y <= maxY; y++)
        {
            if(buffer[x][y]) draw_pix(x, y);
            //  clear screen buffer, prepare for new changes to take place
            buffer[x][y] = 0;
        }
    }
    //      #
    //     ###
    //    #####
    //   #######
    //     ###
    //     ###
    // ADDED CODES

    //blits the current opengl framebuffer on the screen
    glutSwapBuffers();
    //checks for opengl errors
	//check();
}


//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y){
    glBegin(GL_POINTS);
    glColor3f(1.0,0.7,0.0);
    glVertex3f(x+.5,y+.5,0);
    glEnd();
}

/*Gets called when display size changes, including initial creation of the display*/
void reshape(int width, int height)
{
	/*set up projection matrix to define the view port*/
    //update the ne window width and height
	win_width = width;
	win_height = height;
    
    //creates a rendering area across the window
	glViewport(0,0,width,height);
    // up an orthogonal projection matrix so that
    // the pixel space is mapped to the grid space
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0,grid_width,0,grid_height,-10,10);
    
    //clear the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //set pixel size based on width, if the aspect ratio
    //changes this hack won't work as well
    pixel_size = width/(float)grid_width;
    
    //set pixel size relative to the grid cell size
    glPointSize(pixel_size);
    //check for opengl errors
	check();
}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
	switch(ch)
	{
        // ADDED CODES
        //     ###
        //     ###
        //   #######
        //    #####
        //     ###
        //      #
        case 'q': // quit
            quit();
            break;
        case 'l': // toggle line algorithm
            if(isDDA) {
                isDDA = false;
            } 
            else {
                isDDA = true;
            }
            break;
        case 'n': // switch to normal mode
            mode = NONE;
            resetBound();
            break;
        case 'c': // crop mode
            mode = CROP;
            break;
        case 't': // translate mode
            mode = TRAN;
            break;
        case 's': // scale mode
            mode = SCAL;
            break;
        case 'r': // rotate mode
            mode = ROTA;
            break;


        //      #
        //     ###
        //    #####
        //   #######
        //     ###
        //     ###
        // ADDED CODES

		default:
            //prints out which key the user hit
            printf("User hit the \"%c\" key\n",ch);
			break;
	}
    // ADDED CODES
    //     ###
    //     ###
    //   #######
    //    #####
    //     ###
    //      #
    showStatus();
    //      #
    //     ###
    //    #####
    //   #######
    //     ###
    //     ###
    // ADDED CODES

    //redraw the scene after keyboard input
	glutPostRedisplay();
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
    //print the pixel location, and the grid location
    printf ("MOUSE AT PIXEL: %d %d, GRID: %d %d\n",x,y,(int)(x/pixel_size),(int)((win_height-y)/pixel_size));

    // ADDED CODES
    //     ###
    //     ###
    //   #######
    //    #####
    //     ###
    //      #
    int gridX = (int)(x/pixel_size), gridY = (int)((win_height-y)/pixel_size);
    //      #
    //     ###
    //    #####
    //   #######
    //     ###
    //     ###
    // ADDED CODES

    switch(button)
	{
		case GLUT_LEFT_BUTTON: //left button
            printf("LEFT ");
            // ADDED CODES
            //     ###
            //     ###
            //   #######
            //    #####
            //     ###
            //      #
            //  if button down
            if(state == GLUT_DOWN) {
                switch(mode) {
                    case NONE:

                        break;
                    case CROP:
                        preX = gridX;
                        preY = gridY;
                        break;
                    case TRAN:
                        
                        break;
                    case SCAL:
                        
                        break;
                    case ROTA:
                        
                        break;
                    default:
                        break;
                }
            }
            else {
                switch(mode) {
                    case NONE:

                        break;
                    case CROP:
                        if(gridX > preX) { minX = preX; maxX = gridX; }
                        else { minX = gridX; maxX = preX; }
                        if(gridY > preY) { minY = preY; maxY = gridY; }
                        else { minY = gridY; maxY = preY; }
                        break;
                    case TRAN:
                        
                        break;
                    case SCAL:
                        
                        break;
                    case ROTA:
                        
                        break;
                    default:
                        break;
                }
            }
            //      #
            //     ###
            //    #####
            //   #######
            //     ###
            //     ###
            // ADDED CODES
            break;
		case GLUT_RIGHT_BUTTON: //right button
            printf("RIGHT ");
		default:
            printf("UNKNOWN "); //any other mouse button
			break;
	}
    if(state !=GLUT_DOWN)  //button released
        printf("BUTTON UP\n");
    else
        printf("BUTTON DOWN\n");  //button clicked
    
    //redraw the scene after mouse click
    glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
    //redraw the scene after mouse movement
	glutPostRedisplay();
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("GLERROR: There was an error %s\n",gluErrorString(err) );
		exit(1);
	}
}

// ADDED CODES
//     ###
//     ###
//   #######
//    #####
//     ###
//      #
void quit() {
    //TODO: change this so that the output file is the original or is of user choice
    std::ofstream out("outputs.dat");
    out << nPoly << "\n\n";
    for(int i = 0; i < nPoly; i++) {
        out << nPoint[i] << "\n";
        for(int j = 0; j < nPoint[i]; j++) {
            out << std::fixed << std::setprecision(1) << coor[i][j][0] << " " << coor[i][j][1] << "\n";
        }
        out << std::endl;
    }
    for(int i = 0; i < nPoly; i++) {
        for(int j= 0; j < nPoint[i]; j++) {
            delete []coor[i][j];
        }
        delete []coor[i];
    }
    delete []coor;
    delete []nPoint;
    for(int i = 0; i < grid_width; i++) {
        delete []buffer[i];
    }
    delete []buffer;
    exit(0);
}

//  simply turn a pixel on in screen buffer
void setPixel(int x, int y) {
    buffer[x][y] = 1;
}

void showStatus() {
    std::cout << "\nLine Algorithm: ";
    if(isDDA) std::cout << "DDA";
    else std::cout << "Bresenham";
    std::cout << "\nTransformation Mode: ";
    switch(mode) {
        case NONE:
            std::cout << "Normal";
            break;
        case CROP:
            std::cout << "Crop";
            break;
        case TRAN:
            std::cout << "Translate";
            break;
        case SCAL:
            std::cout << "Scale";
            break;
        case ROTA:
            std::cout << "Rotate";
            break;
        default:
            break;
    }
    std::cout << "\n" << std::endl;    
}

// lineDDA copied straight out of the book
void lineDDA(int x0, int y0, int xEnd, int yEnd) {
    int dx = xEnd - x0, dy = yEnd - y0, steps, k;
    float xIncrement, yIncrement, x = x0, y = y0;
    if(fabs(dx) > fabs(dy)) {
        steps = fabs(dx);
    }
    else {
        steps = fabs(dy);
    }
    xIncrement = float(dx) / float(steps);
    yIncrement = float(dy) / float(steps);
    setPixel(round(x), round(y));
    for(k = 0; k < steps; k++) {
        x += xIncrement;
        y += yIncrement;
        setPixel(round(x), round(y));
    }
}

// modified copy of the book's version, add case for m > 1 and negative m
void lineBres(int x0, int y0, int xEnd, int yEnd) {
    int dx = fabs(xEnd - x0), dy = fabs(yEnd - y0);
    //  m < 1 boolean
    bool mlt1 = dx > dy;
    int p = (mlt1 ? 2*dy - dx : 2*dx - dy);
    int twoD = 2*(mlt1 ? dy : dx), twoDMinusD = 2 * (mlt1 ? dy - dx : dx - dy);
    int x, y;
    /* Determine which endpoint to use as start position. */
    //  if(m < 1 and x0 < xEnd or m >= 1 and y0 > yEnd) then switch
    if(mlt1 && x0 > xEnd || !mlt1 && y0 > yEnd) {
        //  switch x0 and xEnd
        x = xEnd;
        xEnd = x0;
        x0 = x;
        //  switch y0 and yEnd
        y = yEnd;
        yEnd = y0;
        y0 = y;
    }
    else {
        x = x0;
        y = y0;
    }  
    setPixel(x, y);
    while(mlt1 && x < xEnd || !mlt1 && y < yEnd) {
        //  if m < 1 then draw along x-axis
        if(mlt1) { x++; }
        else { y++; }
        if(p < 0)
            p += twoD;
        else {
            if(mlt1) {
                if(y0 < yEnd) { y++; }
                else { y--; }
            }
            else {
                if(x0 < xEnd) { x++; }
                else { x--; }
            }
            p += twoDMinusD;
        }
        setPixel(x, y);
    }   
}

void resetBound() {
    minX = minY = 0;
    maxX = grid_width - 1;
    maxY = grid_height - 1;
}

//  true when scan-line hit polygon vertex
bool hitVertex(int x, int y, int iP) {
    //  itterate through all poin in polygon iP
    for(int p = 0; p < nPoint[iP]; p++) {
        if(x == round(coor[iP][p][0]) && y == round(coor[iP][p][1])) { return true;}
    }
    return false;
}
//  true when polygon intersect an edge
bool intersected(int x, int y, int iP, int left, int right, int low, int up) {
    //  itterate through edges and test intersection,
    for(int p = 0; p < nPoint[iP] - 1; p++) {
        float x0 = coor[iP][p][0], y0, xE, yE;
    }
    return false;
}

//  rasterize a polygon, indicated by pIndex
void rasterize(int iP) {
    //  used to hold index of rasterize region
    int start = grid_width, end = 0, upBnd = 0, lowBnd = grid_height;
    //  set rasterize region to be a rectangle, large enough to surround the polygon
    for(int p = 0; p < nPoint[iP]; p++) {
        if(coor[iP][p][0] < start) { start = coor[iP][p][0]; }
        if(coor[iP][p][0] > end) { end = coor[iP][p][0]; }
        if(coor[iP][p][1] < lowBnd) { lowBnd = coor[iP][p][1]; }
        if(coor[iP][p][1] > upBnd) { upBnd = coor[iP][p][1]; }
    }
    //  indicates that the pixeel needed to be rasterize
    bool needFill = false;
    //  start rasterizing
    for(int y = lowBnd; y <= upBnd; y++) {
        for(int x = start; x <= end; x++) {
            //  if scan-line hit vertex, keep the value of needFill
            if(hitVertex(x, y, iP)) {
                //  do nothing
            }
            //  if scan-line intersect edge, toggle value of needFill
            else if(intersected(x, y, iP, start, end, lowBnd, upBnd)) {
                if(needFill) { needFill = false; }
                else { needFill = true; }
            }
            if(needFill) { setPixel(x, y); }
        }
    }
}



//      #
//     ###
//    #####
//   #######
//     ###
//     ###
// ADDED CODES