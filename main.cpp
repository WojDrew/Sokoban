#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./sdl-2.0.7/include/SDL.h"
#include"./sdl-2.0.7/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	524
#define CHEST 0
#define FREE 1
#define WALL 2
#define TARGET 3
#define TARGETCHEST 4
#define CHESTS 5
#define MAX_CHESTS 10
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define FALSE 0
#define TRUE 1



typedef struct pole {
	int type;
	int x;
	int y;
	SDL_Surface *brick;
} pole_t;


typedef struct man {
	int x;
	int y;
	SDL_Surface *player;
} men_t;

typedef struct chest {
	int x;
	int y;
	SDL_Surface *chs;
} chest_t;

void createwall(pole_t **plansza);
void drawwall(pole_t **plansza, SDL_Surface *screen);
void createmen(pole_t **plansza,  men_t *man);
void drawmen(men_t men, SDL_Surface *screen);
void createdefaultgame(pole_t **plansza, chest_t *chests, int *max_chests_x, int *max_chests_y);
void drawchests(SDL_Surface *screen, chest_t *chests);
void colision(pole_t **plansza, men_t *men, chest_t *chests, int tryb, int *win);
void createtargets(pole_t **plansza, int *max_targets_x, int *max_targets_y);
void drawtargets(pole_t **plansza, SDL_Surface *screen, int *max_targets_x, int *max_targets_y);



// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc, x = 1, y = 1;
	double delta, worldTime, fpsTimer, fps, distance;
	int win = 0;

	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	pole_t **plansza;
	men_t men;
	chest_t *chests;
	int *max_chests_x;
	int *max_chests_y;
	int *max_targets_x;
	int *max_targets_y;

	chests = new chest_t[CHESTS];
	max_chests_x = new int[MAX_CHESTS];
	max_chests_y = new int[MAX_CHESTS];
	max_targets_x = new int[MAX_CHESTS];
	max_targets_y = new int[MAX_CHESTS];
	
	max_chests_x[0] = 2; max_chests_y[0] = 2;
	max_chests_x[1] = 2; max_chests_y[1] = 3;
	max_chests_x[2] = 5; max_chests_y[2] = 2;
	max_chests_x[3] = 4; max_chests_y[3] = 2;
	max_chests_x[4] = 4; max_chests_y[4] = 3;
	max_chests_x[5] = 3; max_chests_y[5] = 2;
	max_chests_x[6] = 5; max_chests_y[6] = 4;
	max_chests_x[7] = 4; max_chests_y[7] = 1;
	max_chests_x[8] = 6; max_chests_y[8] = 1;
	max_chests_x[9] = 6; max_chests_y[9] = 2;

	max_targets_x[0] = 1; max_targets_y[0] = 2;
	max_targets_x[1] = 2; max_targets_y[1] = 1;
	max_targets_x[2] = 6; max_targets_y[2] = 3;
	max_targets_x[3] = 4; max_targets_y[3] = 4;
	max_targets_x[4] = 6; max_targets_y[4] = 4;
	max_targets_x[5] = 3; max_targets_y[5] = 3;
	max_targets_x[6] = 5; max_targets_y[6] = 1;
	max_targets_x[7] = 1; max_targets_y[7] = 4;
	max_targets_x[8] = 4; max_targets_y[8] = 4;
	max_targets_x[9] = 5; max_targets_y[9] = 4;


	men.x = 1;
	men.y = 1;


	plansza = new pole_t*[6];
	for (int i = 0; i<6; i++)
	{
		plansza[i] = new pole_t[8];
	}
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 8; j++)
			plansza[i][j].type = FREE;
	}

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);

	eti = SDL_LoadBMP("./eti.bmp");
	if(eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;
	

	
	createwall(plansza);
	createtargets(plansza, max_targets_x, max_targets_y);
	createmen(plansza, &men);
	createdefaultgame(plansza, chests, max_chests_x, max_chests_y);
	


	while(!quit) {
		t2 = SDL_GetTicks();
		
		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;


		SDL_FillRect(screen, NULL, czarny);

		drawwall(plansza, screen);
		drawtargets(plansza, screen, max_targets_x, max_targets_y);
		drawmen(men, screen);
		drawchests(screen, chests);


		fpsTimer += delta;
		if(fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
			};

		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
		sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		//	      "Esc - exit, \030 - faster, \031 - slower"
		if (win == CHESTS)
		{
			sprintf(text, "Wygrales!!!");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
		}

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);



		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE)
					{
						quit = 1;
						delete[] chests;
						delete[] max_chests_x;
						delete[] max_chests_y;
						delete[] max_targets_x;
						delete[] max_targets_y;
						for (int i = 0; i<6; i++)
						{
							delete[] plansza[i];
						}
						delete[] plansza;
					}
					else if (event.key.keysym.sym == SDLK_UP && men.y > 1)
					{
						men.y--;
						colision(plansza, &men, chests, UP, &win);
					}
					else if (event.key.keysym.sym == SDLK_DOWN && men.y < 4)
					{
						men.y++;
						colision(plansza, &men, chests, DOWN, &win);
					}
					else if (event.key.keysym.sym == SDLK_LEFT && men.x > 1)
					{
						men.x--;
						colision(plansza, &men, chests, LEFT, &win);
					}
					else if (event.key.keysym.sym == SDLK_RIGHT && men.x < 6)
					{
						men.x++;
						colision(plansza, &men, chests, RIGHT, &win);
					}
					else if (event.key.keysym.sym == SDLK_n)
					{
						for (int i = 0; i < 6; i++)
						{
							for (int j = 0; j < 8; j++)
							{
								if (i == 0 || i == 5 || j == 0 || j == 7)
									plansza[i][j].type = WALL;
								else
									plansza[i][j].type = FREE;
							}
						}
						createdefaultgame(plansza, chests, max_chests_x, max_chests_y);
						createtargets(plansza, max_targets_x, max_targets_y);
						men.x = 1;
						men.y = 1;
						worldTime = 0;
						win = 0;
					}
					break;
				case SDL_KEYUP:
					
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};



