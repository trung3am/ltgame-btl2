/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#define PI 3.14159265
#define PLAYER_NUM 1
//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int BALL_SIZE = 30;
const int PLAYER_SIZE = 55;
enum aDirect
{
	aW = 0,
	aWD = 45,
	aD = 90,
	aSD = 135,
	aS = 180,
	aSA = 225,
	aA = 270,
	aWA = 315,
	null = 400
};
//64 ticks 
const double COLLIDE_SIZE = 30;
const double SERVER_TICK = 1000 / 640;
const double TICK_PER_SEC = 64;
const double ballVMax = 200;
const double ballFriction = 5;
const double PlayerFriction = 11;
const double PlayerVMax = 60;
const double PlayerAcceleration = 10;

std::string dir;
//Image pathing
const std::string circle = "circle.bmp";
const std::string bg = "hello_world.bmp";


//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);

	//Deallocates texture
	void free();

	//Renders texture at given point
	void render(int x, int y, double angle = 0);

	//Gets image dimensions
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);
	void setDimension(int w, int h);
private:
	//The actual hardware texture
	SDL_Texture* mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	path = dir + path;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::render(int x, int y, double angle)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	SDL_RendererFlip flipType = SDL_FLIP_NONE;
	SDL_RenderCopyEx(gRenderer, mTexture, NULL, &renderQuad, angle, NULL,flipType);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

void LTexture::setWidth(int w)
{
	this->mWidth = w;
}
void LTexture::setHeight(int h)
{
	this->mHeight = h;
}

void LTexture::setDimension(int w, int h) {
	this->setHeight(h);
	this->setWidth(w);
}
class Vector
{
private:
	double vDirection;
	double velocity;
	double acceleration;
	double aDirection;
	double friction;
	double vMax;
	bool isFriction;

public:
	Vector()
	{
		vDirection=0;
		velocity=0;
		acceleration=0;
		aDirection=0;
		friction=0;
		vMax=0;
		isFriction=0;
	}
	Vector(double f, double v, bool isF = true)
	{
		vDirection = 0;
		velocity = 0;
		acceleration = 0;
		aDirection = 0;
		friction = f;
		vMax = v;
		isFriction = isF;
	}
	double getVDirection()
	{
		return this->vDirection;
	}
	void setVDirection(double v)
	{
		this->vDirection = fmod(abs(v), 360);
	}
	double getVelocity()
	{
		return this->velocity;
	}
	void setVelocity(double v)
	{
		this->velocity = v > vMax ? vMax : v;
	}
	double getAcceleration()
	{
		return this->acceleration;
	}
	void setAcceleration(double a)
	{
		this->acceleration = a;
	}
	double getADirection()
	{
		return this->aDirection;
	}
	void setADirection(aDirect a)
	{
		this->aDirection = a;
	}
	double getFriction()
	{
		return this->friction;
	}
	void setFriction(bool f)
	{
		this->isFriction = f;
	}
	double getVMax()
	{
		return this->vMax;
	}
	void update(double& x, double& y)
	{
		double rad = this->vDirection == 0 ? 0 : this->vDirection * PI / 180;
		double mX = round(sin(rad)*1000);
		double mY = round(cos(rad)*1000);
		
		mX = abs(mX) < 10 ? 0 : mX/1000;
		mY = abs(mY) < 10 ? 0 : mY/1000;
		
		mX = mX == 0 ? 0 : mX * this->velocity / TICK_PER_SEC;
		mY = mY == 0 ? 0 : mY * this->velocity / TICK_PER_SEC;

		double nX = x + mX;
		double nY = y - mY;

		x = nX > 0 ? nX : 0;
		y = nY > 0 ? nY : 0;
		updateVelocity();
	}
	void updateVelocity()
	{
		if (this->isFriction)
		{
			this->velocity = this->velocity - friction/TICK_PER_SEC <= 0 ? 0 : this->velocity - friction / TICK_PER_SEC;
			return;
		}
		else {
			if (this->acceleration > 0)
			{
				accelerate();

			}
		}

	}
	
