/*This source code copyrighted by Lazy Foo' Productions (2004-2022)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h> 


#define PI 3.14159265
#define PLAYER_NUM 2
#define PLAYER_INFO 5
#define RING_OFFSET 60

//565 910 27 80
//Screen dimension constants
const double UPX = 84;
const double UPY = 31;
const double LEFT_GOAL_X = 44;
const double GOAL_UPY = 222;
const double GOAL_DOWNY = 366;
const double RIGHT_GOAL_X = 950;

const double DOWNY = 560;
const double DOWNX = 905;


const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600;
const int BALL_SIZE = 10;
const int PLAYER_SIZE = 40;
//KEYMAP
//p1
const SDL_Keycode P1_UP = SDLK_w;
const SDL_Keycode P1_DOWN = SDLK_s;
const SDL_Keycode P1_LEFT = SDLK_a;
const SDL_Keycode P1_RIGHT = SDLK_d;
const SDL_Keycode P1_KICK = SDLK_SPACE;
const SDL_Keycode P1_SWITCH = SDLK_r;
//p2
const SDL_Keycode P2_UP = SDLK_UP;
const SDL_Keycode P2_DOWN = SDLK_DOWN;
const SDL_Keycode P2_LEFT = SDLK_LEFT;
const SDL_Keycode P2_RIGHT = SDLK_RIGHT;
const SDL_Keycode P2_KICK = SDLK_PERIOD;
const SDL_Keycode P2_SWITCH = SDLK_m;

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
	zilch = 999
};
//64 ticks 
const double COLLIDE_VELOCITY = 25;
const double KICK_AREA = (BALL_SIZE + PLAYER_SIZE)/2;
const double COLLIDE_SIZE = (BALL_SIZE + PLAYER_SIZE * RING_OFFSET/100)/2;
const double KICK_POWER = 200;
const double SERVER_TICK = 1000 / 640;
const double TICK_PER_SEC = 64;
const double ballVMax = 200;
const double ballFriction = 4;
const double PlayerFriction = 11;
const double PlayerVMax = 25;
const double PlayerAcceleration = 10;
const double frictionThreshold = 85;

std::string dir;
//Image pathing
const std::string circle = "ball.png";
const std::string circle_ring = "circle_ring.bmp";
const std::string bg = "field.png";
const std::string windImg = "wind.png";
const std::string scoreImg = "score.png";

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;
bool kick = false;
bool isGoal = false;
bool leftTurn = true;
int score[2] = { 0,0 };
int leftControl = 1;
int rightControl = 0;
double wind = 0;
double windVelocity = 80;
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
	void render(int x, int y,  SDL_Rect* clip,double angle = 0);

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

void LTexture::render(int x, int y,  SDL_Rect* clip ,double angle)
{
	//Set rendering space and render to screen

	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	SDL_RendererFlip flipType = SDL_FLIP_NONE;
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, NULL,flipType);
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
	int count = 0;

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
			if (this->velocity > frictionThreshold)
			{


				if (count > 1)
				{
					double nV = 0;
					double nD = 0;
					sumVector(this->velocity, this->vDirection, windVelocity / TICK_PER_SEC, wind, nV, nD);
					this->setVDirection(nD);
					this->setVelocity(nV);
					count = 0;
				}
				else
				{
					count++;
				}
				
			}
			else
			{
				count = 0;
			}
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
		if (this->velocity < this->vMax * 50 / 100)
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
				value = sqrt(v1 * v1 + v2 * v2 - 2 * v1 * v2 * cos((180 - degree)*PI/180));
				if (vD1 > vD2)
				{
					double nDegree = asin(v1 * sin((180 - degree) * PI / 180) / value) * 180 / PI;
					if (degree <= 90) {
						direction = vD2 + nDegree;
					}
					else {
						direction =  vD2+  nDegree +90;

					}
					
				}
				else
				{
					double nDegree = asin(v2 * sin((180 - degree) * PI / 180) / value) * 180 / PI;
					direction = vD1 + nDegree;
				}
				

			}
			else
			{
				degree = 360 - degree;
				
				value = sqrt(v1 * v1 + v2 * v2 - 2 * v1 * v2 * cos((180 - degree) * PI / 180));
				if (vD1 > vD2)
				{
					double nDegree = asin(v2 * sin((180 - degree) * PI / 180) / value) * 180 / PI;

					direction = vD1 + nDegree;
				}
				else
				{
					double nDegree = asin(v1 * sin((180 - degree) * PI / 180) / value) * 180 / PI;

					direction = vD1 + nDegree;
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

	void setVelocity(double v)
	{
		this->vector.setVelocity(v);
	}


	void update(double arr[PLAYER_NUM*2][PLAYER_INFO])
	{
		this->angle = this->vector.getVDirection();
		double nX = this->getX();
		double nY = this->getY();
		this->vector.update(nX, nY);
		
		collide(arr);
			
		this->setPos(nX, nY);
		
		wallBounce();
		if (kick)
		{
			
			for (int i = 0; i < PLAYER_NUM*2; i++)
			{
				if (arr[i][4] == 1 and sqrt((arr[i][0] - this->x) * (arr[i][0] - this->x) + (arr[i][1] - this->y) * (arr[i][1] - this->y)) < KICK_AREA ) {

					kick = false;
					this->vector.setVelocity(KICK_POWER);
					this->vector.setVDirection(arr[i][3]);
				}
			}
		}
		this->texture.render(this->x - BALL_SIZE/2, this->y - BALL_SIZE/2, NULL, this->angle);
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
		if (((this->x > DOWNX and this->x < RIGHT_GOAL_X)) and (this->y < GOAL_DOWNY and this->y > GOAL_UPY) and !isGoal)
		{
			score[0]++;
			leftTurn = false;
			isGoal = true;
		}
		if (((this->x < UPX and this->x > LEFT_GOAL_X)) and (this->y < GOAL_DOWNY and this->y > GOAL_UPY) and !isGoal)
		{
			score[1]++;
			leftTurn = true;
			isGoal = true;
		}
		if (this->y < UPY)
		{
			this->y = UPY;
			double boundDirection = direction < 90 ? 180 - direction : 540 - direction ;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 4);
			return;
		}
		else if (this->y > DOWNY)
		{
			this->y = DOWNY;
			double boundDirection = direction < 180 ? 180 - direction : 540 - direction;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 4);
			return;
		}
		else if (((this->x < UPX and this->x > LEFT_GOAL_X) or (this->x > DOWNX and this->x < RIGHT_GOAL_X)) and this->y < GOAL_UPY and this->y > GOAL_UPY - 4)
		{
			this->y = GOAL_UPY;
			double boundDirection = direction < 90 ? 180 - direction : 540 - direction;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 20);
			return;
		}
		else if (((this->x < UPX and this->x > LEFT_GOAL_X) or (this->x > DOWNX and this->x < RIGHT_GOAL_X)) and this->y > GOAL_DOWNY and this->y < GOAL_DOWNY + 4)
		{

			this->y = GOAL_DOWNY;
			double boundDirection = direction < 180 ? 180 - direction : 540 - direction;
			vector.setVDirection(boundDirection);
			vector.setVelocity(vector.getVelocity() / 20);
			return;
		}
		if (this->x > DOWNX and (this->y > GOAL_DOWNY or this->y < GOAL_UPY))
		{
			
			this->x = DOWNX;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 4);
			return;
		}
		else if (this->x < UPX and (this->y > GOAL_DOWNY or this->y < GOAL_UPY) )
		{
			this->x = UPX;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 4);
			return;
		}
		else if (this->x > RIGHT_GOAL_X)
		{
			
			this->x = RIGHT_GOAL_X;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 20);
			return;
		}
		else if (this->x < LEFT_GOAL_X)
		{
			this->x = LEFT_GOAL_X;
			vector.setVDirection(direction - 360);
			vector.setVelocity(vector.getVelocity() / 20);
			return;
		}

		
	}

	bool collide(double arr[PLAYER_NUM*2][PLAYER_INFO])
	{
		
		for (int i = 0; i < PLAYER_NUM*2; i++)
		{
			
			double dX = (this->x - arr[i][0]);
			double dY = (this->y - arr[i][1]);
			double distance = sqrt(dX * dX + dY * dY);
			
			if (distance < COLLIDE_SIZE)
			{

				this->vector.setVelocity(COLLIDE_VELOCITY);
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
	aDirect move = zilch;
	bool inControl = false;
	bool isSprint = false;
	bool isAim = false;
	bool kicked = false;
	bool drifted = false;
	aDirect kickDirection = zilch;

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

	void update(double x, double y, double a[PLAYER_NUM][PLAYER_INFO], int idx)
	{

		double nX = this->getX();
		double nY = this->getY();
		this->collide(x, y,a, idx);
		this->vector.update(nX, nY);
		this->setPos(nX, nY);
		
		boundaryCheck();

		this->makeMove();
		this->texture.render(this->x - PLAYER_SIZE/2, this->y - PLAYER_SIZE / 2,NULL ,this->angle);
	}

	void loadImg()
	{
		this->texture.loadFromFile(circle_ring);
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
	
	void setVelocity(double v)
	{
		this->vector.setVelocity(v);
	}
	void setAcceleration(double a)
	{
		this->vector.setAcceleration(a);
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
		
		if (this->x < UPX - 40) this->x = UPX - 40;
		if (this->x > DOWNX + 40) this->x = DOWNX + 40;
		if (this->y < 0) this->y = 0;
		if (this->y > DOWNY + 40) this->y = DOWNY + 40;
	}

	void makeMove()
	{
		
		if (isAim) return;
		if (this->move == zilch)
		{
			this->vector.setAcceleration(0);
			this->vector.setFriction(true);
			return;
		}
		this->angle = this->vector.getVDirection();
		this->vector.setAcceleration(PlayerAcceleration);
		this->vector.setADirection(this->move);
		this->vector.setFriction(false);
	}

	void buttonDownLeft(SDL_Keycode k, double x, double y)
	{



		switch (k)
		{
		case P1_SWITCH:
			leftControl = leftControl == 0 ? 1 : 0;
			this->move = zilch;
			if (isAim)
			{
				kick = true;
				isAim = false;
				kicked = true;
			}
			break;
		case P1_LEFT:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aA;
			if (this->move == aD) this->move = aA;
			if (this->move == aWD) this->move = aWA;
			if (this->move == aSD) this->move = aSA;
			if (this->move == aS) this->move = aSA;
			if (this->move == aW) this->move = aWA;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_RIGHT:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aD;
			if (this->move == aA) this->move = aD;
			if (this->move == aWA) this->move = aWD;
			if (this->move == aSA) this->move = aSD;
			if (this->move == aW) this->move = aWD;
			if (this->move == aS) this->move = aSD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_UP:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aW;
			if (this->move == aS) this->move = aW;
			if (this->move == aSA) this->move = aWA;
			if (this->move == aSD) this->move = aWD;
			if (this->move == aA) this->move = aWA;
			if (this->move == aD) this->move = aWD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_DOWN:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aS;
			if (this->move == aW) this->move = aS;
			if (this->move == aWA) this->move = aSA;
			if (this->move == aWD) this->move = aSD;
			if (this->move == aA) this->move = aSA;
			if (this->move == aD) this->move = aSD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_KICK:
			std::cout << this->x << "||" << this->y << std::endl;
			if (kicked) kicked = false;
			if (!this->isSprint and !this->kicked and !this->drifted)
			{
				
				this->aimKick( x,  y);
			}
		default:
			break;
		}

	}
	void buttonUpLeft(SDL_Keycode k)
	{
		switch (k)
		{
		case P1_LEFT:
			if (this->move == aA) this->move = zilch;
			if (this->move == aWA) this->move = aW;
			if (this->move == aSA) this->move = aS;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_RIGHT:
			if (this->move == aD) this->move = zilch;
			if (this->move == aWD) this->move = aW;
			if (this->move == aSD) this->move = aS;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_UP:
			if (this->move == aW) this->move = zilch;
			if (this->move == aWA) this->move = aA;
			if (this->move == aWD) this->move = aD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_DOWN:
			if (this->move == aS) this->move = zilch;
			if (this->move == aSA) this->move = aA;
			if (this->move == aSD) this->move = aD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P1_KICK:
			if (isAim)
			{
				kick = true;
				isAim = false;
				kicked = true;
			}
		default:
			break;
		}
		std::cout << kickDirection;

	}
	void buttonDownRight(SDL_Keycode k, double x, double y)
	{

		switch (k)
		{
		case P2_SWITCH:
			rightControl = rightControl == 0 ? 1 : 0;
			this->move = zilch;
			if (isAim)
			{
				kick = true;
				isAim = false;
				kicked = true;
			}
			break;
		case P2_LEFT:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aA;
			if (this->move == aD) this->move = aA;
			if (this->move == aWD) this->move = aWA;
			if (this->move == aSD) this->move = aSA;
			if (this->move == aS) this->move = aSA;
			if (this->move == aW) this->move = aWA;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_RIGHT:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aD;
			if (this->move == aA) this->move = aD;
			if (this->move == aWA) this->move = aWD;
			if (this->move == aSA) this->move = aSD;
			if (this->move == aW) this->move = aWD;
			if (this->move == aS) this->move = aSD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_UP:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aW;
			if (this->move == aS) this->move = aW;
			if (this->move == aSA) this->move = aWA;
			if (this->move == aSD) this->move = aWD;
			if (this->move == aA) this->move = aWA;
			if (this->move == aD) this->move = aWD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_DOWN:
			if (kicked) kicked = false;
			if (this->move == zilch) this->move = aS;
			if (this->move == aW) this->move = aS;
			if (this->move == aWA) this->move = aSA;
			if (this->move == aWD) this->move = aSD;
			if (this->move == aA) this->move = aSA;
			if (this->move == aD) this->move = aSD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_KICK:
			std::cout << this->x << "||" << this->y << std::endl;
			if (kicked) kicked = false;
			if (!this->isSprint and !this->kicked and !this->drifted)
			{

				this->aimKick(x, y);
			}
		default:
			break;
		}

	}
	void buttonUpRight(SDL_Keycode k)
	{
		switch (k)
		{
		case P2_LEFT:
			if (this->move == aA) this->move = zilch;
			if (this->move == aWA) this->move = aW;
			if (this->move == aSA) this->move = aS;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_RIGHT:
			if (this->move == aD) this->move = zilch;
			if (this->move == aWD) this->move = aW;
			if (this->move == aSD) this->move = aS;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_UP:
			if (this->move == aW) this->move = zilch;
			if (this->move == aWA) this->move = aA;
			if (this->move == aWD) this->move = aD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_DOWN:
			if (this->move == aS) this->move = zilch;
			if (this->move == aSA) this->move = aA;
			if (this->move == aSD) this->move = aD;
			kickDirection = this->move == zilch ? kickDirection : move;
			this->angle = kickDirection;
			break;
		case P2_KICK:
			if (isAim)
			{
				kick = true;
				isAim = false;
				kicked = true;
			}
		default:
			break;
		}


	}
	void collide(double x, double y, double a[PLAYER_NUM][PLAYER_INFO], int idx)
	{
		double dX = (this->x - x);
		double dY = (this->y -y);
		double distance = sqrt(dX * dX + dY * dY);

		if (distance < COLLIDE_SIZE)
		{
			this->vector.setVelocity(this->vector.getVelocity()*3/10);
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


		}
		for (int i = 0; i < PLAYER_NUM*2; i++)
		{
			if (i == idx) continue;
			double dX = (this->x - a[i][0]);
			double dY = (this->y - a[i][1]);
			double distance = sqrt(dX * dX + dY * dY);

			if (distance < COLLIDE_SIZE)
			{
				this->vector.setVelocity(this->vector.getVelocity() * 3 / 10);
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


			}
		}

	}

	double getVelocity()
	{
		return this->vector.getVelocity();
	}

	double getKickDirection() 
	{
		return this->kickDirection;
	}

	bool isKicked()
	{
		return this->kicked;
	}

	void aimKick(double x, double y)
	{
		double dX = (this->x - x);
		double dY = (this->y - y);
		double distance = sqrt(dX * dX + dY * dY);
		if (distance < KICK_AREA) {
			this->isAim = true;
			this->vector.setVelocity(0);
			this->vector.setAcceleration(0);
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
Player players[PLAYER_NUM*2];
LTexture windSock;
LTexture scoreDisplay;
SDL_Rect scoreClip = {50*10,0,50,94};
LTexture scoreDisplay1;
SDL_Rect scoreClip1 = { 50 * 10,0,50,94 };
LTexture colon;
SDL_Rect colonClip = { 584,0,50,94 };
double playerPos[PLAYER_NUM*2][PLAYER_INFO];

void matchInit(Player p[])
{
	for (int i = 0; i < PLAYER_NUM*2; i++)
	{


		if (i >= PLAYER_NUM)
		{
			p[i].setAcceleration(0);
			p[i].setVelocity(0);
			p[i].setAngle(270);
		}
		else {
			p[i].setAcceleration(0);
			p[i].setVelocity(0);
			p[i].setAngle(90);
		}
	}
	
	p[0].setPos((LEFT_GOAL_X + RIGHT_GOAL_X - 160) / 2, (GOAL_DOWNY + GOAL_UPY + 4) / 2);
	p[1].setPos((LEFT_GOAL_X + 50) , (GOAL_DOWNY + GOAL_UPY + 4) / 2);
	p[2].setPos((LEFT_GOAL_X + RIGHT_GOAL_X + 160) / 2, (GOAL_DOWNY + GOAL_UPY + 4) / 2);
	p[3].setPos((RIGHT_GOAL_X - 50), (GOAL_DOWNY + GOAL_UPY + 4) / 2);
	ball.setVelocity(0);
	ball.setPos((LEFT_GOAL_X + RIGHT_GOAL_X-3)/2, (GOAL_DOWNY + GOAL_UPY+4) / 2);

}


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
	for (int i = 0; i < PLAYER_NUM*2; i++)
	{
		players[i].loadImg();
		players[i].setDimension(PLAYER_SIZE, PLAYER_SIZE);
	}

	//ball.getTexture().setDimension(100, 100);
	ball.setDimension(BALL_SIZE, BALL_SIZE);
	
	//Loading success flag
	bool success = true;

	//Load Foo' texture

	if (!windSock.loadFromFile(windImg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}
	if (!gBackgroundTexture.loadFromFile(bg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}
	if (!scoreDisplay.loadFromFile(scoreImg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}
	if (!scoreDisplay1.loadFromFile(scoreImg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}
	if (!colon.loadFromFile(scoreImg))
	{
		printf("Failed to load background texture image!\n");
		success = false;
	}
	//gBackgroundTexture.setDimension(1200, 700);
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
	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[])
{
	
	dir = args[0];
	dir = dir.substr(0, dir.length() - 18);
	Uint32 start = 0;
	Uint32 delay = 0;
	Uint32 windDelay = 0;
	//Start up SDL and create window
	matchInit(players);
	srand(time(NULL));
	

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
			scoreDisplay.setWidth(80);
			scoreDisplay1.setWidth(80);
			colon.setWidth(80);
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{
				if (SDL_GetTicks() - windDelay > 20000) {
					windDelay = SDL_GetTicks();
					wind = fmod((wind + 90),360);
				}
				if ( SDL_GetTicks()-start > SERVER_TICK)
				{
					start = SDL_GetTicks();
					
					//Clear screen
					SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
					SDL_RenderClear(gRenderer);

					//Render background texture to screen
					gBackgroundTexture.render(0, 0, NULL);
					colon.render(1100, 100, &colonClip);
					scoreDisplay.render(950, 100, &scoreClip);
					colon.render(1020, 100, &colonClip);
					scoreDisplay1.render(1080, 100, &scoreClip1);
					scoreClip.x = (score[0]*50);
					scoreClip1.x = (score[1] * 50);
					windSock.render(1000, 400, NULL, wind);
					//Render Foo' to the screen
					//gFooTexture.render(70, 90);
					//ball.setPos(333, 111);
					if (delay == 0 and isGoal) {
						delay = SDL_GetTicks();
					}
					if (SDL_GetTicks() - delay > 2000 and isGoal)
					{
						matchInit(players);
						isGoal = false;
						delay = 0;
						std::cout << score[0] << "-" << score[1];
						
					}
					ball.update(playerPos);
					
					for (int i = 0; i < PLAYER_NUM*2; i++)
					{

					players[i].update(ball.getX(), ball.getY(), playerPos, i);
					playerPos[i][0] = players[i].getX();
					playerPos[i][1] = players[i].getY();
					playerPos[i][2] = players[i].getMove() != zilch ? 30 : 8;
					playerPos[i][3] = players[i].getKickDirection();
					playerPos[i][4] = players[i].isKicked() ? 1 : 0;
						


					};
					
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

						players[leftControl].buttonDownLeft(e.key.keysym.sym, ball.getX(), ball.getY());
						players[rightControl+PLAYER_NUM].buttonDownRight(e.key.keysym.sym, ball.getX(), ball.getY());


					} else if (e.key.state == SDL_RELEASED)
					{

						players[leftControl].buttonUpLeft(e.key.keysym.sym);
						players[rightControl+ PLAYER_NUM].buttonUpRight(e.key.keysym.sym);
					}
				}


			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}