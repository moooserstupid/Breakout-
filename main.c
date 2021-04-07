#include "include/raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>


#define pi pi 3.14159f
#define SCREENWIDTH 800
#define SCREENHEIGHT 750

const int MAXBLOCKSX = 25;
const int MAXBLOCKSY = 20;


typedef struct BatStruct{
	Rectangle Rect;
	Color batcolor;
	float fSpeed;
} batstruct;

typedef struct BallStruct{
	Rectangle Rect;
	Vector2 D;
	Color ballcolor;
	float fSpeed;
} ballstruct;

typedef struct BlockStruct{
	Rectangle Rect;
	int hitstokill;
	Color blockcolor;
	char cBlockType;
} blockstruct;

bool collides(struct Rectangle a, struct Rectangle b);
float map(float src, int srclowerbound, int srcupperbound, int destlowerbound, int destupperbound);
int collisionside(struct Rectangle a, struct Rectangle b);
void brickballcollision(blockstruct * a, ballstruct * ball, ballstruct * oldball);
float vecdistance(Vector2 a, Vector2 b);
int main()
{
	int nCountX, nCountY; 
	char cShape[MAXBLOCKSX * MAXBLOCKSY];
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, "......*$$$$$$$...........");		//Block arrangement
	strcat(cShape, "...........*********.....");
	strcat(cShape, ".........********........");
	strcat(cShape, "......***////*****.......");
	strcat(cShape, "......*****++++*****.....");
	strcat(cShape, ".....****************....");
	strcat(cShape, "......**************.....");
	strcat(cShape, ".....**************......");
	strcat(cShape, ".........................");
	strcat(cShape, "............///////......");
	strcat(cShape, ".........................");		
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	strcat(cShape, ".........................");
	
	Rectangle GameWindow = {0, 0, SCREENWIDTH, SCREENHEIGHT};
	
	blockstruct Block[MAXBLOCKSX * MAXBLOCKSY];
	int nArrIndex;
	for(nCountY = 0; nCountY < MAXBLOCKSY; nCountY++)
	{
		for(nCountX = 0; nCountX < MAXBLOCKSX; nCountX++)
		{
			nArrIndex = nCountY  * MAXBLOCKSX + nCountX;
			
			Block[nArrIndex].Rect.width = GameWindow.width / MAXBLOCKSX;
			Block[nArrIndex].Rect.height = (GameWindow.height - 150.0f) / MAXBLOCKSY;
			Block[nArrIndex].Rect.x = Block[nArrIndex].Rect.width * (nCountX - 1);  
			Block[nArrIndex].Rect.y = Block[nArrIndex].Rect.height * (nCountY - 1); 
			
			switch(cShape[nArrIndex])
			{
				case '*':     
					Block[nArrIndex].cBlockType = '*';
					Block[nArrIndex].hitstokill = 2;
					Block[nArrIndex].blockcolor = RED;
					break;
				case '/':
					Block[nArrIndex].cBlockType = '/';
					Block[nArrIndex].hitstokill = 3;
					Block[nArrIndex].blockcolor = BLUE;
					break;
				case '+':
					Block[nArrIndex].cBlockType = '+';
					Block[nArrIndex].hitstokill = 4;
					Block[nArrIndex].blockcolor = PURPLE;
					break;
				case '$':
					Block[nArrIndex].cBlockType = '$';
					Block[nArrIndex].hitstokill = 5;
					Block[nArrIndex].blockcolor = LIME;
					break;
				default:
					Block[nArrIndex].cBlockType = '.';
			}
			
		}
	}
	
	batstruct Bat;
	batstruct OldBat;
	Bat.Rect.height = 8.0f;
	Bat.Rect.width = 100.0f;
	Bat.Rect.x = ((float)GameWindow.width - Bat.Rect.width)/ 2.0f;
	Bat.Rect.y = GameWindow.height - 50;
	Bat.fSpeed = 10.0f;
	
	ballstruct Ball;
	ballstruct OldBall;
	Ball.Rect.x = (float)GameWindow.width / 2.0f;
	Ball.Rect.y = (float)GameWindow.height / 2.0f + 150.0f;
	Ball.Rect.width = 16.0f;
	Ball.Rect.height = 16.0f;
	Ball.fSpeed = 10.0f;
	float fAngle = 0.6f;
	Ball.D.x = cosf(fAngle);
	Ball.D.y = sinf(fAngle);
	
	bool bGameOver = false;
	
	InitWindow(GameWindow.width, GameWindow.height, "BreakOut!");
	
	SetTargetFPS(60);
	
	while(!WindowShouldClose())
	{
		OldBat.Rect.x = Bat.Rect.x;
		
		
		
		
		
		if(IsKeyDown(KEY_RIGHT)) 
		{
			Bat.Rect.x += Bat.fSpeed;
			if(!collides(Bat.Rect, GameWindow)) Bat.Rect.x -= Bat.fSpeed; 
		}
        if(IsKeyDown(KEY_LEFT)) 
		{
			Bat.Rect.x -= Bat.fSpeed;
			if(!collides(Bat.Rect, GameWindow)) Bat.Rect.x += Bat.fSpeed;
		}
		
		OldBall.Rect.x = Ball.Rect.x;
		OldBall.Rect.y = Ball.Rect.y;
		
		Ball.Rect.x += Ball.D.x * Ball.fSpeed;
		Ball.Rect.y += Ball.D.y * Ball.fSpeed;
		
		
		if(!collides(Ball.Rect, GameWindow))
		{
			if((Ball.Rect.x + Ball.Rect.width > GameWindow.width || Ball.Rect.x < GameWindow.x))
			{
				Ball.D.x *= -1;
			}
			if (Ball.Rect.y < GameWindow.y)
			{
				Ball.D.y *= -1;
			}
			 
			if(Ball.Rect.y + Ball.Rect.height > GameWindow.height) {
				printf("Game Over.\n");
				bGameOver = true;
			}  
		}
		
		
		if(collides(Ball.Rect, Bat.Rect))
		{
			
			
			if (Ball.D.y > 0)
                {
                    Ball.D.y *= -1;
                    if(Bat.Rect.x > OldBat.Rect.x){
                    	if(Ball.D.x > 0)
                    	{
                    		Ball.D.x *= 1;
						}else if(Ball.D.x < 0){
							Ball.D.x *= -1;
						}
					} else if(Bat.Rect.x < OldBat.Rect.x)
					{
						if(Ball.D.x < 0)
                    	{
                    		Ball.D.x *= 1;
						}else if(Ball.D.x > 0){
							Ball.D.x *= -1;
						}
						
					} 
					Ball.Rect.x = OldBall.Rect.x;
					Ball.Rect.y = OldBall.Rect.y;
					
                    printf("%f\n", Ball.D.x);
                }
		}
		brickballcollision(Block, &Ball, &OldBall);
		
		if(bGameOver)
		{
			Ball.Rect.x = (float)GameWindow.width / 2;
			Ball.Rect.y = (float)GameWindow.height / 2 + 150.0f;
			bGameOver = false;
		}
		
		BeginDrawing();
		//Draw stuff out here
		
		ClearBackground(RAYWHITE);
		
		for(nArrIndex = 0; nArrIndex < MAXBLOCKSX * MAXBLOCKSY; nArrIndex++)
		{
			if(Block[nArrIndex].cBlockType != '.')
			{
				DrawRectangleRec(Block[nArrIndex].Rect, Block[nArrIndex].blockcolor);
			}
			
		}
		//DrawRectangle(Ball.Rect.x, Ball.Rect.y, Ball.Rect.width, Ball.Rect.height, GREEN);
		DrawCircle(Ball.Rect.x + (Ball.Rect.width / 2), Ball.Rect.y + (Ball.Rect.height / 2), (float)Ball.Rect.width / 2, ORANGE);
		
		
		DrawRectangle(Bat.Rect.x, Bat.Rect.y, Bat.Rect.width, Bat.Rect.height, BLUE); //Draws the Bat
		
		EndDrawing();
	}
	return 0;
}