	void accelerate()
	{
		double a = this->acceleration / TICK_PER_SEC;
		double pen = this->velocity - friction*2 / TICK_PER_SEC <= 0 ? 0 : this->velocity - friction / TICK_PER_SEC;
		double temp;
		temp = abs(this->vDirection - this->aDirection);
		temp = temp > 180 ? 360 - temp : temp;
		if (this->velocity < this->vMax * 40 / 100)
		{
			setVDirection(this->aDirection);
		}
		else if (this->velocity < this->vMax * 70 / 100 and temp <= 90)
		{
			setVDirection(this->aDirection);
		}
		else if (this->velocity < this->vMax * 90 / 100 and temp <= 45)
		{
			setVDirection(this->aDirection);
		}
		temp = abs(this->vDirection - this->aDirection);
		if (temp==0)
		{
			setVelocity(this->velocity + a);
		}
		else {
			setVelocity(pen);
		}
	}

	void sumVector(double v1, double vD1, double v2, double vD2, double& value, double& direction)
	{

		if (abs(vD1 - vD2) < 1)
		{
			value = v1 + v2;
			direction = vD1;
		}
		else if (abs(vD1 - vD2 - 180) < 1)
		{
			double v = v1 - v2;
			value = v > 0 ? v : v2;
			direction = v > 0 ? vD1 : vD2;
		}
		else
		{
			double degree = abs(vD1 - vD2);

			if (degree < 180)
			{
				double value = sqrt(v1 * v1 + v2 * v2 - 2 * v1 * v2 * cos(180 - degree));
				if (vD1 > vD2)
				{
					double nDegree = asin(v1 * sin(180 - degree) / value) * 180 / PI;
					direction = vD2 + nDegree;
				}
				else
				{
					double nDegree = asin(v2 * sin(180 - degree) / value) * 180 / PI;
					direction = vD1 + nDegree;
				}

			}
			else
			{
				degree = 360 - degree;
				double value = sqrt(v1 * v1 + v2 * v2 - 2 * v1 * v2 * cos(180 - degree));
				if (vD1 > vD2)
				{
					double nDegree = asin(v1 * sin(180 - degree) / value) * 180 / PI;
					direction = vD1 + nDegree;
				}
				else
				{
					double nDegree = asin(v2 * sin(180 - degree) / value) * 180 / PI;
					direction = vD2 + nDegree;
				}
			}
		}
	}
};

class Ball
{
private:
	double x;
	double y;
	double angle;
	LTexture texture;
	Vector vector;

public:
	Ball()
	{
		vector = Vector(ballFriction, ballVMax);
		x = 0;
		y = 0;
		angle = 0.0;
		texture = LTexture();
		vector.setVelocity(100);
		vector.setVDirection(120);

	}

	~Ball()
	{
		this->free();
	}

	void free()
	{
		this->texture.free();
	}

	void update(double arr[PLAYER_NUM][2])
	{
		this->angle = this->vector.getVDirection();
		double nX = this->getX();
		double nY = this->getY();
		this->vector.update(nX, nY);
		
		collide(arr);
			
		this->setPos(nX, nY);
		
		wallBounce();
		this->texture.render(this->x - BALL_SIZE/2, this->y - BALL_SIZE/2, this->angle);
	}

	void loadImg()
	{
		this->texture.loadFromFile(circle);
	}

	double getX()
	{
		return this->x;
	}
	double  getY()
	{
		return this->y;
	}
	void setY(double y)
	{
		this->y = y;
	}
	void setX(double x)
	{
		this->x = x;
	}
	void setPos(double x, double y)
	{
		this->setX(x);
		this->setY(y);
	}

	void setAngle(double a)
	{
		this->angle = a;
	}

	double getAngle()
	{
		return this->angle;
	}

	LTexture getTexture()
	{
		return this->texture;
	}
	//Starts up SDL and creates window
	void setDimension(int w, int h)
	{
		this->texture.setDimension(w, h);
	}

	Vector getVector()
	{
		return this->vector;
	}

