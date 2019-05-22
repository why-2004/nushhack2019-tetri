#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <assert.h>
#include <time.h>
#include <citro2d.h>//2d graphix

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

#define NUM_TETROMINOS 29
#define NUM_ORIENTATIONS 4
#define TETRA 4

#define MAX_LEVEL 20
#define LINES_PER_LEVEL 10

#define ADD_EMPTY(w) waddch((w), ' '); waddch((w), ' ')

int level;
int linesCleared;

typedef struct {
  int row;
  int col;
} tetris_location;

typedef struct {
  int typ;
  int ori;
  tetris_location loc;
} tetris_block;

typedef struct
{
	C2D_Sprite spr;
	float dx, dy; // velocity
} Sprite;

typedef struct {
  /*
    Game board stuff:
   */
  int rows;
  int cols;
  char *board;
  /*
    Scoring information:
   */
  int points;
  int level;
  /*
    Falling block is the one currently going down.  Next block is the one that
    will be falling after this one.  Stored is the block that you can swap out.
   */
  tetris_block falling;
  tetris_block next;
  tetris_block stored;
  /*
    Number of game ticks until the block will move down.
   */
  int ticks_till_gravity;
  /*
    Number of lines until you advance to the next level.
   */
  int lines_remaining;
} tetris_game;

void tg_init(tetris_game *obj, int rows, int cols);
tetris_game *tg_create(int rows, int cols);
void tg_destroy(tetris_game *obj);
void tg_delete(tetris_game *obj);
tetris_game *tg_load(FILE *f);
void tg_save(tetris_game *obj, FILE *f);


typedef enum {
  TM_LEFT, TM_RIGHT, TM_CLOCK, TM_COUNTER, TM_SOFT, TM_HOLD, TM_NONE, TM_HARD
} tetris_move;

typedef struct {
  /*
    Game board stuff:
   */
  int rows;
  int cols;
  char *board;
  /*
    Scoring information:
   */
  int points;
  int level;
  /*
    Falling block is the one currently going down.  Next block is the one that
    will be falling after this one.  Stored is the block that you can swap out.
   */
  tetris_block falling;
  tetris_block next;
  tetris_block stored;
  /*
    Number of game ticks until the block will move down.
   */
  int ticks_till_gravity;
  /*
    Number of lines until you advance to the next level.
   */
  int lines_remaining;
} tetris_game;


