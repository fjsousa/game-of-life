#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include <string>
#include <time.h>
#include <sstream>

/////////Global variables and values/////////////////////////////////
#define Rows       10
#define Cols       10
#define FRAMES_PER_SECOND 10


//Neighbor Cells                //S SE  E  NE    N   NW  W  SE
int ncol[8]                     = {0, 1, 1,  1,  0, -1, -1, -1};
int nrow[8]                     = {1, 1, 0, -1, -1, -1,  0,  1};

//SDL related  
const int SCREEN_WIDTH  = Cols*12;
const int SCREEN_HEIGHT = Rows*12;
const int SCREEN_BPP    = 32;

SDL_Surface* screen = NULL;
SDL_Event event;
//////////////////////////////////////////////////////////////////////
//The timer Class
//
//////////////////////////////////////////////////////////////////////
class Timer
{
    private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

    public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    int get_ticks();

    //Checks the status of the timer
    bool is_started();
    bool is_paused();
};

//////////////////////////////////////////////////////////////////////
//Load Image function
//
//Recives: image filename
//Returns: pointer to optimized image of type SDL_Surface 
//////////////////////////////////////////////////////////////////////
SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );
	printf("\nImage loaded in function load_image.\n");
    
	//If the image loaded
    if( loadedImage != NULL )
    {

        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );
		printf("\nImage Optimized\n");

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {

            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}
//////////////////////////////////////////////////////////////////////
//SDL Initialization function
//
//Recives: void
//Returns: true if everything initialied ok
//As it is, the function initializes everything, and sets video mode
//And a caption. To change, the function must take what stuff to init
//and the string of the caption.
//////////////////////////////////////////////////////////////////////
bool init()
{
	//Random seed
	srand( time(NULL) );

    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Game Of Life", NULL );

    //If everything initialized fine
    return true;
}

//////////////////////////////////////////////////////////////////////
//State Matrix Initialization by random seed
//////////////////////////////////////////////////////////////////////
bool Fill_Random(int* state)
{
	int cell;

	for (cell = 0 ; cell < Rows*Cols; cell++)
	{
		state[cell] = rand()%2;

		if (state[cell] > 1  )
			return false;

	}

	return true;
}
//////////////////////////////////////////////////////////////////////
//State Matrix Initialization by defined patern
//
//Fills even rows, and in the middle, a dot to collapse the structure
//Makes a simetrical patern
//////////////////////////////////////////////////////////////////////
void Fill_Defined(int* state)
{
	int cell;

	for (cell = 0 ; cell < Rows*Cols; cell++)
	{
		if (cell%2 == 0)
			state[cell] = 1;
		else
			state[cell] = 0;
	}

	state[Cols/2 + Rows/2*Cols ] = 0;
	state[Cols/2+1 + Rows/2*Cols ] = 0;
}
//////////////////////////////////////////////////////////////////////
//Apply Rules Function
//
//////////////////////////////////////////////////////////////////////
void Apply_Rules(int* state, int* state_new, int neighborLifeCells, int currentCell)
{
	if (state[currentCell] == 1)
	{
		if (neighborLifeCells < 2  || neighborLifeCells > 3)
			state_new[currentCell] = 0;
	}

	if (state[currentCell] == 0)
	{
		if (neighborLifeCells == 3)
			state_new[currentCell] = 1;
	}
}
//////////////////////////////////////////////////////////////////////
//Print State Function
//
//Fills the screen Blank and Blits cells
//////////////////////////////////////////////////////////////////////
bool Print_state(int * state, SDL_Surface* source)
{
	int row, col;
	SDL_Rect offset;

    //Fill the screen white
	SDL_FillRect( screen, &screen->clip_rect, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );

	for (row = 0; row < Rows; row++)
	{
		for (col = 0; col < Cols; col++)
		{  
			if(state[col + row*Cols] == 1)
			{
				offset.x = col*(source->w);
				offset.y = row*(source->h);
				SDL_BlitSurface(source, NULL, screen, &offset);
			}
		}
	}

	SDL_Flip( screen );

	return true;
}