	void wallBounce()
	{
		double direction = vector.getVDirection();

		if (this->y < 50)
		{
			this->y = 50;

			double boundDirection = direction < 90 ? 180 - direction : 360 - direction;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 2);

		}
		else if (this->y > 500)
		{
			this->y = 500;
			double boundDirection = direction < 180 ? 180 - direction : 540 - direction;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 2);

		}
		if (this->x > 650)
		{
			this->x = 650;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 2);

		}
		else if (this->x < 50)
		{
			this->x = 50;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 2);

		}
	}

	bool collide(double arr[PLAYER_NUM][2])
	{
		
		for (int i = 0; i < PLAYER_NUM; i++)
		{
			
			double dX = (this->x - arr[i][0]);
			double dY = (this->y - arr[i][1]);
			double distance = sqrt(dX * dX + dY * dY);
			
			if (distance < COLLIDE_SIZE)
			{
				std::cout << distance;
				this->vector.setVelocity(30);
				double degreeXY = atan(abs(dX / dY)) * 180 / PI;
				double degreeYX = atan(abs(dY / dX)) * 180 / PI;
				if (dX >= 0 and dY >= 0)
				{
					this->vector.setVDirection(degreeYX + 90);
				}
				else if (dX <= 0 and dY >= 0)
				{
					this->vector.setVDirection(degreeXY + 180);
				}
				else if (dX <= 0 and dY <= 0)
				{
					this->vector.setVDirection(degreeYX + 270);
				}
				else
				{
					this->vector.setVDirection(degreeXY);
				}
				
				return true;
			}
		}

	}


};


class Player
{
private:
	double x;
	double y;
	double angle;
	LTexture texture;
	Vector vector;
	aDirect move = null;
	bool inControl = false;
public:
	Player()
	{
		vector = Vector(PlayerFriction, PlayerVMax);
		x = 0;
		y = 0;
		angle = 0.0;
		texture = LTexture();


	}

	~Player()
	{
		this->free();
	}

	void free()
	{
		this->texture.free();
	}

	void update(double x, double y)
	{
		this->angle = this->vector.getVDirection();
		double nX = this->getX();
		double nY = this->getY();
		this->collide(x, y);
		this->vector.update(nX, nY);
		this->setPos(nX, nY);
		this->collide(x, y);
		boundaryCheck();
		
		this->makeMove();
		this->texture.render(this->x - PLAYER_SIZE/2, this->y - PLAYER_SIZE / 2, this->angle);
	}

	void loadImg()
	{
		this->texture.loadFromFile(circle);
	}

	aDirect getMove()
	{
		return this->move;
	}

	double getX()
	{
		return this->x;
	}
	double  getY()
	{
		return this->y;
	}
	void setY(double y)
	{
		this->y = y;
	}
	void setX(double x)
	{
		this->x = x;
	}
	void setPos(double x, double y)
	{
		this->setX(x);
		this->setY(y);
	}

	void setAngle(double a)
	{
		this->angle = a;
	}

	double getAngle()
	{
		return this->angle;
	}

	LTexture getTexture()
	{
		return this->texture;
	}

	void setDimension(int w, int h)
	{
		this->texture.setDimension(w, h);
	}

	Vector getVector()
	{
		return this->vector;
	}

	void boundaryCheck()
	{
		if (this->x < 0) this->x = 0;
		if (this->x > 650) this->x = 650;
		if (this->y < 0) this->y = 0;
		if (this->y > 650) this->y = 650;
	}

	void makeMove()
	{
		if (this->move == null)
		{
			this->vector.setAcceleration(0);
			this->vector.setFriction(true);
			return;
		}

		this->vector.setAcceleration(PlayerAcceleration);
		this->vector.setADirection(this->move);
		this->vector.setFriction(false);
	}

