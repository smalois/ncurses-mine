#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h> 

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define ROWS	15	
#define COLUMNS 15

typedef struct {
	double x;
	double y;
	double xVeloc;
	double yVeloc;
	double inertia;
	int revealed;
	int color;
	char val[2];
} block;

typedef struct grid {
	int columns;
	int rows;
	int num_mines;
	int **array;
	int **minelist;
} grid;

grid g = {.columns = COLUMNS, .rows = ROWS, .num_mines = 20};

void init_blocks(block ***);
void allocate_arrays();
void fill_board();
void generate_mines();
void free_mem();
void draw_blocks(block **);
void update_blocks(block **);
void draw_cursor(block);
void explode(block **);
void prep_explosion(block ***blocks, int bombX, int bombY);
void reveal_surrounding(block ***, int, int);

int main(int argc, char *argv[]) {
	int ch;

	srandom(time(NULL));
	
	block **blocks;

	initscr();
	cbreak();
	noecho();
	curs_set(0);

	start_color();

	allocate_arrays();
	generate_mines();
	fill_board();

	block cursor = {.x = 0, .y = 0, .color = 4, .val = " "};
	init_blocks(&blocks);

	while(1) {
		draw_blocks(blocks);
		draw_cursor(cursor);
		ch = getch();
		switch(ch) {
			case(32): //space
				blocks[(int)cursor.y][(int)cursor.x].revealed = 1;
				if(strcmp(blocks[(int)cursor.y][(int)cursor.x].val, "9") == 0) {
					prep_explosion(&blocks, (int) cursor.x, (int) cursor.y);
					explode(blocks);
				}
				if(strcmp(blocks[(int)cursor.y][(int)cursor.x].val, "0") == 0) {
					reveal_surrounding(&blocks, (int)cursor.x, (int)cursor.y);
				}
				break;
			case(108):	//right
				cursor.x = min(COLUMNS - 1, cursor.x + 1);
				break;
			case(104):	//left
				cursor.x = max(0, cursor.x - 1);
				break;
			case(106):	//down
				cursor.y = min(ROWS - 1, cursor.y + 1);
				break;
			case(107):	//up
				cursor.y = max(0, cursor.y - 1);
				break;
			default:
				break;
		}
	}
	getch();
	free_mem;
	endwin();

	return 0;
}

void reveal_surrounding(block ***blocks, int x, int y) {
	int j = 0;
	int k = 0;

	(*blocks)[y][x].revealed = 1;

	/* Iterate through a 3x3 box around the node, checking for zeros. */
	for (j = max(0,x-1); j <= min(x+1, g.columns-1); j++) {
		for (k = max(0,y-1); k <= min(y+1, g.rows-1); k++) {
			if (g.array[k][j] != 0 && !(*blocks)[k][j].revealed) {
				(*blocks)[k][j].revealed = 1;
			} else if (g.array[k][j] == 0 && !(*blocks)[k][j].revealed) {
				reveal_surrounding(blocks, j, k);
			}
		}
	}
}

void allocate_arrays() {
	int i;
	g.array = (int**) malloc(g.rows * sizeof(int*));
	for (i = 0; i < g.rows; i++) {
		g.array[i] = (int*) malloc(g.columns * sizeof(int));
	}

	g.minelist = (int**) malloc(g.num_mines * sizeof(int*));
	for (i = 0; i < g.num_mines; i++) {
		g.minelist[i] = (int*) malloc(2 * sizeof(int));
	}
}

void generate_mines() {
	int current = 0;
	int dupe;
	int i;
	int x, y;
	while (current < g.num_mines) {
		dupe = 0;
		x = rand() % g.columns;
		y = rand() % g.rows;
		for (i = 0; i < current; i++) {
			if (x == g.minelist[i][0] && y == g.minelist[i][1]) {
				dupe = 1;
			}
		}
		if (dupe == 0) {
			g.minelist[current][0] = x;
			g.minelist[current][1] = y;
			current++;
		}
	}
}

void fill_board() {
	int i, j, k, x, y;
	for (i = 0; i < g.num_mines; i++) {
		x = g.minelist[i][0];
		y = g.minelist[i][1];
		g.array[x][y] = 9;
		/* Iterate through a 3x3 box around each mine, incrementing each num */
		for (j = max(0,x-1); j <= min(x+1, g.columns-1); j++) {
			for (k = max(0,y-1); k <= min(y+1, g.rows-1); k++) {
				if (g.array[j][k] != 9) {
					g.array[j][k]++;
				}
			}
		}
	}
}



