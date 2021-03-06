#include "include/raylib.h"
#include "include/raymath.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define U_MIN(a,b)         (((a)<(b))?(a):(b))
#define U_MAX(a,b)         (((a)>(b))?(a):(b))

#define SCREENWIDTH 800
#define SCREENHEIGHT 700

#define DEFAULT_FONT_SIZE 60
#define MAX_NAME_LENGTH 10

enum states{ACTIVE, GAMEOVER, YOUWIN, PAUSED, MENU, HIGHSCORE, CREDITS};

typedef struct Pair{
	int index;
		
	float time;
} pair;
typedef struct Players{
	char name[MAX_NAME_LENGTH];
	int score;
} players;

typedef struct BatStruct{
	Rectangle rect;
	Color colour;
	Vector2 velocity;
} batstruct;

typedef struct BlockStruct{
	Rectangle rect;
	Color colour;
	bool active;
	int hitstokill;
	int reward;
} blockstruct;

typedef struct BallStruct{
	Rectangle rect;
	Color colour;
	Vector2 velocity;
	Rectangle contact[4];
	bool active;
	int lives;
} ballstruct;

typedef struct Dimensions
{
    // real screen coordinates
    float width;
    float height;

    // virtual screen (internal resolution of the game assets)
    float virtual_width;
    float virtual_height;

    // scale used for graphics
    float scale_x;
    float scale_y;

    //default font size (scaled)
    float font_size;
} dimensions;

bool RayVsRect(Vector2, Vector2, Rectangle, Vector2 *, Vector2 *, float *);
bool DynamicRectVsRect(Rectangle, Rectangle, Vector2, Vector2 *, Vector2 *, float *, float);
bool ResolveDynamicRectVsRect(ballstruct *, const float, blockstruct *, float);
bool RectVsRectAABB(Rectangle, Rectangle);

//Various helper functions
static void swap(void *, void *, size_t);
static int compare(const void *, const void *);
static char *readfile(const char *, int *, int *);
static void findfileDim(FILE *, int *, int *);
static float mapf(float, float, float, float, float);
static players* GetSortedScores(const char *);
static void addScore(const char *, char *, int); 

//Declaration of the game structs
static blockstruct *block;
static ballstruct ball;
static batstruct bat;
static Rectangle bat_bounds;
static dimensions screen;
static players player; 

//Declaration of game state enum
static enum states GAME_STATE;

//Declaration of Texture data
static Texture2D menubuttons[4];
static Texture2D spacebarlogo;
static Texture2D enterbuttonlogo;

//Declaration of variables needed in multiple functions
static int MAXBLOCKSX;
static int MAXBLOCKSY;
static float min_velocity;
static float max_velocity;
static float sling_force;
static int framesCounter;
static float prev_instance_time;
static float round_time;
static int max_time;
static int max_active_blocks;
static int active_blocks_count;
static int letterCount;

//Declaration of sfx
static Sound sfxGameOver;
static Sound sfxButton;
static Sound sfxWallCollision;

//Declaration of game functions
//static void IntroScreen(void);
static void MainMenuState(void);
static void GameOverState(void);
static void WinState(void);
static void PauseState(void);
static void CreditsState(void);
static void HighScoreState(void);
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);


int main()
{
	InitWindow(SCREENWIDTH, SCREENHEIGHT, "Breakout!");
	InitAudioDevice();
	InitGame();
	
	SetTargetFPS(60);
	while(!WindowShouldClose())
	{
		switch(GAME_STATE)
		{
			case ACTIVE:
				UpdateGame();
				DrawGame();
				break;
			case PAUSED:
				PauseState();
				break;
			case GAMEOVER:
				GameOverState();
				break;
			case MENU:
				MainMenuState();
				break;
			case CREDITS:
				CreditsState();
				break;
			case YOUWIN:
				WinState();
				break;
			case HIGHSCORE:
				HighScoreState();
				break;
		}
	}
	UnloadGame();
	CloseAudioDevice();
	CloseWindow();
	
	return 0;
}

