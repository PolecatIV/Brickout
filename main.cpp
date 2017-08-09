
//TODO: fix per il sistema di fps count



#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>

using namespace std;

const int SCREEN_HEIGHT = 720;
const int SCREEN_WIDTH = 1280;
const int NUM_BLOCKS = 33;
const int JOYSTICK_DEAD_ZONE = 8000;
const int FPS = 60;
const float PADMAXSPEED = 25.0f;

struct pos
{
	float x = 0.0f;
	float y = 0.0f;
};

struct object
{
	string name;
	SDL_Texture *texture = NULL;	//NOTE: Texture of the object
	SDL_Rect sdlrect;				//NOTE: Info for dimensions and position in SDL format
	SDL_Rect oldRect;				//NOTE: previous frame position
	pos rect;						//NOTE:
	glm::vec4 vect;					//NOTE: Vector for velocity
	glm::vec4 oldVect;				//NOTE: Vector of the previous frame
	glm::mat4 matrix = glm::mat4();	//NOTE: World matrix
	float angle = 0.0f;				//NOTE: Angle if needed
	glm::vec4 rotLeft;				//NOTE: Center for the left rotation
	glm::vec4 rotRight;				//NOTE: Center for the right rotation
	Sint16 xMove = 0;				//NOTE: (only for Pad) Offset from last input
	Sint16 yMove = 0;				//NOTE: (only for Pad) Offset from last input
	float lBend = 0.0f;
	float rBend = 0.0f;
	int energy = 0;

	inline void positionToSDL();

};

inline void object::positionToSDL()
{
	this->sdlrect.x = (int)this->rect.x;
	this->sdlrect.y = (int)this->rect.y;
}

//NOTE: Check for collision between ball and rotate objects
SDL_bool BallPadCollision(object *Ball, object *Pad, int *NoCollisionCount) 
{
	Ball->positionToSDL();
	Pad->positionToSDL();
	glm::vec3 vector = glm::vec3();
	glm::vec3 n = glm::vec3();
	glm::vec3 reflected = glm::vec3();

	glm::mat4 rotation = glm::mat4();
	rotation = Pad->matrix;

	vector = glm::vec3(Ball->vect.x, Ball->vect.y, 0.0f);
	n = glm::vec3(Pad->matrix[1].x, Pad->matrix[1].y, 0.0f);

	SDL_Rect BallPadResult;
	SDL_bool Result = SDL_IntersectRect(&Ball->sdlrect, &Pad->sdlrect, &BallPadResult);


	if (Result && *NoCollisionCount == 0)
	{

		if (*NoCollisionCount == 0)
		{
			

			if (BallPadResult.h <= BallPadResult.w)
			{

				reflected = glm::reflect(vector, n);
				Ball->vect.x = reflected.x;
				Ball->vect.y = reflected.y;

				if (Ball->rect.y >= Pad->rect.y + (Pad->sdlrect.h / 2))
				{
					Ball->rect.y = Pad->rect.y + Pad->sdlrect.h + 2;

				}
				else
				{
					Ball->rect.y = Pad->rect.y - Ball->sdlrect.h - 2;
				}
			
			}
			else
			{
				float theta;
				if (BallPadResult.x < Pad->rect.x + (Pad->sdlrect.w / 2))
				{
					theta = glm::radians(-90.0f);
					//cout << "\nLEFT";
					rotation = glm::rotate(theta, glm::vec3(0.0f, 0.0f, 1.0f)) * rotation;
					
				}
				else if (BallPadResult.x > Pad->rect.x + (Pad->sdlrect.w / 2))
				{
					theta = glm::radians(90.0f);
					//cout << "\nRIGHT";
					rotation = glm::rotate(theta, glm::vec3(0.0f, 0.0f, 1.0f)) * rotation;
				}
				
				n.x = rotation[1].x;
				n.y = rotation[1].y;
				
				reflected = glm::reflect(vector, n);
				Ball->vect.x = reflected.x;
				Ball->vect.y = reflected.y;


				if (Ball->rect.x >= Pad->rect.x + (Pad->sdlrect.w / 2))
				{
					Ball->rect.x = Pad->rect.x + Pad->sdlrect.w + Pad->xMove + 5;

				}
				else
				{
					Ball->rect.x = Pad->rect.x - Ball->sdlrect.w + Pad->xMove - 5;
				}

			}

			*NoCollisionCount = 10;
			//cout << *NoCollisionCount;

		}
		else
		{
			if (Pad->name == "Pad")
			{
				*NoCollisionCount = *NoCollisionCount - 1;
			}
				
			//cout << *NoCollisionCount;
		}
		
	}
	else
	{
		if (*NoCollisionCount > 0)
		{
			if (Pad->name == "Pad")
			{
				*NoCollisionCount = *NoCollisionCount - 1;
			}
			//cout << *NoCollisionCount;
		}
	}


	return Result;
}