//////////////////////////////////////////////////////////////////////
// Print state matrix in file
//
//////////////////////////////////////////////////////////////////////
void Print_state_file(FILE *out, int * state)
{
	int row, col, index;

	for (row = 0; row < Rows; row++)
	{
		for (col = 0; col < Cols; col++)
		{
			index = col + row*Cols;
			fprintf(out,"%d ",state[index]);
		}
		fprintf(out, "\n");
	}
	fprintf(out, "\n");
}
//////////////////////////////////////////////////////////////////////
// Timer Class Methods
//
//////////////////////////////////////////////////////////////////////
Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}

//////////////////////////////////////////////////////////////////////
// Main Function
//
//////////////////////////////////////////////////////////////////////
int main( int argc, char* args[] )
{
	int Nrow, Ncol;
	int n;
	int neighbor_cell;
	int neighborLifeCells = 0;
	int cell, row, col; 
	int cells = Rows*Cols;
	int *state, *state_new, *temp;
	clock_t start, end;
	bool quit = false;
	FILE *out;
    SDL_Surface *cellImg = NULL;
	int frame = 0;
	Timer fps_show, fps_ctrl, update;
	 
	//Open file for writing	
   	//out = fopen("state_and_neighbor_count.txt","w");

	//Initialize SDL
	init();
	
	//Loads cell surface
	cellImg = load_image("cell.jpg");

	if ((state      = (int *) malloc(cells*sizeof(int))) != NULL &&
	    (state_new  = (int *) malloc(cells*sizeof(int))) != NULL )
		 printf("\nState Matrix allocated!\n");
	
	//Fill initial state matrix by defined patern
	Fill_Defined(state);
	Fill_Defined(state_new);

	update.start();
	fps_show.start();

	while ( quit == false)
	{
		fps_ctrl.start();

		for (row = 0; row < Rows; row ++)
		{
			for ( col = 0; col < Cols; col++)
			{
				cell = col + row*Cols;
				
				//iner loop for neighbor cells
				neighborLifeCells = 0;
				for (n = 0; n < 8; n++)
				{
					Nrow = row + nrow[n];
					Ncol = col + ncol[n];
					
					//Toroidal conditions
					if (Nrow < 0)
						Nrow = Rows-1;

					if (Nrow > Rows-1)
						Nrow = 0;

					if (Ncol < 0)
						Ncol = Cols-1;

					if (Ncol > Cols-1)
						Ncol = 0;
				
					neighbor_cell = Ncol + Cols*Nrow;
	
					neighborLifeCells += state[neighbor_cell];

				}

				Apply_Rules(state, state_new, neighborLifeCells, cell);
	
			}	
		}

		//Quiting Stuff
		if( SDL_PollEvent( &event ) )
		{
			//If the user has Xed out the window		
			if( event.type == SDL_QUIT )
			{
				//Quit the program
				quit = true;
			}
		}
		
		//Copy values from state_new to state, while mantaining state_new.
		//Later state_new will be reused and updated only when necessary
		for (cell = 0; cell < cells; cell++)
			state[cell] = state_new[cell];

		//Show next iteration
		Print_state( state, cellImg );

		//fps in caption stuff
		frame++;

		if(  fps_ctrl.get_ticks() < 1000 / FRAMES_PER_SECOND  ) 
		{ 
			//Sleep the remaining frame time 
			SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps_ctrl.get_ticks() ); 
		} 

		//Caption update in every 1/4 of the second
		if( update.get_ticks() > 250 ) 
		{ 
			std::stringstream caption;

			//Calculate the frames per second and create the string 
			caption << "Game of Life Running @ [fps]: " << frame / ( fps_show.get_ticks() / 1000.f ); 
			SDL_WM_SetCaption( caption.str().c_str(), NULL );
	
			//Restart update timer
			update.start();
		}

	}

}
