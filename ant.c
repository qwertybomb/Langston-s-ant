/* standard headers */
#include <stdint.h>
#include <stdbool.h>

/* SDL2 headers */
#include <SDL.h>

#define WINDOW_WIDTH (1920)
#define WINDOW_HEIGHT (1080)
#define SCALE (1)
#define WIDTH (WINDOW_WIDTH/SCALE)
#define HEIGHT (WINDOW_HEIGHT/SCALE)
#define N_ANTS (2)
#define STEPS_PER_FRAME (100)

/* RL is for the regular Langston's ant */
static const char turns[] =  "RLRL";
static const uint32_t n_turns = (sizeof(turns) - 1) / sizeof(turns[0]);

uint32_t rgb(double ratio)
{ 
    /* we want to normalize ratio so that it fits in to 6 regions */
    /* where each region is 256 units long */
    int32_t normalized = (int32_t)(ratio * 256 * 6);

    /* find the distance to the start of the closest region */
    uint32_t x = (uint32_t)normalized & 255u;

    uint32_t red = 0, grn = 0, blu = 0;
    switch(normalized >> 8)
    {
    case 0: red = 255;      grn = x;        blu = 0;       break; /* red     */
    case 1: red = 255 - x;  grn = 255;      blu = 0;       break; /* yellow  */
    case 2: red = 0;        grn = 255;      blu = x;       break; /* green   */
    case 3: red = 0;        grn = 255 - x;  blu = 255;     break; /* cyan    */
    case 4: red = x;        grn = 0;        blu = 255;     break; /* blue    */
    case 5: red = 255;      grn = 0;        blu = 255 - x; break; /* magenta */
    }

    return (red << 24u) | (grn << 16u) | (blu << 8u);
}

uint32_t get_state(uint32_t *grid, uint32_t x, uint32_t y)
{
	return grid[y * WIDTH + x] & 255u;
}

void set_state(uint32_t *grid, uint32_t x, 
	uint32_t y, uint32_t state)
{
	 grid +=  y * WIDTH + x;
	 *grid = state ? rgb(state/(double)n_turns) : 0;
	 *grid |= state;
}

typedef enum DIR
{
	ANTUP,
	ANTRIGHT,
	ANTDOWN,
	ANTLEFT
} DIR;

/* this allows for multiple ants */
typedef struct ant
{
	int32_t x, y;
	/* avoid modulos */
	uint32_t dir : 2;
} ant;

void turn_ant(ant *self, DIR dir)
{
	self->dir += dir > 1 ? -1 : 1;
}

void move_ant(ant *self)
{
	switch(self->dir)
	{
		case ANTUP:
		{
			self->y--;
		} break;

		case ANTRIGHT:
		{
			self->x++;
		} break;

		case ANTDOWN:
		{
			self->y++;
		} break;

		case ANTLEFT:
		{
			self->x--;
		} break;
	}
}

void update_ant(ant *self, uint32_t *grid)
{

	/* get current state */
	uint32_t state = get_state(grid, self->x, self->y);	
	state = (state % n_turns);

	/* turn ant */
	switch (turns[state])
	{
		/* we don't need to do any thing for 'F' since we are already going to move forward */
		case 'F': break;
		case 'B':
		{
			self->dir += 2;
		} break;
		default:
		{
			turn_ant(self, turns[state] == 'L' ? ANTRIGHT : ANTLEFT);
		} break;
	}
	set_state(grid, self->x, self->y, ++state % n_turns);

	/* move ant */
	move_ant(self);

	/* prevent ant from going out of bounds */
	self->x = (self->x % WIDTH + WIDTH) % WIDTH;
	self->y = (self->y % HEIGHT + HEIGHT) % HEIGHT;
}

#define create_SDL(type, var, ...) \
    type var = __VA_ARGS__;        \
    if (var == NULL)               \
    {                              \
        perror(SDL_GetError());	   \
        return EXIT_FAILURE;       \
    }                              \

int main(int argc, char *argv[])
{

	(void)argc;
	(void)argv;

	if(SDL_Init(SDL_INIT_VIDEO < 0))
	{
		/* print error message to stderr */
		perror(SDL_GetError());
		return EXIT_FAILURE;		
	}

	/* setup SDL */

	create_SDL(SDL_Window*, window, 
		SDL_CreateWindow("",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL));

	create_SDL(SDL_Renderer*, renderer, 
		SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED));

	create_SDL(SDL_Texture*, texture,
		SDL_CreateTexture(renderer, 
		SDL_PIXELFORMAT_RGBA8888, 
		SDL_TEXTUREACCESS_STREAMING, 
		WIDTH, HEIGHT));
	SDL_Event event;
	bool quit = false;


	/* create grid */
	uint32_t *grid = calloc(WIDTH * HEIGHT, sizeof(uint32_t));

	/* setup ants */
	ant ants[N_ANTS];
	for(uint32_t i = 0; i < N_ANTS; ++i) 
	ants[i] = (ant){ .x = WIDTH / 2 - i * 260,
	  .y = HEIGHT / 2 + i * -5,
	  .dir = i * 2};

	while(!quit)
	{
		while(SDL_PollEvent(&event))
			switch(event.type)
			{
				case SDL_QUIT:
				{
					quit = true;
				} break;
			}
		/* update ant(s) */
		for(uint32_t j = 0; j < STEPS_PER_FRAME; ++j) 
		{	
			for(uint32_t i = 0; i < N_ANTS; ++i)
			{
				update_ant(&ants[i], grid);
			}
		}
		/* update texture and draw it to screen */
		SDL_UpdateTexture(texture, NULL, grid, sizeof(uint32_t) * WIDTH);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	/* cleanup SDL and free memory */
	free(grid);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