int main(int argc, char *argv[])
{
	
	//NOTE: SDL Initializations
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	TTF_Init();
	//NOTE: Window handle
	SDL_Window* Window = NULL;
	Window = SDL_CreateWindow("Brickout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	if (!Window)
	{
		SDL_Quit();
		return 0;
	}
	

	//NOTE: Creating the objects 
	object Pad;
	Pad.name = "Pad";
	Pad.rect.x = 640;
	Pad.rect.y = 600;
	Pad.sdlrect.h = 32;
	Pad.sdlrect.w = 128;
	Pad.rotLeft.x = (float)(Pad.sdlrect.h / 2);
	Pad.rotLeft.y = (float)(Pad.sdlrect.h / 2);
	Pad.rotRight.x = (float)(Pad.sdlrect.w - Pad.rotLeft.x);
	Pad.rotRight.y = (float)(Pad.rotLeft.y);
	Pad.vect.x = 0.0f;
	Pad.vect.y = 0.0f;
	

	object Ball;
	Ball.name = "Ball";
	Ball.rect.x = 200;
	Ball.rect.y = 200;
	Ball.sdlrect.h = 16;
	Ball.sdlrect.w = 16;
	Ball.vect.x = 9.0f;
	Ball.vect.y = 9.0f; 

	object Ball2 = Ball;


	Pad.rect.x = 640;
	Pad.rect.y = 600;
	Ball.rect.x = 200;
	Ball.rect.y = 200;
	Ball.vect.x = 6.0f;
	Ball.vect.y = 6.0f;

	//NOTE: Hittable blocks init

	object Bricks[NUM_BLOCKS];

	for (int i = 0; i < NUM_BLOCKS; i++)
	{
		Bricks[i].name = "Brick";
		Bricks[i].sdlrect.w = 88;
		Bricks[i].sdlrect.h = 32;
		Bricks[i].energy = 1;
		Bricks[i].rBend = 0.0f;
		Bricks[i].lBend = 0.0f;
		
	}
	
	float y = 60.0f;
	
	int initblocks = 0;
	
	for (float y = 60.0f; y < SCREEN_HEIGHT; y += 50.0f)
	{
		float x = 60.0f;
		for (int i = 0; i < 11; i++)
		{
			Bricks[initblocks].rect.x = x;
			Bricks[initblocks].rect.y = y;
			initblocks++;
			x += 100.0f;
			Bricks[initblocks].positionToSDL(); 
			if (initblocks >= NUM_BLOCKS)
				break;
		}
		if (initblocks >= NUM_BLOCKS)
			break;
		
	}

	//NOTE: Ball\Pad collision frame count to avoid multiple consecutive collisions

	int NoCollisionCount = 0;


	//NOTE: main loop flag
	bool Quit = false;
	bool DebugOnScreen = true;

	//NOTE: Loading the joystick
	SDL_Joystick* Controller = NULL;
	Controller = SDL_JoystickOpen(0);

	if (SDL_NumJoysticks() < 1) 
	{
		printf("Warning: No joysticks connected!\n"); 
	}
	else 
	{ 
		//Load joystick 
		Controller = SDL_JoystickOpen( 0 ); 
		if( Controller == NULL ) 
		{
			printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() ); 
		}
	}

	printf("%s\n", SDL_JoystickName(Controller));

	//NOTE: SDL Renderer
	
	SDL_Renderer* Renderer = NULL;
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);// | SDL_WINDOW_FULLSCREEN);

	//NOTE: Texture loading
	SDL_Texture* padImg = NULL;
	SDL_Surface* padImgtemp = NULL;

	SDL_Texture* ballImg = NULL;
	SDL_Surface* ballImgtemp = NULL;

	SDL_Texture* brickImg = NULL;
	SDL_Surface* brickImgtemp = NULL;
	
	padImgtemp = SDL_LoadBMP("F:/dev/brickout/brickout/resources/pad.bmp");
	ballImgtemp = SDL_LoadBMP("F:/dev/brickout/brickout/resources/ball.bmp");
	brickImgtemp = SDL_LoadBMP("F:/dev/brickout/brickout/resources/brick.bmp");
	
	


	//NOTE: Creting the actual texture from SDL_Surface
	padImg = SDL_CreateTextureFromSurface(Renderer, padImgtemp);
	Pad.texture = padImg;

	ballImg = SDL_CreateTextureFromSurface(Renderer, ballImgtemp);
	Ball.texture = ballImg;

	brickImg = SDL_CreateTextureFromSurface(Renderer, brickImgtemp);
	Bricks[0].texture = brickImg;

	//NOTE: Main SDL Event
	SDL_Event Event;
	
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	
	SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);

	Pad.matrix = glm::mat4();
	Ball.matrix = glm::mat4();
	glm::mat4 BallLocMat = glm::mat4();

	TTF_Font *font = TTF_OpenFont("F:/dev/brickout/brickout/resources/Verdana.ttf", 10);

	SDL_Color color;
	color = { 0,0,50,255 };
	
	Uint32 start;
	float actualFPS = 0.0f;
	while (!Quit)
	{
		start = SDL_GetTicks();
		while (SDL_PollEvent(&Event)) 
		{
			switch (Event.type)
			{
				case SDL_KEYDOWN:
					switch (Event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							Quit = true;
							break;
						case SDLK_KP_PLUS:
							Ball.vect.x *= 1.1f;
							Ball.vect.y *= 1.1f;
						case SDLK_KP_MINUS:
							Ball.vect.x *= 0.95f;
							Ball.vect.y *= 0.95f;
	
						case SDLK_r:
							if ((SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN))
							{
								SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							}
							else
							{
								SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);
							}

						default:
							break;
					}
				case SDL_WINDOWEVENT:
					switch (Event.window.event)
					{
						case SDL_WINDOWEVENT_CLOSE:
							Quit = true;
					}

				
				default:
					break;
			}
		}
	
		//NOTE: Keyboard input
		if (state[SDL_SCANCODE_LEFT]) {
		
			Pad.rect.x -= 16;
		}
		if (state[SDL_SCANCODE_RIGHT]) {
			Pad.rect.x += 16;
		}
		if (state[SDL_SCANCODE_PAGEUP]) 
		{
			Ball.vect.x *= 1.1f;
			Ball.vect.y *= 1.1f;
		}
		if (state[SDL_SCANCODE_PAGEDOWN])
		{
			Ball.vect.x *= 0.95f;
			Ball.vect.y *= 0.95f;
		}

		

		//NOTE: Up and down not needed at the moment
		/*
		if (state[SDL_SCANCODE_UP]) {
			Pad.rect.y -= 3;
		}
		if (state[SDL_SCANCODE_DOWN]) {
			Pad.rect.y += 3;
		}
		*/

		//NOTE: Joypad input
		Sint16 xjoy;
		//Sint16 yjoy;

		float xValue = 0.0f;
		//float yValue = 0.0f;


		xjoy = SDL_JoystickGetAxis(Controller, 0);
		Pad.yMove = SDL_JoystickGetAxis(Controller, 1);
		if (xjoy < -JOYSTICK_DEAD_ZONE || xjoy > JOYSTICK_DEAD_ZONE)
		{
			//TODO: Clean the joypad input and check bug ib xValue when quick change in direction
			//Pad.rect.x += Pad.xMove; //NOTE: Old non vector movement

			xValue = ((float)xjoy / 1820.0f)*0.15f;
			
		
		}
		else
		{

			if (fabs(Pad.vect.x) < 0.2f && Pad.vect.x != 0.0f)
			{
				Pad.vect.x = 0.0f;
				
			}
			else
			{
				//NOTE: nice value for this is 0.8f
				Pad.vect.x *= 0.8f;
			}
			//Pad.xMove = 0;

		}

		
		if (fabs(Pad.vect.x) <= PADMAXSPEED)
		{
		
			Pad.vect.x += xValue;
			Pad.rect.x += (int)Pad.vect.x;
		}
		else
		{
		
			if (Pad.vect.x > 0.0f)
			{
				Pad.vect.x = PADMAXSPEED;
			}
			if (Pad.vect.x < 0.0f)
			{
				Pad.vect.x = -PADMAXSPEED;
			}

			
			//cout << "cccc";
			Pad.rect.x += (int)Pad.vect.x;
		}

		
		
		if (SDL_JoystickGetButton(Controller, 6))
		{
			Quit = true;
		}

		if (SDL_JoystickGetButton(Controller, 7))
		{
			if (DebugOnScreen == false)
			{
				DebugOnScreen = true;
			}
			else
			{
				DebugOnScreen = false;
			}
		}

		if (SDL_JoystickGetButton(Controller, 4))
		{
			Ball.vect.x *= 0.9f;
			Ball.vect.y *= 0.9f;
		}

		if (SDL_JoystickGetButton(Controller, 3))
		{
			Pad.rect.x = 640;
			Pad.rect.y = 635;
			Ball.rect.x = 200;
			Ball.rect.y = 200;
			Ball.vect.x = 6.0f;
			Ball.vect.y = 6.0f;
		}

		if (SDL_JoystickGetButton(Controller, 5))
		{
			if (Ball.vect.x == 0 && Ball.vect.y == 0)
			{
				Ball.vect.x = 1.0f;
				Ball.vect.y = 1.0f;
			}
			Ball.vect.x *= 1.1f;
			Ball.vect.y *= 1.1f;
		}

		if (state[SDL_SCANCODE_RETURN]) {
			printf("<RETURN> is pressed.\n");
		}
		
		Pad.lBend = (float)(SDL_JoystickGetAxis(Controller, 5) + 32768.0f);
		if (Pad.lBend > 0.0f)
		{
			Pad.lBend /= 2184.5f;
			Pad.lBend *= 1.5f;
		}
			//cout << Pad.lBend <<endl;
		
		
		Pad.rBend = (float)SDL_JoystickGetAxis(Controller, 2) + 32768.0f;
		if (Pad.rBend > 0.0f)
		{
			Pad.rBend /= 2184.5f;
			Pad.rBend *= 1.5f;
		}
		

		if (SDL_JoystickGetButton(Controller, 9))
		{
			for (int i = 0; i < NUM_BLOCKS; i++)
			{
				Bricks[i].sdlrect.w = 88;
				Bricks[i].sdlrect.h = 32;
				Bricks[i].energy = 1;
				Bricks[i].rBend = 0.0f;
				Bricks[i].lBend = 0.0f;

			}

			int y = 60;

			int initblocks = 0;

			for (int y = 60; y < SCREEN_HEIGHT; y += 50)
			{
				int x = 60;
				for (int i = 0; i < 11; i++)
				{
					Bricks[initblocks].sdlrect.x = x;
					Bricks[initblocks].sdlrect.y = y;
					initblocks++;
					x += 100;
					if (initblocks >= NUM_BLOCKS)
						break;
				}
				if (initblocks >= NUM_BLOCKS)
					break;
			}

		}


		//NOTE: Update the ball
		//Ball.vect.y += 0.5f;

		//NOTE: Code for halframe collision ---
		Ball.vect.x *= 0.25f;
		Ball.vect.y *= 0.25f;

		//-------------------------------------
		
		Ball.rect.x += (int)Ball.vect.x;
		Ball.rect.y += (int)Ball.vect.y;

		
		
		object objBallLoc = Ball;
		object Ball2 = {};

		object objPad = Pad;


		if (state[SDL_SCANCODE_RETURN])
		{
			Pad.rect.x = 640;
			Pad.rect.y = 635;
			Ball.rect.x = 200;
			Ball.rect.y = 200;
			Ball.vect.x = 6.0f;
			Ball.vect.y = 6.0f;

			for (int i = 0; i < NUM_BLOCKS; i++)
			{
				Bricks[i].energy = 1;						
			}

		}


		for (int i = 0; i < 4; i++)
		{
			//NOTE: Matrix operations for collisions
			if ((Pad.lBend == 0.0f && Pad.rBend == 0.0f) || (Pad.lBend > 0 && Pad.rBend > 0))
			{
				Pad.matrix = glm::mat4();
				Ball.matrix = glm::mat4();

				Pad.matrix = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * Pad.matrix;
				Pad.matrix = glm::translate(glm::vec3(Pad.rect.x + Pad.rotRight.x, Pad.rect.y + Pad.rotRight.y, 0.0f)) * Pad.matrix;

				Ball.matrix = glm::translate(glm::vec3(Ball.rect.x, Ball.rect.y, 0.0f)) * Ball.matrix;
				BallLocMat = glm::inverse(Pad.matrix) * Ball.matrix;

				objPad.sdlrect.x = 0 - (int)Pad.rotRight.x;
				objPad.sdlrect.y = 0 - (int)Pad.rotRight.y;

				objBallLoc.sdlrect.x = (int)BallLocMat[3].x;
				objBallLoc.sdlrect.y = (int)BallLocMat[3].y;



				BallPadCollision(&objBallLoc, &objPad, &NoCollisionCount);


				glm::mat4 BallNewWorld = Pad.matrix * BallLocMat;


				Ball.vect.x = objBallLoc.vect.x;
				Ball.vect.y = objBallLoc.vect.y;
				/*
				//NOTE: If pad is not inclined
				Pad.matrix = glm::mat4();
				Pad.matrix = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * Pad.matrix;
				//cout <<"\n\nRotated:\n"<< glm::to_string(Pad.matrix) << endl;
				Pad.matrix = glm::translate(glm::vec3((float)Pad.rect.x, (float)Pad.rect.y, 0.0f)) + Pad.matrix;
				BallPadCollision(&Ball, &Pad, &NoCollisionCount);
				*/
			}

			else if (Pad.lBend > 0 && Pad.rBend == 0)
			{
				Pad.matrix = glm::mat4();
				Ball.matrix = glm::mat4();

				Pad.matrix = glm::rotate(glm::radians(Pad.lBend), glm::vec3(0.0f, 0.0f, 1.0f)) * Pad.matrix;
				Pad.matrix = glm::translate(glm::vec3(Pad.rect.x + Pad.rotLeft.x, Pad.rect.y + Pad.rotLeft.y, 0.0f)) * Pad.matrix;
				Ball.matrix = glm::translate(glm::vec3(Ball.rect.x, Ball.rect.y, 0.0f)) * Ball.matrix;
				BallLocMat = glm::inverse(Pad.matrix) * Ball.matrix;

				objPad.rect.x = 0 - Pad.rotLeft.x;
				objPad.rect.y = 0 - Pad.rotLeft.y;
				objBallLoc.rect.x = BallLocMat[3].x;
				objBallLoc.rect.y = BallLocMat[3].y;
				BallPadCollision(&objBallLoc, &objPad, &NoCollisionCount);


				glm::mat4 BallNewWorld = Pad.matrix * BallLocMat;
				Ball.vect.x = objBallLoc.vect.x;
				Ball.vect.y = objBallLoc.vect.y;

			}


			//NOTE: anticlockwise rotation collision
			else if (Pad.rBend > 0 && Pad.lBend == 0)
			{


				Pad.matrix = glm::mat4();
				Ball.matrix = glm::mat4();

				Pad.matrix = glm::rotate(glm::radians(-Pad.rBend), glm::vec3(0.0f, 0.0f, 1.0f)) * Pad.matrix;
				Pad.matrix = glm::translate(glm::vec3(Pad.rect.x + Pad.rotRight.x, Pad.rect.y + Pad.rotRight.y, 0.0f)) * Pad.matrix;

				Ball.matrix = glm::translate(glm::vec3(Ball.rect.x, Ball.rect.y, 0.0f)) * Ball.matrix;
				BallLocMat = glm::inverse(Pad.matrix) * Ball.matrix;

				objPad.rect.x = 0 - Pad.rotRight.x;
				objPad.rect.y = 0 - Pad.rotRight.y;

				objBallLoc.rect.x = BallLocMat[3].x;
				objBallLoc.rect.y = BallLocMat[3].y;


				BallPadCollision(&objBallLoc, &objPad, &NoCollisionCount);


				glm::mat4 BallNewWorld = Pad.matrix * BallLocMat;


				Ball.vect.x = objBallLoc.vect.x;
				Ball.vect.y = objBallLoc.vect.y;

			}

			Ball.rect.x += Ball.vect.x;
			Ball.rect.y += Ball.vect.y;

		}

		Ball.vect.x *= 4.0f;
		Ball.vect.y *= 4.0f;
		
		if(NoCollisionCount > 0 )
		cout << NoCollisionCount << "\n";
	
		//NOTE: Screen limit collisions
		if (Pad.rect.x < 0.0f)
		{
			Pad.rect.x = 0.0f;
			Pad.vect.x = 0.0f;
			//Pad.rect.x = SCREEN_WIDTH - Pad.rect.w;
		}
			
		if (Pad.rect.x > (1280 - Pad.sdlrect.w))
		{
			Pad.rect.x = 1280 - (float)Pad.sdlrect.w;
			Pad.vect.x = 0.0f;
			//Pad.rect.x = 0 + Pad.rect.w;
		}
		if ((Ball.rect.x + Ball.sdlrect.w) > 1280)
		{
			Ball.rect.x -= Ball.vect.x;
			Ball.vect.x *= -1;
			NoCollisionCount = 0;
		}

		if ((Ball.rect.x) < 0.0f)
		{
			Ball.rect.x -= Ball.vect.x;
			Ball.vect.x *= -1;
			NoCollisionCount = 0;
		}
		
		if ((Ball.rect.y + Ball.sdlrect.h) > 740)
		{
			
			Ball.rect.y -= Ball.vect.y;
			Ball.vect.y *= -1;
			NoCollisionCount = 0;
			
		}
		
		
		if (Ball.rect.y < 0)
		{
			Ball.rect.y -= Ball.vect.y;
			Ball.vect.y *= -1;
			NoCollisionCount = 0;
		}

		//NOTE: Wall bricks collisions
		
		for (int i = 0; i < NUM_BLOCKS; i++)
		{
			if (Bricks[i].energy > 0)
			{
				SDL_bool hit = BallPadCollision(&Ball, &Bricks[i], &NoCollisionCount);
				if (hit)
				{
					Bricks[i].energy -= 1;
				}
			}
		}


		//NOTE: Gravity test code
		//Ball.vect.y += 0.4f;

		//NOTE: Rendering all the things! \o	

		//NOTE: Render text

		//string myStr = "Original Ball matrix  "+ glm::to_string(Ball.matrix);
		string myStr = "Pad.vect.x:  " + to_string(Pad.vect.x) + " - xValue: " + to_string(xValue) + " xjoy: " + to_string(xjoy) + " NoCollisionCount: " + to_string(NoCollisionCount);
		SDL_Surface *textSurface = TTF_RenderText_Solid(font, myStr.c_str(), color);
		SDL_Texture *text = SDL_CreateTextureFromSurface(Renderer, textSurface);
		SDL_Rect textRect;
		textRect.x = textRect.y = 0;
		SDL_QueryTexture(text, NULL, NULL, &textRect.w, &textRect.h);

		myStr = "                Ball local  " + glm::to_string(BallLocMat);
		SDL_Surface *textSurface2 = TTF_RenderText_Solid(font, myStr.c_str(), color);
		SDL_Texture *text2 = SDL_CreateTextureFromSurface(Renderer, textSurface2);
		SDL_Rect textRect2;
		textRect2.y = 20;
		textRect2.x = 0,
 		SDL_QueryTexture(text2, NULL, NULL, &textRect2.w, &textRect2.h);
	
		float velocity = (glm::sqrt(glm::pow2(Ball.vect.x) + glm::pow2(Ball.vect.y)));
		myStr = "FPS: " + to_string((int)actualFPS) + "   Ball velocity: " + to_string((int)velocity) + " Ball VelY: " + to_string((int)Ball.rect.y);

		SDL_Surface *textSurface3 = TTF_RenderText_Solid(font, myStr.c_str(), color);
		SDL_Texture *text3 = SDL_CreateTextureFromSurface(Renderer, textSurface3);
		SDL_Rect textRect3;
		textRect3.y = 40;
		textRect3.x = 0;
		SDL_QueryTexture(text3, NULL, NULL, &textRect3.w, &textRect3.h);


		//Rendering objects

		Ball.positionToSDL();
		Pad.positionToSDL();
		
		SDL_Point center = {};
		
		SDL_RenderClear(Renderer);
		
		if ((Pad.lBend == 0.0f && Pad.rBend == 0.0f) || (Pad.lBend > 0 && Pad.rBend > 0) )
		{
			SDL_RenderCopy(Renderer, Pad.texture, NULL, &Pad.sdlrect);
		}
			
		else if (Pad.lBend > 0 && Pad.rBend == 0)
		{
			center.x = (int)Pad.rotLeft.x;
			center.y = (int)Pad.rotLeft.y;
			SDL_RenderCopyEx(Renderer, Pad.texture, NULL, &Pad.sdlrect, Pad.lBend, &center, SDL_FLIP_NONE);

		}
			
		else if (Pad.rBend > 0 && Pad.lBend == 0)
		{
			center.x = (int)Pad.rotRight.x;
			center.y = (int)Pad.rotRight.y;
			SDL_RenderCopyEx(Renderer, Pad.texture, NULL, &Pad.sdlrect, -Pad.rBend, &center, SDL_FLIP_NONE);

		}

		
		SDL_RenderCopy(Renderer, Ball.texture, NULL, &Ball.sdlrect);		
		
		
		for (int i = 0; i < NUM_BLOCKS; i++)
		{
			if (Bricks[i].energy > 0)
			{
				SDL_RenderCopy(Renderer, Bricks[0].texture, NULL, &Bricks[i].sdlrect);
			}
		}
		

		//NOTE: render debug for rotating pad
		glm::vec4 vett1 = glm::vec4();
		glm::vec4 vett2 = glm::vec4();

		


		//NOTE: Draw for the pivot centers
	
		SDL_Rect centers;
		if (DebugOnScreen)
		{
			SDL_SetRenderDrawColor(Renderer, 0, 0, 255, 255);
			SDL_RenderDrawRect(Renderer, &Pad.sdlrect);
			vett1.x = Pad.matrix[0].x*20.0f;
			vett1.y = Pad.matrix[0].y*20.0f *-1;

			vett2.x = Pad.matrix[1].x*20.0f;
			vett2.y = Pad.matrix[1].y*20.0f *-1;


			SDL_RenderDrawLine(Renderer, SCREEN_WIDTH / 2,
				SCREEN_HEIGHT / 2,
				(SCREEN_WIDTH / 2) + (int)vett1.x,
				(SCREEN_HEIGHT / 2) + (int)vett1.y);

			SDL_RenderDrawLine(Renderer, SCREEN_WIDTH / 2,
				SCREEN_HEIGHT / 2,
				(SCREEN_WIDTH / 2) + (int)vett2.x,
				(SCREEN_HEIGHT / 2) + (int)vett2.y);
			
			centers.x = (int)(Pad.sdlrect.x + Pad.rotLeft.x);
			centers.y = (int)(Pad.sdlrect.y + Pad.rotLeft.y);
			centers.w = 1;
			centers.h = 1;
			SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
			SDL_RenderFillRect(Renderer, &centers);
			centers.x = (int)(Pad.rect.x + Pad.rotRight.x);
			SDL_RenderFillRect(Renderer, &centers);

			

			
			objPad.sdlrect.x += (SCREEN_WIDTH / 2);
			objPad.sdlrect.y += (SCREEN_HEIGHT / 2);
			Ball2 = objBallLoc;
			Ball2.sdlrect.x = (int)BallLocMat[3].x + (SCREEN_WIDTH / 2);
			Ball2.sdlrect.y = (int)BallLocMat[3].y + (SCREEN_HEIGHT / 2);


			SDL_RenderDrawRect(Renderer, &Ball2.sdlrect);
			SDL_RenderDrawRect(Renderer, &objPad.sdlrect);
			SDL_RenderCopy(Renderer, text, NULL, &textRect);
			SDL_RenderCopy(Renderer, text2, NULL, &textRect2);
			SDL_RenderCopy(Renderer, text3, NULL, &textRect3);

		}
		SDL_SetRenderDrawColor(Renderer, 240, 240, 240, 240);
		SDL_RenderPresent(Renderer);
		
		
			SDL_DestroyTexture(text);
			SDL_DestroyTexture(text2);
			SDL_DestroyTexture(text3);

			SDL_FreeSurface(textSurface);
			SDL_FreeSurface(textSurface2);
			SDL_FreeSurface(textSurface3);
		
			
		if (1000 / FPS > SDL_GetTicks() - start)
		{
			//SDL_Delay(1000 / FPS - (SDL_GetTicks() - start));
		}
		actualFPS = 1000.0f/ (SDL_GetTicks() - start);
		
	};

	//NOTE: Clean up and quit

	TTF_CloseFont(font);
	font = NULL;
	

	SDL_JoystickClose(Controller); 
	Controller = NULL;
	SDL_DestroyWindow(Window);
	Window = NULL;
	SDL_DestroyRenderer(Renderer);
	Renderer = NULL;
	SDL_Quit();
	return 0;

}