void createwall(pole_t **plansza)
{
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (i == 0 || i == 5 || j == 0 || j == 7)
			{
				plansza[i][j].brick = SDL_LoadBMP("./brick.bmp");
				plansza[i][j].brick->w = 80;
				plansza[i][j].brick->h = 80;
				plansza[i][j].type = WALL;
			}
		}
	}
}

void drawwall(pole_t **plansza, SDL_Surface *screen)
{
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (i == 0 || i == 5 || j == 0 || j == 7 )
				DrawSurface(screen, plansza[i][j].brick, j * 80 + 40, i * 80 + 84);
		}
	}
}

void createmen(pole_t **plansza, men_t *men)
{
	men->player = SDL_LoadBMP("./man.bmp");
	men->player->w = 80;
	men->player->h = 80;
}

void drawmen(men_t man, SDL_Surface *screen)
{
	DrawSurface(screen, man.player, man.x * 80 + 40, man.y * 80 + 84);
}

void createdefaultgame(pole_t **plansza, chest_t *chests, int *max_chests_x, int *max_chests_y)
{
	for (int i = 0; i < CHESTS; i++)
	{
		chests[i].chs = SDL_LoadBMP("./skrzynia.bmp");
		chests[i].x = max_chests_x[i];
		chests[i].y = max_chests_y[i];
		plansza[chests[i].y][chests[i].x].type = CHEST;
	}
}

void drawchests(SDL_Surface *screen, chest_t *chests)
{
	for(int i = 0; i < CHESTS; i++)
		DrawSurface(screen, chests[i].chs, chests[i].x * 80 + 40, chests[i].y * 80 + 84);
}

