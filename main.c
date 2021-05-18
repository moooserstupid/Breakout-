#include "include/raylib.h"
#include "include/raymath.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define U_MIN(a,b)         (((a)<(b))?(a):(b))
#define U_MAX(a,b)         (((a)>(b))?(a):(b))

#define SCREENWIDTH 800
#define SCREENHEIGHT 650

static Rectangle GameWindow = {0, 0, SCREENWIDTH, SCREENHEIGHT};

static int MAXBLOCKSX = 25;
static int MAXBLOCKSY = 20;

typedef struct Pair{
	int index;
	float time;
} pair;

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
} blockstruct;

typedef struct BallStruct{
	Rectangle rect;
	Color colour;
	Vector2 velocity;
	Rectangle contact[4];
	bool active;
	int lives;
} ballstruct;

bool RayVsRect(Vector2, Vector2, Rectangle, Vector2 *, Vector2 *, float *);
bool DynamicRectVsRect(Rectangle, Rectangle, Vector2, Vector2 *, Vector2 *, float *, float);
bool ResolveDynamicRectVsRect(ballstruct *, const float, blockstruct *, float);
bool RectVsRectAABB(Rectangle, Rectangle);

void swap(void *, void *, size_t);
void normalize(Vector2 *);
int compare(const void *, const void *);
int main()
{
	int count;
	int max_blocks = MAXBLOCKSX * MAXBLOCKSY;
	InitWindow(GameWindow.width, GameWindow.height, "Breakout!");
	ballstruct ball;
	ball.velocity = (Vector2){0, 500.0f};
	ball.rect = (Rectangle){GameWindow.width / 2.0f, GameWindow.height / 2.0f + 150.0f, 16, 16};
	ball.colour = BLUE;
	ball.active = true;
	float floor_velocity = 250.0f;
	
	blockstruct block[max_blocks];
	char cShape[max_blocks];
	cShape[0] = '\0';
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
	
	
	int countX, countY;
	int arrindex = 0;
	for(countY = 0; countY < MAXBLOCKSY; countY++)
	{
		for(countX = 0; countX < MAXBLOCKSX; countX++)
		{
			arrindex = countY  * MAXBLOCKSX + countX;
//			if(game_map[arrindex] == '\n')
//			{
//				
//				//arrindex = (++countY) * MAXBLOCKSX + countX;
//				printf("New line.\n");
//				break;
//			}
			switch(cShape[arrindex])
			{
				case '*':     
					(block + arrindex)->active = true;
					(block + arrindex)->hitstokill = 2;
					(block + arrindex)->colour = RED;
					break;
				case '/':
					(block + arrindex)->active = true;
					(block + arrindex)->hitstokill = 3;
					(block + arrindex)->colour = BLUE;
					break;
				case '+':
					(block + arrindex)->active = true;
					(block + arrindex)->hitstokill = 4;
					(block + arrindex)->colour = PURPLE;
					break;
				case '$':
					(block + arrindex)->active = true;
					(block + arrindex)->hitstokill = 5;
					(block + arrindex)->colour = LIME;
					break;
				default:
					(block + arrindex)->active = false;
			}
			if((block + arrindex)->active)
			{
				(block + arrindex)->rect.width = GameWindow.width / MAXBLOCKSX;
				(block + arrindex)->rect.height = (GameWindow.height - 150.0f) / MAXBLOCKSY;
				(block + arrindex)->rect.x = (block + arrindex)->rect.width * countX;  
				(block + arrindex)->rect.y = (block + arrindex)->rect.height * countY;
			}
		}
	}
	batstruct bat;
	bat.rect.height = 8.0f;
	bat.rect.width = 100.0f;
	Rectangle bat_bounds = (Rectangle){GameWindow.x + bat.rect.width, GameWindow.height - 125 + bat.rect.height, GameWindow.width - bat.rect.width * 2, 125 - bat.rect.height * 2};
	bat.rect.x = (GameWindow.width - bat.rect.width)/ 2;
	bat.rect.y = bat_bounds.y;
	bat.colour = BROWN;
	
	bool GameOver = false;
	bool SlowMo = false;
	bool Pause = false;
	
	int sling_force = 0;
	Vector2 ray_direction;
	Vector2 cn, cp;
	float ct;
	float elapsed_time;
	pair z[max_blocks];
	int framesCounter = 0;
	
	SetTargetFPS(60);
	
	//Note to myself:There are alot of useful vector processing functions in raymath.h I really need to start to use them
	while(!WindowShouldClose())
	{
		if(!GameOver)
		{
			if(IsKeyPressed('P')) Pause = !Pause;
			if(!Pause)
			{
				float time_step = GetFrameTime();
				elapsed_time = GetTime();
				bat.velocity = Vector2Zero();  //Initial Velocity
				if(IsMouseButtonDown(MOUSE_LEFT_BUTTON))
				{
					Vector2 MousePos = GetMousePosition();
					ray_direction = (Vector2){MousePos.x - ball.rect.x, MousePos.y - ball.rect.y};
					ray_direction = Vector2Normalize(ray_direction);
					ball.velocity.x += ray_direction.x * time_step * 1000.0f;
					ball.velocity.y += ray_direction.y * time_step * 1000.0f;
				}
				
				
				if(IsKeyDown(KEY_RIGHT)) 
				{
					bat.velocity.x  = 500.0f;
					bat.rect.x += bat.velocity.x * time_step;
					if(!RectVsRectAABB(bat.rect, bat_bounds)) bat.rect.x -= bat.velocity.x * time_step; 
				}
			    if(IsKeyDown(KEY_LEFT)) 
				{
					bat.velocity.x = -500.0f;
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
						if(sling_force > 0) ball.velocity.y += sling_force;
						ball.velocity.y *= -1;
						
						//Then we calculate the velocity for the x component
						ball.velocity.x += bat.velocity.x;
	//					if(bat.velocity.x > 0)
	//					{
	//						if(ball.velocity.x > 0) ball.velocity.x *= -1;
	//					}
						
						
						
					}
				}
				
				//This code stops the ball from slowing down too much
				if(fabs(ball.velocity.x) < floor_velocity || fabs(ball.velocity.y ) < floor_velocity)
				{
					ball.velocity.x = (ball.velocity.x >= 0) ? floor_velocity: -floor_velocity;
					ball.velocity.y = (ball.velocity.y >= 0) ? floor_velocity: -floor_velocity;
				}
				
				if (((ball.rect.x + ball.rect.width) >= GameWindow.width) || ((ball.rect.x) <= 0)) ball.velocity.x *= -1;
			    if ((ball.rect.y) <= 0) ball.velocity.y *= -1;
			    if ((ball.rect.y + ball.rect.height) >= GameWindow.height)
			    {
			    	//If you lose
			        ball.velocity = Vector2Zero();
			        ball.active = false;
			
			        ball.lives--; 
			    }
			    
			    
			    
				
				int zcount = 0;
				for(count = 0; count < max_blocks; count++)
				{
					if(block[count].active && DynamicRectVsRect(ball.rect, block[count].rect, ball.velocity, &cp, &cn, &ct, time_step))
					{
						z[zcount].index = count;
						z[zcount].time = ct;
						zcount++;
						--block[count].hitstokill;
						switch(block[count].hitstokill)
						{
							case -1:
								block[count].active = false;
								break;
							case 1:
								block[count].colour = GREEN;
								break;
							case 2:
								block[count].colour = RED;
								break;
							case 3:
								block[count].colour = BLUE;
								break;
							case 4:
								block[count].colour = PURPLE;
								break;
		
						}
					}
				}
				qsort(z, zcount, sizeof(pair), compare);
				
				
				for(count = 0; count < zcount; count++)
				{
					
					ResolveDynamicRectVsRect(&ball, time_step, &block[z[count].index], 2.1f);
				}
				
				if(ball.active)
				{
					ball.rect.x += ball.velocity.x * time_step;
					ball.rect.y += ball.velocity.y * time_step;
				} else{
					ball.rect.x = (float)GameWindow.width / 2;
					ball.rect.y = (float)GameWindow.height / 2 + 150.0f;
				}
				
				
				
				if(ball.lives <= 0) GameOver = true;
			    else{
			    	GameOver = true;
			    	int count;
			    	for(count = 0; count < MAXBLOCKSX * MAXBLOCKSY; count++)
			    	{
			    		if((block + count)->active) GameOver = false;
					}
			}
			} else framesCounter++;
			
			
		} else if(IsKeyPressed(KEY_ENTER))
		{
			GameOver = false;
			
		}
		
		BeginDrawing();
		
		ClearBackground(RAYWHITE);
		DrawRectangleRec(bat_bounds, YELLOW);
		const char * ballVelocity = TextFormat("X = %2f Y = %2f", ball.velocity.x, ball.velocity.y);
		
		DrawText(ballVelocity, ball.rect.x + 10.0f, ball.rect.y, 10, GREEN);
		DrawCircle(ball.rect.x + ball.rect.width / 2 , ball.rect.y + ball.rect.height / 2, ball.rect.width / 2, ball.colour);
		
		for(count = 0; count < max_blocks; count++)
		{
			if(block[count].active)
			{
				DrawRectangleRec(block[count].rect, block[count].colour);
			}
			
		}
		
		DrawRectangle(bat.rect.x, bat.rect.y, bat.rect.width, bat.rect.height, bat.colour);
		
		if (Pause && ((framesCounter/30)%2)) DrawText("PAUSED", 350, 200, 30, GRAY);
		DrawFPS(10, 10);
		
		
		EndDrawing();
	}
	CloseWindow();
	
	return 0;
}

bool RayVsRect(Vector2 ray_origin, Vector2 ray_dir, Rectangle target, Vector2 * contact_point, Vector2 * contact_normal, float * t_hit_near)
{
	/* All the functions dealing with collision were learned from Javid on the OneLoneCoder youtube.
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