tetris_location TETROMINOS[NUM_TETROMINOS][NUM_ORIENTATIONS][TETRA] = {
		// I0
		{{{1, 0}, {1, 1}, {1, 2}, {1, 3}},
		{{0, 2}, {1, 2}, {2, 2}, {3, 2}},
		{{3, 0}, {3, 1}, {3, 2}, {3, 3}},
		{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
		// L1
		{{{0, 2}, {1, 0}, {1, 1}, {1, 2}},
		{{0, 1}, {1, 1}, {2, 1}, {2, 2}},
		{{1, 0}, {1, 1}, {1, 2}, {2, 0}},
		{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
		// J2
		{{{0, 0}, {1, 0}, {1, 1}, {1, 2}},
		{{0, 1}, {0, 2}, {1, 1}, {2, 1}},
		{{1, 0}, {1, 1}, {1, 2}, {2, 2}},
		{{0, 1}, {1, 1}, {2, 0}, {2, 1}}},
		// O3
		{{{0, 1}, {0, 2}, {1, 1}, {1, 2}},
		{{0, 1}, {0, 2}, {1, 1}, {1, 2}},
		{{0, 1}, {0, 2}, {1, 1}, {1, 2}},
		{{0, 1}, {0, 2}, {1, 1}, {1, 2}}},
		// S4
		{{{0, 1}, {0, 2}, {1, 0}, {1, 1}},
		{{0, 1}, {1, 1}, {1, 2}, {2, 2}},
		{{1, 1}, {1, 2}, {2, 0}, {2, 1}},
		{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
		// Z5
		{{{0, 0}, {0, 1}, {1, 1}, {1, 2}},
		{{0, 2}, {1, 1}, {1, 2}, {2, 1}},
		{{1, 0}, {1, 1}, {2, 1}, {2, 2}},
		{{0, 1}, {1, 0}, {1, 1}, {2, 0}}},
		// T6
		{{{0, 1}, {1, 0}, {1, 1}, {1, 2}},
		{{0, 1}, {1, 1}, {1, 2}, {2, 1}},
		{{1, 0}, {1, 1}, {1, 2}, {2, 1}},
		{{0, 1}, {1, 0}, {1, 1}, {2, 1}}},
		//special blocks ahead
		//7
		{{{0, 1}, {1, 1}, {2, 1}, {3, 2}},
		{{2, 0}, {1, 1}, {1, 2}, {1, 3}},
		{{0, 1}, {1, 2}, {2, 2}, {3, 2}},
		{{2, 0}, {2, 1}, {2, 2}, {1, 3}}},
		//8
		{{{0, 2}, {1, 1}, {2, 1}, {3, 1}},
		{{1, 0}, {1, 1}, {1, 2}, {2, 3}},
		{{0, 2}, {3, 1}, {1, 2}, {2, 2}},
		{{2, 2}, {1, 0}, {2, 3}, {2, 1}}},
		//9
		{{{0, 1}, {1, 1}, {2, 2}, {3, 2}},
		{{2, 0}, {2, 1}, {1, 2}, {1, 3}},
		{{0, 1}, {1, 1}, {2, 2}, {3, 2}},
		{{2, 0}, {2, 1}, {1, 2}, {1, 3}}},
        //10
        {{{0, 2}, {1, 2}, {2, 1}, {3, 1}},
		{{1, 0}, {1, 1}, {2, 2}, {2, 3}},
		{{3, 1}, {1, 2}, {0, 2}, {2, 1}},
		{{0, 1}, {1, 0}, {1, 1}, {2, 1}}},
        //11
        {{{0, 2}, {1, 1}, {2, 1}, {2, 2}},
		{{2, 2}, {1, 1}, {1, 0}, {2, 0}},
		{{2, 0}, {1, 1}, {0, 1}, {0, 0}},
		{{0, 0}, {1, 1}, {1, 2}, {0, 2}}},
        //12
        {{{2, 2}, {1, 1}, {0, 1}, {0, 2}},
		{{2, 0}, {1, 1}, {1, 2}, {2, 2}},
		{{0, 0}, {1, 1}, {2, 1}, {2, 0}},
		{{0, 2}, {1, 1}, {1, 0}, {0, 0}}},
        //13
        {{{0, 0}, {1, 1}, {2, 2}, {3, 3}},
		{{0, 3}, {1, 2}, {2, 1}, {3, 0}},
		{{0, 0}, {1, 1}, {2, 2}, {3, 3}},
		{{0, 3}, {1, 2}, {2, 1}, {3, 0}}},
        //14
        {{{0, 1}, {1, 2}, {2, 2}, {3, 3}},
		{{1, 3}, {2, 2}, {2, 1}, {3, 0}},
		{{0, 0}, {1, 1}, {2, 1}, {3, 2}},
		{{0, 3}, {1, 2}, {1, 1}, {2, 0}}},
        //15
        {{{1, 0}, {2, 1}, {2, 2}, {3, 3}},
		{{0, 2}, {1, 1}, {2, 1}, {3, 0}},
		{{0, 0}, {1, 1}, {1, 2}, {2, 3}},
		{{0, 3}, {1, 2}, {2, 2}, {3, 1}}},
        //16
        {{{0, 2}, {1, 1}, {2, 1}, {3, 2}},
		{{2, 3}, {1, 2}, {1, 1}, {2, 2}},
		{{0, 1}, {1, 2}, {2, 2}, {3, 1}},
		{{1, 0}, {2, 1}, {2, 2}, {1, 3}}},
        //17
        {{{0, 1}, {1, 0}, {2, 1}, {3, 2}},
		{{1, 3}, {0, 2}, {1, 1}, {2, 0}},
		{{3, 2}, {2, 3}, {1, 2}, {0, 1}},
		{{2, 0}, {3, 1}, {2, 2}, {1, 3}}},
        //18
        {{{0, 2}, {2, 0}, {1, 1}, {3, 1}},
		{{0, 1}, {1, 2}, {1, 0}, {2, 3}},
		{{0, 2}, {3, 1}, {1, 3}, {2, 2}},
		{{3, 2}, {2, 3}, {1, 0}, {2, 1}}},
        //19
        {{{0, 1}, {1, 2}, {2, 1}, {3, 2}},
		{{1, 3}, {2, 2}, {1, 1}, {2, 0}},
		{{0, 1}, {1, 2}, {2, 1}, {3, 2}},
		{{1, 0}, {2, 1}, {1, 2}, {0, 2}}},
        //20
        {{{0, 2}, {3, 1}, {1, 1}, {2, 2}},
		{{1, 0}, {1, 2}, {2, 1}, {2, 3}},
		{{0, 2}, {1, 1}, {2, 2}, {3, 1}},
		{{1, 0}, {2, 3}, {1, 2}, {2, 1}}},
        //21
        {{{0, 1}, {1, 0}, {2, 1}, {2, 2}},
		{{1, 2}, {0, 1}, {1, 0}, {2, 0}},
		{{2, 1}, {1, 2}, {0, 1}, {0, 0}},
		{{1, 0}, {2, 1}, {1, 2}, {0, 2}}},
        //22
        {{{0, 2}, {1, 0}, {0, 1}, {2, 1}},
		{{0, 1}, {1, 0}, {1, 2}, {2, 2}},
		{{0, 1}, {2, 1}, {1, 2}, {2, 0}},
		{{0, 0}, {1, 2}, {1, 0}, {2, 1}}},
        //23
        {{{0, 0}, {1, 1}, {2, 2}, {2, 0}},
		{{0, 2}, {1, 1}, {2, 0}, {0, 0}},
		{{2, 2}, {1, 1}, {0, 0}, {0, 2}},
		{{2, 2}, {1, 1}, {0, 2}, {2, 2}}},
        //24
        {{{0, 2}, {1, 1}, {3, 0}, {2, 0}},
		{{3, 3}, {1, 1}, {1, 0}, {2, 2}},
		{{0, 2}, {2, 1}, {1, 2}, {3, 0}},
		{{0, 0}, {2, 3}, {1, 1}, {2, 2}}},
        //25
        {{{0, 0}, {1, 0}, {2, 1}, {3, 2}},
		{{1, 3}, {1, 2}, {2, 1}, {3, 0}},
		{{3, 2}, {2, 2}, {1, 1}, {0, 0}},
		{{2, 0}, {2, 1}, {1, 2}, {0, 3}}},
        //26
        {{{0, 2}, {2, 2}, {1, 1}, {3, 2}},
		{{2, 3}, {1, 2}, {2, 1}, {2, 0}},
		{{3, 1}, {0, 1}, {1, 1}, {2, 2}},
		{{1, 0}, {1, 3}, {1, 2}, {2, 1}}},
        //27
        {{{0, 2}, {1, 2}, {2, 1}, {3, 2}},
		{{2, 3}, {2, 2}, {1, 1}, {2, 0}},
		{{0, 1}, {1, 2}, {2, 1}, {3, 1}},
		{{1, 0}, {1, 1}, {2, 2}, {1, 3}}},
        //28
        {{{0, 1}, {1, 0}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 0}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 0}, {1, 2}, {2, 1}},
        {{0, 1}, {1, 0}, {1, 2}, {2, 1}}},
		};
	
	
int GRAVITY_LEVEL[MAX_LEVEL+1] = {
// 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
  53, 49, 45, 41, 37, 33, 28, 22, 17, 11,
//10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
  10,  9,  8,  7,  6,  6,  5,  5,  4,  4,  3
};

/*
   Return the block at the given row and column.
 */
char tg_get(tetris_game *obj, int row, int column)
{
  return obj->board[obj->cols * row + column];
}

/*
  Set the block at the given row and column.
 */
static void tg_set(tetris_game *obj, int row, int column, char value)
{
  obj->board[obj->cols * row + column] = value;
}

bool tg_check(tetris_game *obj, int row, int col)
{
  return 0 <= row && row < obj->rows && 0 <= col && col < obj->cols;
}

static void tg_put(tetris_game *obj, tetris_block block)
{
  int i;
  for (i = 0; i < TETRA; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    tg_set(obj, block.loc.row + cell.row, block.loc.col + cell.col,
           TYPE_TO_CELL(block.typ));
  }
}


static void tg_remove(tetris_game *obj, tetris_block block)
{
  int i;
  for (i = 0; i < TETRA; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    tg_set(obj, block.loc.row + cell.row, block.loc.col + cell.col, TC_EMPTY);
  }
}


static bool tg_fits(tetris_game *obj, tetris_block block)
{
  int i, r, c;
  for (i = 0; i < TETRA; i++) {
    tetris_location cell = TETROMINOS[block.typ][block.ori][i];
    r = block.loc.row + cell.row;
    c = block.loc.col + cell.col;
    if (!tg_check(obj, r, c) || TC_IS_FILLED(tg_get(obj, r, c))) {
      return false;
    }
  }
  return true;
}


static int random_tetromino(void) {
  return rand() % NUM_TETROMINOS;
}

static void tg_new_falling(tetris_game *obj)
{
  // Put in a new falling tetromino.
  obj->falling = obj->next;
  obj->next.typ = random_tetromino();
  obj->next.ori = 0;
  obj->next.loc.row = 0;
  obj->next.loc.col = obj->cols/2 - 2;
}

static void tg_down(tetris_game *obj)
{
  tg_remove(obj, obj->falling);
  while (tg_fits(obj, obj->falling)) {
    obj->falling.loc.row++;
  }
  obj->falling.loc.row--;
  tg_put(obj, obj->falling);
  tg_new_falling(obj);
}
static void tg_rotate(tetris_game *obj, int direction)
{
  tg_remove(obj, obj->falling);

  while (true) {
    obj->falling.ori = (obj->falling.ori + direction) % NUM_ORIENTATIONS;

    // If the new orientation fits, we're done.
    if (tg_fits(obj, obj->falling))
      break;

    // Otherwise, try moving left to make it fit.
    obj->falling.loc.col--;
    if (tg_fits(obj, obj->falling))
      break;

    // Finally, try moving right to make it fit.
    obj->falling.loc.col += 2;
    if (tg_fits(obj, obj->falling))
      break;

    // Put it back in its original location and try the next orientation.
    obj->falling.loc.col--;
    // Worst case, we come back to the original orientation and it fits, so this
    // loop will terminate.
  }

  tg_put(obj, obj->falling);
}

static void tg_hold(tetris_game *obj)
{
  tg_remove(obj, obj->falling);
  if (obj->stored.typ == -1) {
    obj->stored = obj->falling;
    tg_new_falling(obj);
  } else {
    int typ = obj->falling.typ, ori = obj->falling.ori;
    obj->falling.typ = obj->stored.typ;
    obj->falling.ori = obj->stored.ori;
    obj->stored.typ = typ;
    obj->stored.ori = ori;
    while (!tg_fits(obj, obj->falling)) {
      obj->falling.loc.row--;
    }
  }
  tg_put(obj, obj->falling);
}

static void tg_handle_move(tetris_game *obj, tetris_move move)
{
  switch (move) {
  case TM_LEFT:
    tg_move(obj, -1);
    break;
  case TM_RIGHT:
    tg_move(obj, 1);
    break;
  case TM_DROP:
    tg_down(obj);
    break;
  case TM_CLOCK:
    tg_rotate(obj, 1);
    break;
  case TM_COUNTER:
    tg_rotate(obj, -1);
    break;
  case TM_HOLD:
    tg_hold(obj);
    break;
  default:
    // pass
    break;
  }
}

static bool tg_line_full(tetris_game *obj, int i)
{
  int j;
  for (j = 0; j < obj->cols; j++) {
    if (TC_IS_EMPTY(tg_get(obj, i, j)))
      return false;
  }
  return true;
}

static void tg_shift_lines(tetris_game *obj, int r)
{
  int i, j;
  for (i = r-1; i >= 0; i--) {
    for (j = 0; j < obj->cols; j++) {
      tg_set(obj, i+1, j, tg_get(obj, i, j));
      tg_set(obj, i, j, TC_EMPTY);
    }
  }
}


static int tg_check_lines(tetris_game *obj)
{
  int i, nlines = 0;
  tg_remove(obj, obj->falling); // don't want to mess up falling block

  for (i = obj->rows-1; i >= 0; i--) {
    if (tg_line_full(obj, i)) {
      tg_shift_lines(obj, i);
      i++; // do this line over again since they're shifted
      nlines++;
    }
  }

  tg_put(obj, obj->falling); // replace
  return nlines;
}

static bool tg_game_over(tetris_game *obj)
{
  int i, j;
  bool over = false;
  tg_remove(obj, obj->falling);
  for (i = 0; i < 2; i++) {
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        over = true;
      }
    }
  }
  tg_put(obj, obj->falling);
  return over;
}

bool tg_tick(tetris_game *obj, tetris_move move)
{
  int lines_cleared;
  // Handle gravity.
  tg_do_gravity_tick(obj);

  // Handle input.
  tg_handle_move(obj, move);

  // Check for cleared lines
  lines_cleared = tg_check_lines(obj);

  tg_adjust_score(obj, lines_cleared);

  // Return whether the game will continue (NOT whether it's over)
  return !tg_game_over(obj);
}


void tg_init(tetris_game *obj, int rows, int cols)
{
  // Initialization logic
  obj->rows = rows;
  obj->cols = cols;
  obj->board = malloc(rows * cols);
  memset(obj->board, TC_EMPTY, rows * cols);
  obj->points = 0;
  obj->level = 0;
  obj->ticks_till_gravity = GRAVITY_LEVEL[obj->level];
  obj->lines_remaining = LINES_PER_LEVEL;
  srand(time(NULL));
  tg_new_falling(obj);
  tg_new_falling(obj);
  obj->stored.typ = -1;
  obj->stored.ori = 0;
  obj->stored.loc.row = 0;
  obj->next.loc.col = obj->cols/2 - 2;
  printf("%d", obj->falling.loc.col);
}

tetris_game *tg_create(int rows, int cols)
{
  tetris_game *obj = malloc(sizeof(tetris_game));
  tg_init(obj, rows, cols);
  return obj;
}

void tg_destroy(tetris_game *obj)
{
  // Cleanup logic
  free(obj->board);
}

void tg_delete(tetris_game *obj) {
  tg_destroy(obj);
  free(obj);
}

static C2D_SpriteSheet spriteSheet;
static Sprite sprites[MAX_SPRITES];
static size_t numSprites = MAX_SPRITES/2;

//************************************************************************************************************************


void display_board(tetris_game *obj)
{
  int i, j,x,y;
  for (i = 0; i < obj->rows; i++) {
    for (j = 0; j < obj->cols; j++) {
      if (TC_IS_FILLED(tg_get(obj, i, j))) {
        Sprite* sprite = &sprites[i];
		C2D_SpriteFromSheet(&sprite->spr, spriteSheet, 8);
		C2D_SpriteSetPos(&sprite->spr, (i*12)+141, (j*12)+12);
      } else {
      }
    }
  }
  wnoutrefresh(w);
}
void display_piece( tetris_block block)
{
  int b;
  tetris_location c;
  if (block.typ == -1) {
    wnoutrefresh(w);
    return;
  }
  for (b = 0; b < TETRA; b++) {
    c = TETROMINOS[block.typ][block.ori][b];
	C2D_SpriteFromSheet(&sprite->spr, spriteSheet, 8);
	
    ADD_BLOCK(w, TYPE_TO_CELL(block.typ));
  }
  wnoutrefresh(w);
}

int main(int argc, char* argv[])
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);

	
	//************************************************************************************************
	
	//************************************************************************************************
	
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		C2D_DrawImage("UI.png",0,0);
		
		
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
	}

	gfxExit();
	return 0;
}