void free_mem() {
	int i;
	for (i = 0; i < g.rows; i++) {
		free(g.array[i]);
		free(g.minelist[i]);
	}
}

void explode(block **blocks) {
	while (1) {
		usleep(1000);	
		update_blocks(blocks);
		erase();
		draw_blocks(blocks);
		refresh();
	}
}

void init_blocks(block ***blocks) {
	int i, j;
	int xpos = (COLS / 2) - COLUMNS;
	int ypos = (LINES / 2) - ROWS;
	char s[2];
	int bombX, bombY;


	*blocks = malloc(sizeof(block *) * COLUMNS);
	for (i = 0; i < COLUMNS; i++)
		(*blocks)[i] = malloc(sizeof(block) * ROWS);

	for (i = 0; i < COLUMNS; i++) {
		for (j = 0;	j < ROWS; j++) {
			(*blocks)[i][j].x = xpos;
			(*blocks)[i][j].y = ypos;
			(*blocks)[i][j].revealed = 0;
			(*blocks)[i][j].color = 7;
			sprintf(s, "%d", g.array[i][j]);
			strcpy((*blocks)[i][j].val, s);
			xpos += 2;
		}
		ypos += 2;
		xpos = (COLS / 2) - COLUMNS;
	}

	init_pair(1, COLOR_BLACK, COLOR_RED);
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_BLACK, COLOR_YELLOW);
	init_pair(4, COLOR_BLACK, COLOR_BLUE);
	init_pair(5, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(6, COLOR_BLACK, COLOR_CYAN);
	init_pair(7, COLOR_BLACK, COLOR_WHITE);
}

void prep_explosion(block ***blocks, int bombX, int bombY) {
	int i, j;
	double distance, spread;
	for (i = 0; i < COLUMNS; i++) {
		for (j = 0;	j < ROWS; j++) {
			distance = sqrt((i - bombY)*(i - bombY) + (j - bombX)*(j - bombX));
			spread = (((double) random() / RAND_MAX) / 32 - .015625);
			(*blocks)[i][j].xVeloc = 1/(distance*3) * (j - bombX) + spread;
			(*blocks)[i][j].yVeloc = 1/(distance*3) * (i - bombY) + spread;
			(*blocks)[i][j].inertia =  1000.0 - 3 * distance;
			if (g.array[i][j] == 9) {
				(*blocks)[i][j].color = 1;
			} else {
				(*blocks)[i][j].color = 7;
			}
		}
	}
}

void draw_blocks(block **blocks) {
	int i, j;

	for (i = 0; i < COLUMNS; i++) {
		for (j = 0; j < ROWS; j++) {
			attron(COLOR_PAIR(blocks[i][j].color) | A_BOLD);
			if(blocks[i][j].revealed == 1) {
				mvprintw((int)blocks[i][j].y, (int)blocks[i][j].x, blocks[i][j].val);
			} else {
				mvprintw((int)blocks[i][j].y, (int)blocks[i][j].x, " ");
			}
			attroff(COLOR_PAIR(blocks[i][j].color) | A_BOLD);
		}
	}
}

void draw_cursor(block cursor) {
	int i, j;
	int xpos = (COLS / 2) - COLUMNS;
	int ypos = (LINES / 2) - ROWS;
	attron(COLOR_PAIR(cursor.color) | A_BOLD);
	mvprintw(ypos + 2 * cursor.y , xpos + 2 * cursor.x, cursor.val);
	attroff(COLOR_PAIR(cursor.color) | A_BOLD);
}

void update_blocks(block **blocks) {
	int i, j;
	double newX, newY;

	for (i = 0; i < COLUMNS; i++) {	
		for (j = 0; j < ROWS; j++) {
			if (blocks[i][j].inertia != 1000.0) {
				blocks[i][j].x += (blocks[i][j].xVeloc);
				blocks[i][j].y += (blocks[i][j].yVeloc);
				blocks[i][j].xVeloc *= blocks[i][j].inertia/1000.0;
				blocks[i][j].yVeloc *= blocks[i][j].inertia/1000.0;
			}
			if ((blocks[i][j].x <= 0) || (blocks[i][j].x >= COLS))
				blocks[i][j].xVeloc = -blocks[i][j].xVeloc;
			if ((blocks[i][j].y <= 0) || (blocks[i][j].y >= LINES))
				blocks[i][j].yVeloc = -blocks[i][j].yVeloc;
		}
	}
}