void MainMenuState(void)
{
	BeginDrawing();
	ClearBackground(RAYWHITE);
	int count;
	const char * BreakoutChar = TextFormat("Breakout");
	int BreakoutSize = MeasureText(BreakoutChar, 60);
	DrawText(BreakoutChar, ((screen.virtual_width) / 2 - BreakoutSize) * screen.scale_x, ((screen.virtual_height)/ 2 - BreakoutSize) * screen.scale_y, 60, BLACK);
	for(count = 0; count < 4; count++)
	{
		Vector2 button_pos = (Vector2){(screen.virtual_width / 2 - menubuttons[count].width) * screen.scale_x - 20.0f, (((screen.virtual_height / 2 - menubuttons[count].height) / 2) + count * 150) * screen.scale_y + BreakoutSize  / 2};
		Rectangle button_bounds = {button_pos.x, button_pos.y, menubuttons[count].width * (screen.scale_x + 1.0f), menubuttons[count].height * (screen.scale_x + 1.0f)};
		if(CheckCollisionPointRec(GetMousePosition(), button_bounds))
		{
			button_pos.x -= 10 * screen.scale_x;
			DrawTextureEx(menubuttons[count], button_pos, 0.0f, screen.scale_x + 1.2f, WHITE);
			if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
			{
				int start_time = 0;
				if(!IsSoundPlaying(sfxButton))
				{
					start_time = GetTime();
					PlaySoundMulti(sfxButton);
				}
				switch(count)
				{
					case 0:
						prev_instance_time = GetTime();
						GAME_STATE = ACTIVE;
						break;
					case 1:
						GAME_STATE = CREDITS;
						break;
					case 2:
						GAME_STATE = HIGHSCORE;
						break;
					case 3:
						while(GetTime() - start_time < 0.3f){};
	
						UnloadGame();
						CloseWindow();
						
						break;
						
				}
				 
			}
		} else {
			DrawTextureEx(menubuttons[count], button_pos, 0.0f, screen.scale_x + 1.1f, WHITE);
		}
		
	}
	EndDrawing();
}
void GameOverState(void)
{
	Vector2 spacebarposition = (Vector2){(screen.virtual_width / 2 - spacebarlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 150};
	Vector2 enterbuttonposition = (Vector2){(screen.virtual_width / 2 - enterbuttonlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 200};
	const char *scoreText = TextFormat("Score: %d", player.score);
	
	Rectangle textBox = {spacebarposition.x + 10, spacebarposition.y - 200, 275, 50};
	bool mouseOnText = false;
	
	if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
	else mouseOnText = false;
	
	if (mouseOnText)
	{
		SetMouseCursor(MOUSE_CURSOR_IBEAM);
		int key = GetCharPressed();
		
		while (key > 0)
		{
			
			if ((key > 32) && (key <= 125) && (letterCount < MAX_NAME_LENGTH))
			{
				player.name[letterCount] = (char)key;
				letterCount++;
			}

			key = GetCharPressed();
		}

		if (IsKeyPressed(KEY_BACKSPACE))
		{
			letterCount--;
			if (letterCount < 0) letterCount = 0;
			player.name[letterCount] = '\0';
		}

		if (IsKeyPressed(KEY_ENTER) && letterCount > 1)
		{
			addScore("scores.txt", player.name, player.score);
			printf("%s", player.name);
			GAME_STATE = HIGHSCORE;
		}
	}
	else SetMouseCursor(MOUSE_CURSOR_DEFAULT);
	
	if(IsKeyPressed(KEY_SPACE))
	{
		StopSoundMulti();
		UnloadGame();
		InitGame();
		GAME_STATE = ACTIVE;
	} else if (IsKeyPressed(KEY_ENTER))
	{
		StopSoundMulti();
		UnloadGame();
		InitGame();
		GAME_STATE = MENU;
	}
	
	BeginDrawing();
	ClearBackground(RAYWHITE);
	DrawText("GAME OVER", (screen.virtual_width / 2 - MeasureText("GAME OVER", 60)) * screen.scale_x - 30.0f, (screen.virtual_height / 2 - MeasureText("GAME OVER", 60)) * screen.scale_y, 60, MAROON);
	DrawText(scoreText, (screen.virtual_width / 2 - MeasureText(scoreText, 30)) * screen.scale_x - 30.0f, (screen.virtual_height / 2) * screen.scale_y + 50, 30, GOLD);
	DrawTextureEx(spacebarlogo, spacebarposition, 0.0f, screen.scale_x + 0.5f, WHITE);
	DrawText("Play again.", spacebarposition.x + spacebarlogo.width + 50.0f, spacebarposition.y, 30, LIGHTGRAY);
	DrawTextureEx(enterbuttonlogo, enterbuttonposition, 0.0f, screen.scale_x + 0.5f, WHITE);
	DrawText("Main menu.", spacebarposition.x + spacebarlogo.width + 50.0f , enterbuttonposition.y, 30, LIGHTGRAY);
	
	
	DrawText("Enter Your Name", textBox.x + textBox.width / 2 - 100, textBox.y - 25, 20, BLACK);
	//DrawText("No Spaces Allowed", textBox.x, textBox.y + 50, 10, ORANGE);
	
	DrawRectangleRec(textBox,WHITE);
	if (mouseOnText) DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, RED);
	else DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, DARKGRAY);

	DrawText(player.name, textBox.x + 5, textBox.y + 8, 40, MAROON);

	if(letterCount < MAX_NAME_LENGTH) 
	{
		DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_NAME_LENGTH), textBox.x, textBox.y + textBox.height + 10, 15, LIME);
	} else 
	{
		DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_NAME_LENGTH), textBox.x, textBox.y + textBox.height + 10, 15, RED);
	}

	if (mouseOnText) DrawText(TextFormat("Press Enter To Save"), textBox.x + 50, textBox.y + textBox.height + 100, 15, BLACK);

	if (mouseOnText)
	{
		if (letterCount < MAX_NAME_LENGTH)
		{

			if (((framesCounter/20)%2) == 0) DrawText("_", textBox.x + 8 + MeasureText(player.name, 40), textBox.y + 12, 40, MAROON);
		}
		else DrawText("Press BACKSPACE to delete chars...", 290, 430, 17, RED);
	}



	EndDrawing();
}
void WinState(void)
{
	Vector2 spacebarposition = (Vector2){(screen.virtual_width / 2 - spacebarlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 150};
	Vector2 enterbuttonposition = (Vector2){(screen.virtual_width / 2 - enterbuttonlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 200};
	const char *scoreText = FormatText("Score: %d", player.score);
	
	Rectangle textBox = {spacebarposition.x + 10, spacebarposition.y - 200, 275, 50};
	bool mouseOnText = false;
	
	if (CheckCollisionPointRec(GetMousePosition(), textBox)) mouseOnText = true;
	else mouseOnText = false;
	
	if (mouseOnText)
	{
		SetMouseCursor(MOUSE_CURSOR_IBEAM);
		int key = GetCharPressed();
		while (key > 0)
		{
			
			if ((key > 32) && (key <= 125) && (letterCount < MAX_NAME_LENGTH))
			{
				player.name[letterCount] = (char)key;
				letterCount++;
			}

			key = GetCharPressed();
		}

		if (IsKeyPressed(KEY_BACKSPACE))
		{
			letterCount--;
			if (letterCount < 0) letterCount = 0;
			player.name[letterCount] = '\0';
		}

		if (IsKeyPressed(KEY_ENTER) && letterCount > 1)
		{
			addScore("scores.txt", player.name, player.score);
			printf("%s", player.name);
			GAME_STATE = HIGHSCORE;
		}
	}
	else SetMouseCursor(MOUSE_CURSOR_DEFAULT);
	
	if(IsKeyPressed(KEY_SPACE))
	{
		StopSoundMulti();
		UnloadGame();
		InitGame();
		GAME_STATE = ACTIVE;
	} else if (IsKeyPressed(KEY_ENTER))
	{
		StopSoundMulti();
		UnloadGame();
		InitGame();
		GAME_STATE = MENU;
	}
	BeginDrawing();
	ClearBackground(RAYWHITE);
	DrawText("YOU WIN!", (screen.virtual_width / 2 - MeasureText("YOU WIN!", 60)) * screen.scale_x - 30.0f, (screen.virtual_height / 2 - MeasureText("YOU WIN!", 60)) * screen.scale_y, 60, GOLD);
	DrawText(scoreText, (screen.virtual_width / 2 - MeasureText(scoreText, 30)) * screen.scale_x - 30.0f, (screen.virtual_height / 2) * screen.scale_y + 50, 30, GOLD);
	DrawTextureEx(spacebarlogo, spacebarposition, 0.0f, screen.scale_x + 0.5f, WHITE);
	DrawText("Play again.", spacebarposition.x + spacebarlogo.width + 50.0f, spacebarposition.y, 30, LIGHTGRAY);
	DrawTextureEx(enterbuttonlogo, enterbuttonposition, 0.0f, screen.scale_x + 0.5f, WHITE);
	DrawText("Main menu.", spacebarposition.x + spacebarlogo.width + 50.0f , enterbuttonposition.y, 30, LIGHTGRAY);
	
	DrawText("Enter Your Name", textBox.x + textBox.width / 2 - 100, textBox.y - 25, 20, BLACK);
	//DrawText("No Spaces Allowed", textBox.x, textBox.y + 50, 10, ORANGE);
	
	DrawRectangleRec(textBox,WHITE);
	if (mouseOnText) DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, RED);
	else DrawRectangleLines(textBox.x, textBox.y, textBox.width, textBox.height, DARKGRAY);

	DrawText(player.name, textBox.x + 5, textBox.y + 8, 40, MAROON);

	if(letterCount < MAX_NAME_LENGTH) 
	{
		DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_NAME_LENGTH), textBox.x, textBox.y + textBox.height + 10, 15, LIME);
	} else 
	{
		DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_NAME_LENGTH), textBox.x, textBox.y + textBox.height + 10, 15, RED);
	}

	if (mouseOnText) DrawText(TextFormat("Press Enter To Save"), textBox.x + 50, textBox.y + textBox.height + 100, 15, BLACK);

	if (mouseOnText)
	{
		if (letterCount < MAX_NAME_LENGTH)
		{

			if (((framesCounter/20)%2) == 0) DrawText("_", textBox.x + 8 + MeasureText(player.name, 40), textBox.y + 12, 40, MAROON);
		}
		else DrawText("Press BACKSPACE to delete chars...", 290, 430, 17, RED);
	}
	
	EndDrawing();
}

