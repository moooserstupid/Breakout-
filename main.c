#include "include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define pi 3.14159f
#define MAXSCREENWIDTH 800
#define MAXSCREENHEIGHT 750
#define MAXBLOCKWIDTH 25
#define MAXBLOCKHEIGHT 20

typedef struct BatStruct{
	Rectangle Rect;
	float fSpeed;
} batstruct;

typedef struct BallStruct{
	Rectangle Rect;
	Vector2 D;
	float fSpeed;
} ballstruct;

typedef struct BlockStruct{
	Rectangle Rect;
	bool bPrint;
	bool bSurface;
} blockstruct;


float random_float(float min, float max)
{
	float scale = (float) rand() / (float) RAND_MAX;
	return min + scale * (max - min);
}
float normalize(float vectorcomp, float magnitude)
{
	if(magnitude != 0)
	{
		return (float)vectorcomp / magnitude;
	}else{
		return 0.0f;
	}
	
}

int collides(struct Rectangle a, struct Rectangle b)
{
	//uses the Minkowski Sum to detect whether there is a collision and what sides are colliding
	int returnvalue = -1;
	float w = 0.5 * (a.width + b.width);
	float h = 0.5 * (a.height + b.height);
	float dx = (a.x / 2.0f) - (b.x / 2.0f);
	//printf("%f %f \n", a.height, b.height);
	float dy = (a.y / 2.0f) - (b.y / 2.0f);
	printf("dx = %f dy = %f \n", dx, dy);
	
	if(abs(dx) <= w && abs(dy) <= h)
	{
		float wy = w * dy;
		float hx = h * dx;
		
		if(wy > hx)
		{
			if(wy > -hx) {
			returnvalue = 0; //collision at the top
			} else {
				returnvalue = 1; //collsion at the left
			}
		} else {
			if(wy > -hx) {
				returnvalue = 2; //collsion at the right
			} else {
				returnvalue = 3; //collsion at the bottom
			}
		}
		
	}
	///printf("returnvalue = %d\n", returnvalue);
	return returnvalue;
}