bool collides(struct Rectangle a, struct Rectangle b)
{
	return (a.x < b.x + b.width && 
			a.x + a.width > b.x && 
			a.y < b.y + b.height && 
			a.y + a.height > b.y);
}


float map(float src, int srclwrbound, int srcupprbound, int dstlwrbound, int dstupprbound)
{
	return (src - srclwrbound) * (dstupprbound - dstlwrbound) / (srcupprbound - srclwrbound) + dstlwrbound;
}

void brickballcollision(blockstruct * a, ballstruct * ball, ballstruct * oldball)
{
	int counter;
	for(counter = 0; counter < MAXBLOCKSX * MAXBLOCKSY; counter++)
	{
		if(a[counter].cBlockType != '.' && collides(a[counter].Rect, ball->Rect))
		{
			a[counter].hitstokill -= 1;
			if(a[counter].hitstokill == 0)
			{
				a[counter].cBlockType = '.';
			} else{
				switch(a[counter].hitstokill)
				{
					case 1:
						a[counter].blockcolor = GREEN;
						break;
					case 2:
						a[counter].blockcolor = RED;
						break;
					case 3:
						a[counter].blockcolor = BLUE;
						break;
					case 4:
						a[counter].blockcolor = PURPLE;
						break;
				}
				int colside = collisionside(ball->Rect, a[counter].Rect);
				switch(colside)
				{
					case 0:
						ball->D.x *= -1;
						break;
					case 1:
						ball->D.x *= -1;
						ball->D.y *= -1;
						break;
					case 2:
						ball->D.y *= -1;
						break;
					case 3:
						ball->D.x *= -1;
						ball->D.y *= -1;
						break;
				}
				//if(ball->D.y < 0)ball->D.y *= -1;
				ball->Rect.x = oldball->Rect.x;
				ball->Rect.y = oldball->Rect.y;
			}
			
			
		}
	}
	
}

int collisionside(struct Rectangle a, struct Rectangle b)
{
	// b should not be the ball.
	//See the collision area png for colour context
	int returnvalue;
	float bw = (float)(b.width / 2.0f);
	float bh = (float)(b.height / 2.0f);
	float dx = (float)(abs(a.x - b.x) - bw);
	float dy = (float)(abs(a.y - b.y) - bh);
	if(dx > dy)
	{
		// Blue and Magenta
		returnvalue = 0;
		if(dx >= 0 && dy >= 0)
		{
			//Magenta
			returnvalue = 1; 
		}
	} else
	{
		//Green and Yellow
		returnvalue = 2;
		if(dx >= 0 && dy >= 0)
		{
			//Yellow
			returnvalue = 3;	
		}
	} 
	printf("%d\n", returnvalue);
	return returnvalue;
	
}

float vecdistance(Vector2 a, Vector2 b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}