void PauseState(void)
{
	if(IsKeyPressed('P')) GAME_STATE = ACTIVE;
	framesCounter++;
	BeginDrawing();
	//ClearBackground(RAYWHITE);
	if ((framesCounter / 30) % 2) DrawText("PAUSED", (screen.width - MeasureText("PAUSED", 30)) / 2, (screen.height - MeasureText("GAME OVER", 30))/ 2, 30, BLACK);
	EndDrawing();
}

void CreditsState(void) {
    char *story[13];

    story[0]="Marmara Universitesi";
    story[1]="Teknoloji Fakultesi";
    story[2]="Bilgisayar Muhendisligi Bolumu";
    story[3]="Donem Projesi";
    story[4]="Ali Asghar, Ali Askaroglu, Esameldin Mahdi, Ibrahim Kedir";

    int story_time=0;
    int story_text=0;


	while (story_text < 5)    
    {
     
        BeginDrawing();
        ClearBackground(WHITE);

 
        story_time++;

        if(story_time==180 && story_text < 5)   {   story_text++;  story_time=0; }

            Color faded_color1=Fade(BLACK,(float)story_time/360.0);
            Color faded_color2=Fade(WHITE,(float)story_time/360.0);

			DrawText(story[story_text], 
                            screen.width/2 + 5 * screen.scale_x - MeasureText(story[story_text], screen.font_size) / 2,
                            screen.height/2 , 
                            screen.font_size, 
                            faded_color2);

            DrawText(story[story_text], 
                            screen.width/2 - MeasureText(story[story_text], screen.font_size) / 2,
                            screen.height/2 , 
                            screen.font_size, 
                            faded_color1);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    GAME_STATE = MENU;

}
void HighScoreState(void)
{
	Vector2 spacebarposition = (Vector2){(screen.virtual_width / 2 - spacebarlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 150};
	Vector2 enterbuttonposition = (Vector2){(screen.virtual_width / 2 - enterbuttonlogo.width) * screen.scale_x - 100.0f, (screen.virtual_height / 2) * screen.scale_y + 200};
	//const char *scoreText = TextFormat("Score: %d", player.score);
	

	if (IsKeyPressed(KEY_ENTER))
	{
		StopSoundMulti();
		UnloadGame();
		InitGame();
		GAME_STATE = MENU;
	}
	players *highscores = NULL;
    int count;
    highscores = GetSortedScores("scores.txt");
    BeginDrawing();
    
    ClearBackground(RAYWHITE);
    

	DrawText("High Scores", 250, 90, 50, DARKBLUE);

	DrawLine(100, 300, 650, 300, DARKBLUE);
	
	DrawText(TextFormat("#Number %40s","Score"), 100, 280, 20, DARKBLUE);

	for(count = 0; count < 5; count++) {
		
		DrawText(TextFormat("%d %s", count + 1, highscores[count].name), 100, 350 + 25 * count, 20, DARKBLUE);
		DrawText(TextFormat("%d", highscores[count].score), 500, 350 + 25 * count, 20, DARKBLUE);
	}
	
	DrawTextureEx(enterbuttonlogo, enterbuttonposition, 0.0f, screen.scale_x + 0.5f, WHITE);
	DrawText("Main menu.", spacebarposition.x + spacebarlogo.width + 50.0f , enterbuttonposition.y, 30, LIGHTGRAY);
    EndDrawing();
}
void InitGame(void)
{
	GAME_STATE = MENU;
	//Load Sfx
	screen.width = SCREENWIDTH;
	screen.height = SCREENHEIGHT;
	screen.virtual_width = 1920;
	screen.virtual_height = 1080;
	
	// calculate the scale
    screen.scale_x = screen.width / screen.virtual_width;
    screen.scale_y = screen.height / screen.virtual_height;
    
    // default font sized (once scaled)
    screen.font_size = DEFAULT_FONT_SIZE * screen.scale_x;
    
	ball.velocity = Vector2Zero();
	ball.rect = (Rectangle){screen.width / 2.0f, screen.height / 2.0f + 150.0f, 16, 16};
	ball.colour = BLUE;
	ball.active = false;
	ball.lives = 5;
	//SetMasterVolume(0.5f);
	
	const char mapdir[] = "map/2.txt"; 
	char * cShape;
	if((cShape = readfile(mapdir, &MAXBLOCKSX, &MAXBLOCKSY)) == NULL) printf("Error. Could not load the map.\n");
	block = (blockstruct *)calloc(MAXBLOCKSX * MAXBLOCKSY, sizeof(blockstruct));
	max_active_blocks = 0;
	int countX, countY;
	int arrindex = 0;
	for(countY = 0; countY < MAXBLOCKSY; countY++)
	{
		for(countX = 0; countX < MAXBLOCKSX; countX++)
		{
			arrindex = countY  * MAXBLOCKSX + countX;
			switch(cShape[arrindex])
			{
				case '*':
					block[arrindex].reward = 100;     
					block[arrindex].active = true;
					block[arrindex].hitstokill = 2;
					block[arrindex].colour = RED;
					break;
				case '/':
					block[arrindex].reward = 300;
					block[arrindex].active = true;
					block[arrindex].hitstokill = 3;
					block[arrindex].colour = BLUE;
					break;
				case '+':
					block[arrindex].reward = 500;
					block[arrindex].active = true;
					block[arrindex].hitstokill = 4;
					block[arrindex].colour = PURPLE;
					break;
				case '$':
					block[arrindex].reward = 1000;
					block[arrindex].active = true;
					block[arrindex].hitstokill = 5;
					block[arrindex].colour = LIME;
					break;
					
				default:
					block[arrindex].active = false;
			}
			if(block[arrindex].active)
			{
				++max_active_blocks;
				block[arrindex].rect.width = screen.width / MAXBLOCKSX;
				block[arrindex].rect.height = (screen.height - 150.0f) / MAXBLOCKSY;
				block[arrindex].rect.x = block[arrindex].rect.width * countX;  
				block[arrindex].rect.y = block[arrindex].rect.height * countY;
			}
		}
	}
	active_blocks_count = max_active_blocks;
	bat.rect.height = 8.0f;
	bat.rect.width = 100.0f;
	bat_bounds = (Rectangle){bat.rect.width, screen.height - 125 + bat.rect.height, screen.width - bat.rect.width * 2, 125 - bat.rect.height * 2};
	bat.rect.x = (screen.width - bat.rect.width)/ 2;
	bat.rect.y = bat_bounds.y;
	bat.colour = YELLOW;
	
	player.score = 0;
	player.name[0] = '\0';
	letterCount = 0;
	
	min_velocity = 250.0f;
	max_velocity = min_velocity;
	
	prev_instance_time = GetTime();
	
	sling_force = 0;
	framesCounter = 0;
	max_time = 3 * 60; // The max time per round is set to 3 minutes
	//Loading button textures
	menubuttons[0] = LoadTexture("textures/playbutton.png");
    menubuttons[1] = LoadTexture("textures/creditsbutton.png");
    menubuttons[2] = LoadTexture("textures/scoresbutton.png");
    menubuttons[3] = LoadTexture("textures/exitbutton.png");
    spacebarlogo = LoadTexture("textures/spacebarlogo.png");
	enterbuttonlogo = LoadTexture("textures/enterbuttonlogo.png");
    
	sfxGameOver = LoadSound("sfx/gameoversfx.wav");         // Load WAV audio file
	sfxButton = LoadSound("sfx/buttonsfx.wav");
	sfxWallCollision = LoadSound("sfx/wallcollisionsfx.wav");
}
void UpdateGame(void)
{
	
	if(IsKeyPressed('P')) GAME_STATE = PAUSED;
	round_time = GetTime() - prev_instance_time;
	float time_step = GetFrameTime();
	bat.velocity = Vector2Zero();  //Initial Velocity
	if(IsMouseButtonDown(MOUSE_LEFT_BUTTON))
	{
		Vector2 ray_direction;
		Vector2 MousePos = GetMousePosition();
		ray_direction = (Vector2){MousePos.x - ball.rect.x, MousePos.y - ball.rect.y};
		ray_direction = Vector2Normalize(ray_direction);
		ball.velocity.x += ray_direction.x * time_step * 1000.0f;
		ball.velocity.y += ray_direction.y * time_step * 1000.0f;
	}
	
	
	if(IsKeyDown(KEY_RIGHT)) 
	{
		bat.velocity.x += 500.0f + (max_active_blocks - active_blocks_count) * 8.0f;
		bat.rect.x += bat.velocity.x * time_step;
		if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.x -= bat.velocity.x * time_step; 
	}
	
    if(IsKeyDown(KEY_LEFT)) 
	{
		bat.velocity.x += -500.0f - (max_active_blocks - active_blocks_count) * 8.0f;
		bat.rect.x += bat.velocity.x * time_step;
		if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.x -= bat.velocity.x * time_step;
	}
	if(IsKeyDown(KEY_DOWN)) 
	{
		bat.velocity.y = 500.0f;
		bat.rect.y += bat.velocity.y * time_step;
		if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.y -= bat.velocity.y * time_step; 
	}
	if(IsKeyUp(KEY_DOWN))
	{
		//This bit of code calculates the sling_force i.e the force with which the bat hits the ball after being pulled down
		if(bat.rect.y > bat_bounds.y)
		{
			sling_force = (bat.rect.y - bat_bounds.y) * 5;
			bat.rect.y -= sling_force * time_step;
		}
		//if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.y += bat.velocity.y;
	}
    if(IsKeyDown(KEY_UP)) 
	{
		bat.velocity.y = -500.0f;
		bat.rect.y += bat.velocity.y * time_step;
		if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.y -= bat.velocity.y * time_step;
	}
	
	if(!ball.active)
	{
		if(IsKeyPressed(KEY_SPACE))
		{
			ball.active = true;
			ball.velocity = (Vector2){0, 500.0f};
		}
	}
	
	
	if(RectVsRectAABB(bat.rect, ball.rect))
	{
		if(ball.velocity.y > 0)
		{
			//First we calculate the velocity of the ball for the y component
			ball.velocity.y += sling_force;
			ball.velocity.y *= -1;
			
			//Then we calculate the velocity for the x component
			ball.velocity.x += bat.velocity.x;	
		}
	}
	
	/*To make the game more fun we increase the min and max velocity values according to 
	the number of blocks broken by the player */
	min_velocity += (max_active_blocks - active_blocks_count) * 0.0025;
	max_velocity += (max_active_blocks - active_blocks_count) * 0.004;
	//This code stops the ball from slowing down too much
	if(fabs(ball.velocity.x) < min_velocity)
	{
		ball.velocity.x = (ball.velocity.x >= 0) ? min_velocity: -min_velocity;
	}
	
	if(fabs(ball.velocity.y) < min_velocity)
	{
		ball.velocity.y = (ball.velocity.y >= 0) ? min_velocity: -min_velocity;
	}
	
	//This code stops the ball from speeding up too much
	if(fabs(ball.velocity.x) > max_velocity)
	{
		ball.velocity.x = (ball.velocity.x >= 0) ? max_velocity: -max_velocity;
	}
	
	if(fabs(ball.velocity.y) > max_velocity)
	{
		ball.velocity.y = (ball.velocity.y >= 0) ? max_velocity: -max_velocity;
	}
	
	
	if (((ball.rect.x + ball.rect.width) >= screen.width) || ((ball.rect.x) <= 0)) 
	{
		SetSoundVolume(sfxWallCollision, mapf(ball.velocity.x, 250.0f, max_velocity + 50.0f, 0.075f, 0.1f));
		PlaySound(sfxWallCollision);
		ball.velocity.x *= -1.0f;
	}
    if ((ball.rect.y) <= 0)
	{
		SetSoundVolume(sfxWallCollision, mapf(ball.velocity.y, 250.0f, max_velocity + 50.0f, 0.05f, 0.1f));
		PlaySound(sfxWallCollision);
		ball.velocity.y *= -1.0f;
	}
    if ((ball.rect.y + ball.rect.height) >= screen.height)
    {
    	//If you lose
        ball.velocity = Vector2Zero();
        ball.active = false;

        --ball.lives; 
    }
    
    
    int countY, countX;
	Vector2 cn, cp;
	float ct;
	pair * collision_data = calloc(1, sizeof *collision_data);
	int arrindex = 0, collision_count = 0;
	for(countY = 0; countY < MAXBLOCKSY; countY++)
	{
		for(countX = 0; countX < MAXBLOCKSX; countX++)
		{
			arrindex = countY * MAXBLOCKSX + countX;
			if(block[arrindex].active && DynamicRectVsRect(ball.rect, block[arrindex].rect, ball.velocity, &cp, &cn, &ct, time_step))
			{
				//The collision_data struct records the details of all the collisions that occur
				collision_data[collision_count].index = arrindex;
				collision_data[collision_count].time = ct;
				
				++collision_count;
				if((collision_data = realloc(collision_data, (collision_count + 1) * sizeof *collision_data)) == NULL) printf("Error! Could not allocate the required memory. collsion_count = %d", collision_count);
				
				--block[arrindex].hitstokill;
				switch(block[arrindex].hitstokill)
				{
					case -1:
						--active_blocks_count;
						player.score += block[arrindex].reward;
						block[arrindex].active = false;
						break;
					case 1:
						block[arrindex].colour = GREEN;
						break;
					case 2:
						block[arrindex].colour = RED;
						break;
					case 3:
						block[arrindex].colour = BLUE;
						break;
					case 4:
						block[arrindex].colour = PURPLE;
						break;

				}
			}
			
		}
		
	}
	qsort(collision_data, collision_count, sizeof(pair), compare);
	int count;
	for(count = 0; count < collision_count; count++)
	{
		ResolveDynamicRectVsRect(&ball, time_step, &block[collision_data[count].index], 2.1f);	
	}
	
	free(collision_data);
	
	if(ball.active)
	{
		ball.rect.x += ball.velocity.x * time_step;
		ball.rect.y += ball.velocity.y * time_step;
	} else{
		ball.rect.x = (float)screen.width / 2;
		ball.rect.y = (float)screen.height / 2 + 150.0f;
	}
	
	//Various checks for the gameover state
	
	if(round_time > max_time) GAME_STATE = GAMEOVER;
	if(ball.lives <= 0) GAME_STATE = GAMEOVER;
    if(active_blocks_count == 0) 
	{
		GAME_STATE = YOUWIN;
	}
    if(GAME_STATE == GAMEOVER) PlaySoundMulti(sfxGameOver);
}

void DrawGame(void){
	
	int countY, countX;
	const char * scoreText = TextFormat("Score: %d", player.score);
	//const char * ballVelocity = TextFormat("X = %2f Y = %2f", ball.velocity.x, ball.velocity.y);
	//float timeLeft = (max_time - round_time) / 60;
	//const char * timeLeftText = TextFormat("Time Left: %.2f Blocksleft = %d", timeLeft, active_blocks_count);
	const char * ballLivesText = TextFormat("%d", ball.lives); //Assumes total number of lives are less than 10 
	BeginDrawing();
	ClearBackground(RAYWHITE);
	//DrawText(timeLeftText, screen.width - MeasureText(timeLeftText, 20) - 10, 10, 20, RED);
	//DrawText(ballVelocity, ball.rect.x + 10.0f, ball.rect.y, 10, GREEN);
	DrawText(ballLivesText, screen.width - 40, screen.height - 40, 40, RED);
	DrawCircle(ball.rect.x + ball.rect.width / 2 , ball.rect.y + ball.rect.height / 2, ball.rect.width / 2, ball.colour);
	
	int arrindex = 0;
	for(countY = 0; countY < MAXBLOCKSY; countY++)
	{
		for(countX = 0; countX < MAXBLOCKSX; countX++)
		{
			arrindex = countY * MAXBLOCKSX + countX;
			if(block[arrindex].active)
			{
				DrawRectangleRec(block[arrindex].rect, block[arrindex].colour);
			}
		}	
	}
	
	
	DrawRectangle(bat.rect.x, bat.rect.y, bat.rect.width, bat.rect.height, bat.colour);	
	
	DrawText(scoreText, 10, 10, 30, GOLD);
	EndDrawing();
}
void UnloadGame(void){
	//Unload button textures
	int count;
	for(count = 0; count < 4; count++) UnloadTexture(menubuttons[count]);
	UnloadTexture(spacebarlogo);
	UnloadTexture(enterbuttonlogo);
	UnloadSound(sfxGameOver);
	UnloadSound(sfxButton);
	UnloadSound(sfxWallCollision);
	free(block);
}

bool RayVsRect(Vector2 ray_origin, Vector2 ray_dir, Rectangle target, Vector2 * contact_point, Vector2 * contact_normal, float * t_hit_near)
{
	/* All the functions dealing with collision were learned from Javid on the OneLoneCoder youtube channel.
	The functions were converted from C++ to C by Ali Asghar*/
	
	(*contact_point) = Vector2Zero();
	(*contact_normal) = Vector2Zero();
	
	//Find the inverse of the direction vector
	Vector2 inv = Vector2Divide(Vector2One(), ray_dir);
	
	//Calculate the vwector positions of the near and far intersections of the ray with the rectangle
	Vector2 t_near = (Vector2){(target.x - ray_origin.x) * inv.x, (target.y - ray_origin.y) * inv.y};
	Vector2 t_far = (Vector2){(target.x + target.width - ray_origin.x) * inv.x, (target.y + target.height - ray_origin.y) * inv.y};
	
	if (isnan(t_far.y) || isnan(t_far.x)) return false;
	if (isnan(t_near.y) || isnan(t_near.x)) return false;
	
	if(t_near.x > t_far.x) swap(&t_near.x, &t_far.x, sizeof(float));
	if(t_near.y > t_far.y) swap(&t_near.y, &t_far.y, sizeof(float));
	
	if(t_near.x > t_far.y || t_near.y > t_far.x) return false;
	
	*t_hit_near = U_MAX(t_near.x, t_near.y);
	float t_hit_far = U_MIN(t_far.x, t_far.y);
	
	if(t_hit_far < 0) return false;
	
	*contact_point = (Vector2){ray_origin.x + (ray_dir.x * (*t_hit_near)), ray_origin.y + (ray_dir.y * (*t_hit_near))};
	
	
	//Calculate the contact normal
	if(t_near.x > t_near.y)
	{
		if(inv.x < 0)
		{
			*contact_normal = (Vector2){1, 0};
		} else{
			*contact_normal = (Vector2){-1, 0};
		}
		
	} else if(t_near.x < t_near.y)
	{
		if(inv.y < 0)
		{
			*contact_normal = (Vector2){0, 1};
		} else{
			*contact_normal = (Vector2){0, -1};
		}
	}
	
	return true;
}
bool DynamicRectVsRect(Rectangle r_dynamic, Rectangle r_static, Vector2 velocity, Vector2 * contact_point, Vector2 * contact_normal, float * contact_time, float frame_time)
{
	//We assume the two rectangles arent already in collision
	if(velocity.x == 0 && velocity.y == 0) return false;
	
	Rectangle expanded = (Rectangle){r_static.x - r_dynamic.width / 2, r_static.y - r_dynamic.height / 2, r_static.width + r_dynamic.width, r_static.height + r_dynamic.height};
	Vector2 origin = (Vector2){r_dynamic.x + r_dynamic.width / 2, r_dynamic.y + r_dynamic.height / 2};
	Vector2 direction = (Vector2){velocity.x * frame_time, velocity.y * frame_time};
	
	if(RayVsRect(origin, direction, expanded, contact_point, contact_normal, contact_time))
	{
		return ((*contact_time) >= 0.0f && (*contact_time) <= 1.0f);
	}else
	{
		return false;
	}
	
}

void swap(void * a, void * b, size_t size)
{
	char temp[size];
  // char serves as the type for "generic" byte arrays

  memcpy(temp, b,    size);
  memcpy(b,    a,    size);
  memcpy(a,    temp, size);
}

int compare(const void *p1, const void *p2)
{
    const pair *elem1 = p1;    
    const pair *elem2 = p2;

   if (elem1->time < elem2->time)
      return -1;
   else if (elem1->time > elem2->time)
      return 1;
   else
      return 0;
}

int compareScores(const void *s1,const void *s2) 
{
    if (((players*)s1)->score > ((players*)s2)->score ) return -1;
    if (((players*)s1)->score == ((players*)s2)->score  ) return 0;
    return 1;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



bool ResolveDynamicRectVsRect(ballstruct * r_dynamic, const float fTimeStep, blockstruct * r_static, float push_back)
{
	Vector2 contact_point, contact_normal;
	float contact_time = 0.0f;
	if (DynamicRectVsRect(r_dynamic->rect, r_static->rect, r_dynamic->velocity, &contact_point, &contact_normal, &contact_time, fTimeStep))
	{
		if (contact_normal.y > 0) r_dynamic->contact[0] = r_static->rect; else NULL;
		if (contact_normal.x < 0) r_dynamic->contact[1] = r_static->rect; else NULL;
		if (contact_normal.y < 0) r_dynamic->contact[2] = r_static->rect; else NULL;
		if (contact_normal.x > 0) r_dynamic->contact[3] = r_static->rect; else NULL;

		r_dynamic->velocity.x += contact_normal.x * (fabs(r_dynamic->velocity.x)) * (1 - contact_time) * push_back;
		r_dynamic->velocity.y += contact_normal.y * (fabs(r_dynamic->velocity.y)) * (1 - contact_time) * push_back;
		return true;
	}

	return false;
}


bool RectVsRectAABB(Rectangle a, Rectangle b)
{
	return (a.x < b.x + b.width && 
			a.x + a.width > b.x && 
			a.y < b.y + b.height && 
			a.y + a.height > b.y);
}

char *readfile(const char * dir, int * chars_in_line, int * total_num_lines)
{
	//Does not work for files greater than 4GB
	FILE *fptr;
	fptr = fopen(dir, "r");
	if(fptr)
	{
		findfileDim(fptr, chars_in_line, total_num_lines);
		//size_t file_size = ((*chars_in_line + 1)) * (*total_num_lines) * sizeof(char);
		char *buffer = (char*)calloc(((*chars_in_line + 1)) * (*total_num_lines), sizeof(char));
		if(buffer == NULL)
		{
			printf("Could not allocate the required memory!");
		} else
		{
			char c;
			int count = 0;
			for(c = getc(fptr); c != EOF; c = getc(fptr))
			{
				if(c != '\n' && c!= '\r') strncat(buffer, &c, 1);//buffer[count] = c;
				++count;
			}
			
			//fread(buffer, 1, file_size, fptr);
			buffer[count] = '\0';
		}
		fclose(fptr);
		return buffer;
	}
	fclose(fptr);
	printf("Could not find the file!");
	return NULL;
}

void findfileDim(FILE *fptr, int * chars_in_line, int *total_num_lines)
{
	char c;
	int length = 0, height = 0;
	if(fptr)
	{
		// Extract characters from file and store in character c
	    for (c = getc(fptr); c != EOF; c = getc(fptr))
	    {
	    	if (c == '\n') // Increment count if this character is newline
	    	{
	    		++height;
			} else if(height == 0)
			{
				++length;
			}
		}
		++height;
		//++length;
	}else{
		printf("Could not find the file!");
	}
	rewind(fptr);
	*chars_in_line = length;
	*total_num_lines = height;
}

players* GetSortedScores(const char * dir) {
	int i = 1, j = 0;
	char buffer[50];
	
	FILE *fptr ;
	
	if((fptr = fopen(dir, "r")) == NULL){
		printf("Score file not found. \n");
		return NULL;
	}
	fgets(buffer, 50, fptr);
	while(!feof(fptr)) 
	{
		fgets(buffer, 50, fptr);
		++i;
	}
	
	
	players *sc = (players *) malloc(i * sizeof(players));
	players *p = sc;
	
	fseek(fptr, 0, SEEK_SET);
	fgets(buffer, 50, fptr);
	
	while(!feof(fptr)) 
	{
		p = sc + j;
		sscanf(buffer,"%s %d", p->name, &p->score);
		fgets(buffer, 50, fptr);
		++j;
	}
	
	p = sc + j;
	sscanf(buffer, "%s %d", p->name, &p->score);
	
	fclose(fptr);
	
	qsort(sc, j + 1, sizeof(players), compareScores);
	
	return sc;
}

void addScore(const char * dir, char * name, int score) {
  
  FILE *fptr;
  if((fptr = fopen(dir, "a")) == NULL) printf("Could not find the score file.");
  fprintf_s(fptr, "\n%s %d", name, score);
  fclose(fptr);
}