void colision(pole_t **plansza, men_t *men, chest_t *chests, int tryb, int *win)
{
	if (plansza[men->y][men->x].type == CHEST || plansza[men->y][men->x].type == TARGETCHEST)
	{
		for (int i = 0; i < CHESTS; i++)
		{
			if (men->x == chests[i].x && men->y == chests[i].y)
			{
				switch (tryb)
				{
				case UP:
					if (plansza[men->y - 1][men->x].type != CHEST && plansza[men->y - 1][men->x].type != WALL && plansza[men->y - 1][men->x].type != TARGETCHEST)
					{
						chests[i].y--;
						if (plansza[men->y - 1][men->x].type == TARGET)
						{
							plansza[men->y - 1][men->x].type = TARGETCHEST;
							(*win)++;
						}
						else
							plansza[men->y - 1][men->x].type = CHEST;
						if (plansza[men->y][men->x].type == TARGETCHEST)
						{
							plansza[men->y][men->x].type = TARGET;
							(*win)--;
						}
						else
							plansza[men->y][men->x].type = FREE;
					}
					else
						men->y++;
					break;
				case DOWN:
					if (plansza[men->y + 1][men->x].type != CHEST && plansza[men->y + 1][men->x].type != WALL && plansza[men->y + 1][men->x].type != TARGETCHEST)
					{
						chests[i].y++;
						if (plansza[men->y + 1][men->x].type == TARGET)
						{
							plansza[men->y + 1][men->x].type = TARGETCHEST;
							(*win)++;
						}
						else
							plansza[men->y + 1][men->x].type = CHEST;
						if (plansza[men->y][men->x].type == TARGETCHEST)
						{
							plansza[men->y][men->x].type = TARGET;
							(*win)--;
						}
						else
							plansza[men->y][men->x].type = FREE;
					}
					else
						men->y--;
					break;
				case LEFT:
					if (plansza[men->y][men->x - 1].type != CHEST && plansza[men->y][men->x - 1].type != WALL && plansza[men->y][men->x - 1].type != TARGETCHEST)
					{
						chests[i].x--;
						if (plansza[men->y][men->x - 1].type == TARGET)
						{
							plansza[men->y][men->x - 1].type = TARGETCHEST;
							(*win)++;
						}
						else
							plansza[men->y][men->x - 1].type = CHEST;
						if (plansza[men->y][men->x].type == TARGETCHEST)
						{
							plansza[men->y][men->x].type = TARGET;
							(*win)--;
						}
						else
							plansza[men->y][men->x].type = FREE;
					}
					else
						men->x++;
					break;
				case RIGHT:
					if (plansza[men->y][men->x + 1].type != CHEST && plansza[men->y][men->x + 1].type != WALL && plansza[men->y][men->x + 1].type != TARGETCHEST)
					{
						chests[i].x++;
						if (plansza[men->y][men->x + 1].type == TARGET)
						{
							plansza[men->y][men->x + 1].type = TARGETCHEST;
							(*win)++;
						}
						else
							plansza[men->y][men->x + 1].type = CHEST;
						if (plansza[men->y][men->x].type == TARGETCHEST)
						{
							plansza[men->y][men->x].type = TARGET;
							(*win)--;
						}
						else
							plansza[men->y][men->x].type = FREE;
					}
					else
						men->x--;
					break;
				}
			}
		}
	}
}

void createtargets(pole_t **plansza, int *max_targets_x, int *max_targets_y)
{
	for (int i = 0; i < CHESTS; i++)
	{
		plansza[max_targets_y[i]][max_targets_x[i]].brick = SDL_LoadBMP("./target.bmp");
		plansza[max_targets_y[i]][max_targets_x[i]].type = TARGET;
	}
}

void drawtargets(pole_t **plansza, SDL_Surface *screen, int *max_targets_x, int *max_targets_y)
{
	for (int i = 0; i < CHESTS; i++)
		DrawSurface(screen, plansza[max_targets_y[i]][max_targets_x[i]].brick, max_targets_x[i] * 80 + 40, max_targets_y[i] * 80 + 84);
}