int main(void)
{
	srand(time(NULL));
	int nCountX, nCountY; 							//Variables used for counting iterations
	//Vector2 Center = {(float)MAXBLOCKWIDTH / 2, (float)MAXBLOCKHEIGHT / 2};
	char cShape[MAXBLOCKWIDTH * MAXBLOCKHEIGHT];
	strcat(cShape, ".........................");
	strcat(cShape, "........************.....");
	strcat(cShape, "......********...........");		//Block arrangement
	strcat(cShape, "...........*********.....");
	strcat(cShape, ".........********........");
	strcat(cShape, "......************.......");
	strcat(cShape, "......**************.....");
	strcat(cShape, ".....****************....");
	strcat(cShape, "......**************.....");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");		
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	
	Rectangle GameWindow = {0, 0, MAXSCREENWIDTH, MAXSCREENHEIGHT};
	
	blockstruct Block[MAXBLOCKHEIGHT * MAXBLOCKWIDTH];
	int nArrIndex;
	for(nCountY = 0; nCountY < MAXBLOCKHEIGHT; nCountY++)
	{
		for(nCountX = 0; nCountX < MAXBLOCKWIDTH; nCountX++)
		{
			nArrIndex = nCountY * MAXBLOCKWIDTH + nCountX;
			switch(cShape[nArrIndex])
			{
				case '*':
					Block[nArrIndex].Rect.width = 32;
					Block[nArrIndex].Rect.height = 32;
					Block[nArrIndex].Rect.x = nCountX * Block[nArrIndex].Rect.width; //Center.x - ((float)MAXBLOCKWIDTH / 2) + 
					Block[nArrIndex].Rect.y = nCountY * Block[nArrIndex].Rect.height; //Center.y - ((float)MAXBLOCKWIDTH / 2) + 
					Block[nArrIndex].bPrint = true;
					
					break;
				default:
					Block[nArrIndex].bPrint = false;
					break;
			}
			
		}
	}
	
	
	batstruct Bat;
	Bat.Rect.height = 8;
	Bat.Rect.width = 100;
	Bat.Rect.x = ((float)GameWindow.width - Bat.Rect.width)/ 2;
	Bat.Rect.y = GameWindow.height - 50;
	Bat.fSpeed = 10.0f;
	
	ballstruct Ball;
	ballstruct OldBall;								//Cache of the Ball's Previous Position
	Ball.Rect.x = (float)GameWindow.width / 2;
	Ball.Rect.y = (float)GameWindow.height / 2 + 150.0f;
	Ball.Rect.width = 16;
	Ball.Rect.height = 16;
	Ball.fSpeed = 5.0f;
	float fAngle = 0.6;//random_float(0, 1) * pi * 2.0f;
	Ball.D.x = cosf(fAngle);
	Ball.D.y = sinf(fAngle);
	
	bool bGameOver = false;
	
	InitWindow(GameWindow.width, GameWindow.height, "BreakOut!");
	
	SetTargetFPS(60);
	
	while(!WindowShouldClose())
	{
		//Update game variables here
		if(bGameOver)
		{
			Ball.Rect.x = (float)GameWindow.width / 2;
			Ball.Rect.y = (float)GameWindow.height / 2 + 150.0f;
		}
		OldBall.Rect.x = Ball.Rect.x;
		OldBall.Rect.y = Ball.Rect.y;
		
		if(IsKeyDown(KEY_RIGHT)) 
		{
			Bat.Rect.x += Bat.fSpeed;
			if(collides(Bat.Rect, GameWindow) == -1) Bat.Rect.x -= Bat.fSpeed; 
		}
        if(IsKeyDown(KEY_LEFT)) 
		{
			Bat.Rect.x -= Bat.fSpeed;
			if(collides(Bat.Rect, GameWindow) == -1) Bat.Rect.x += Bat.fSpeed;
		}
		
		if(collides(Ball.Rect, Bat.Rect) == 0|| collides(Ball.Rect, Bat.Rect) == 3) Ball.D.y *= -1;
		if(collides(Ball.Rect, Bat.Rect) == 1|| collides(Ball.Rect, Bat.Rect) == 4) Ball.D.x *= -1;
		
		Ball.Rect.x += Ball.D.x * Ball.fSpeed;
		Ball.Rect.y += Ball.D.y * Ball.fSpeed;
		if(!CheckCollisionRecs(Ball.Rect, GameWindow))
		{
			if((Ball.Rect.x > GameWindow.width || Ball.Rect.x < GameWindow.x) && Ball.Rect.y < GameWindow.height)
			{
				Ball.D.x *= -1;
				Ball.Rect.x = OldBall.Rect.x + Ball.D.x * Ball.fSpeed;
			}
			else if (Ball.Rect.y < GameWindow.y && Ball.Rect.x < GameWindow.width)
			{
				Ball.D.y *= -1;
				Ball.Rect.y = OldBall.Rect.y + Ball.D.y * Ball.fSpeed;
			} 
			else {
				
				bGameOver = true;
			}
			
			 
		}
		BeginDrawing();
		//Draw stuff out here
		
		ClearBackground(RAYWHITE);
		
		for(nArrIndex = 0; nArrIndex < MAXBLOCKWIDTH * MAXBLOCKHEIGHT; nArrIndex++)
		{
			if(Block[nArrIndex].bPrint)
			{
				DrawRectangleRec(Block[nArrIndex].Rect, RED);
			}
			
		}
		DrawRectangle(Ball.Rect.x, Ball.Rect.y, Ball.Rect.width, Ball.Rect.height, GREEN);
		DrawCircle(Ball.Rect.x + 8, Ball.Rect.y + 8, (float)Ball.Rect.width / 2, ORANGE);
		
		
		DrawRectangle(Bat.Rect.x, Bat.Rect.y, Bat.Rect.width, Bat.Rect.height, BLUE); //Draws the Bat
		
		EndDrawing();
	}
	
	
	
}










