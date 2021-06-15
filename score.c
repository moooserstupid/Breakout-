#include<stdio.h>
#define		x	collide	//i cant find your defination for collision between ball and rectangles..So x the collision and if x==1 it means collide was happened.
#define		y	GameOver //i saw a code block about game over but you know our game cant over the game. when the ball falls ahead background code screen typing "game over" so i will use this .When the ball is falls under screen background says gameover and score will going to reduce.
						//but still y means falling of the ball.


int Score=0;
int hiScore
while("while game running"){

if(x==1){
	Score+=10
}
if(y==1){
	Score-=20;
}
if (score>hiScore)
	hiScore=Score;
}
// you were write "if(ball.lives==0) gameover=true;" 
//1st we must define a start value for ball.lives
//2nd we can over the game with exit(); function and ball.lives value
//3rd for example we define 10 live for ball. when it falls down score will be reduce.If user falls the ball 10 times game will over.