	void buttonDown(SDL_Keycode k)
	{
		switch (k)
		{
		case SDLK_a:
			if (this->move == null) this->move = aA;
			if (this->move == aD) this->move = aA;
			if (this->move == aWD) this->move = aWA;
			if (this->move == aSD) this->move = aSA;
			if (this->move == aS) this->move = aSA;
			if (this->move == aW) this->move = aWA;
			break;
		case SDLK_d:
			if (this->move == null) this->move = aD;
			if (this->move == aA) this->move = aD;
			if (this->move == aWA) this->move = aWD;
			if (this->move == aSA) this->move = aSD;
			if (this->move == aW) this->move = aWD;
			if (this->move == aS) this->move = aSD;
			break;
		case SDLK_w:
			if (this->move == null) this->move = aW;
			if (this->move == aS) this->move = aW;
			if (this->move == aSA) this->move = aWA;
			if (this->move == aSD) this->move = aWD;
			if (this->move == aA) this->move = aWA;
			if (this->move == aD) this->move = aWD;
			break;
		case SDLK_s:
			if (this->move == null) this->move = aS;
			if (this->move == aW) this->move = aS;
			if (this->move == aWA) this->move = aSA;
			if (this->move == aWD) this->move = aSD;
			if (this->move == aA) this->move = aSA;
			if (this->move == aD) this->move = aSD;
			break;

		default:
			break;
		}
	}
	void buttonUp(SDL_Keycode k)
	{
		switch (k)
		{
		case SDLK_a:
			if (this->move == aA) this->move = null;
			if (this->move == aWA) this->move = aW;
			if (this->move == aSA) this->move = aS;
			break;
		case SDLK_d:
			if (this->move == aD) this->move = null;
			if (this->move == aWD) this->move = aW;
			if (this->move == aSD) this->move = aS;
			break;
		case SDLK_w:
			if (this->move == aW) this->move = null;
			if (this->move == aWA) this->move = aA;
			if (this->move == aWD) this->move = aD;
			break;
		case SDLK_s:
			if (this->move == aS) this->move = null;
			if (this->move == aSA) this->move = aA;
			if (this->move == aSD) this->move = aD;
			break;

		default:
			break;
		}
	}
	bool collide(double x, double y)
	{
		double dX = (this->x - x);
		double dY = (this->y -y);
		double distance = sqrt(dX * dX + dY * dY);

		if (distance < COLLIDE_SIZE)
		{
			std::cout << distance;
			this->vector.setVelocity(this->vector.getVelocity()*3/10);
			

			return true;
		}
	}
};



bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();


//Scene textures
LTexture gFooTexture;
LTexture gBackgroundTexture;
Ball ball;
Player p1;
Player arr[1];
double playerPos[PLAYER_NUM][2];




bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	ball.loadImg();
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
	{
		arr[i].loadImg();
		arr[i].setDimension(PLAYER_SIZE, PLAYER_SIZE);
	}
	//ball.getTexture().setDimension(100, 100);
	ball.setDimension(BALL_SIZE, BALL_SIZE);
	
	//Loading success flag
	bool success = true;

	//Load Foo' texture
	if (!gFooTexture.loadFromFile(circle))
	{
		printf("Failed to load Foo' texture image!\n");
		success = false;
	}
	gFooTexture.setDimension(33,33);
	printf(std::to_string(gFooTexture.getHeight()).c_str());
	//Load background texture
	if (!gBackgroundTexture.loadFromFile(bg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gFooTexture.free();
	gBackgroundTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	ball.free();
	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	dir = args[0];
	dir = dir.substr(0, dir.length() - 18);
	Uint32 start = 0;
	//Start up SDL and create window
	ball.setPos(300, 300);
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{
				if ( SDL_GetTicks()-start > SERVER_TICK)
				{
					start = SDL_GetTicks();
					
					//Clear screen
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					//Render background texture to screen
					gBackgroundTexture.render(0, 0);

					//Render Foo' to the screen
					//gFooTexture.render(70, 90);
					//ball.setPos(333, 111);
					
					
					for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
					{
						arr[i].update(ball.getX(),ball.getY());
						playerPos[i][0] = arr[i].getX();
						playerPos[i][1] = arr[i].getY();
					};
					ball.update(playerPos);
					//Update screen
					SDL_RenderPresent(gRenderer);
				}

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
						
					}
					else if (e.key.state == SDL_PRESSED)
					{
						for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
						{
							arr[i].buttonDown(e.key.keysym.sym);
						}


					} else if (e.key.state == SDL_RELEASED)
					{

						for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
						{
							arr[i].buttonUp(e.key.keysym.sym);
						}
					}
				}


			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}