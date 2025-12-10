// Game.cpp : This file contains the 'main' function. Program execution begins and ends there.

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "GamesEngineeringBase.h"
#include <time.h>
#include <thread>
#include <chrono>
#include <string>
#include <fstream>
#include <windows.h>

using namespace GamesEngineeringBase;

void timingus(size_t us) {
	auto start = std::chrono::system_clock::now();
	while (true) {
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		if (duration.count() > us) break;
	}
}

int camX = 1024, camY = 768;//location of cam
int interval = 0;//interval of changing frame
int level = 1;
class Animation
{
public:
	Image image;
	int frames;
	int index;
	int speed;
	Animation()
	{
		frames = 0;//frames num of the animation
		index = 0;//index of the current frame
		speed = 1;
	}
};
class Character
{
protected:

	int x, y;
	int spriteNum;
	
	//int pre_state;
	int width;
	int height;
	
	float speed;

public:

	Animation ani[10];
	int state;
	bool isLeft;
	int HP;
	int attackInterval;
	int damage;
	int attaTimer;
	bool isCollide;
	bool isHurt;

	Character() {
		state = 0;
		isLeft = false;
		isCollide = false;
		isHurt = false;
		attaTimer = 0;
	}
	Character(int _x, int _y, std::string filename) {
		
		state = 0;
		isLeft = false;
		isCollide = false;
		isHurt = false;
		attaTimer = 0;

	}
	void SetState(int new_state)
	{
		if (new_state != state)
			ani[state].index = 0;
		state = new_state;
		//std::cout <<"state"<< state << std::endl;
	}
	void update(int _x, int _y) {
		if (isCollide == false)
		{
			x += _x;
			y += _y;
		}
	}
	void toggleCollision(bool to)
	{
		isCollide = to;
	}
	int getX() { return x; }
	int getY() { return y;  }
	int getSpeed() { return speed; }
	int getWidth() { return width; }
	int getHeight() { return height; }
	bool getcollision() { return isCollide; }

	void setX(int _x) { x = _x; };
	void setY(int _y) { y = _y; };

	/*Character* getClassName()
	{
		return this;
	}*/
	bool collide(Character& c) {
		int dx = (x + width / 2) - (c.x + c.getWidth() / 2);
		int dy = (y + height / 2) - (c.y + c.getHeight() / 2);

		// this should be set in constructor
		float radius = static_cast<float>(height / 2.5 + c.getHeight() / 2.5);
		float d = sqrtf(dx * dx + dy * dy); 
		return d < radius;
	}
	void unitVector(float& a, float& b, Character& h)
	{
		float A = (h.getX() + h.getWidth() / 2) - (x + width / 2);
		float B = (h.getY() + h.getHeight() / 2) - (y + height / 2);
		float AB = sqrtf(A * A + B * B);
		a = A / AB;
		b = B / AB;
	}
};

class Hero : public Character {
public:
	bool direct[4];//if the direction is blocked
	int hurt_time;
	float cooldown1;
	float cooldown2;
	float AOERange;
	int N;
	int AOE_damage;
	int killed;
	int powerup_time;
	int score;
	bool ispowerup;

	Hero(int _x, int _y, std::string filename) : Character(_x, _y, filename) { 
		speed = 500; HP = 100; spriteNum = 10; attackInterval = 1.0f; hurt_time = 0; 
		cooldown1 = 0.5f; cooldown2 = 300; AOERange = 134; N = 3; AOE_damage = 5; 
		killed = 0; powerup_time = 0; score = 0; ispowerup = false;
		for (int i = 0; i < spriteNum; i++)
		{
			//std::cout << filename + "/sprite" + std::to_string(i + 1) +".png" << std::endl;
			ani[i].image.load(filename + "/sprite" + std::to_string(i) + ".png");
			switch (i) {
			case 0:
			case 1:
			case 2:
			case 3:
				ani[i].frames = 8;
				break;
			case 4:
			case 5:
				ani[i].frames = 6;
				break;
			case 6:
			case 7:
				ani[i].frames = 8;
				break;
			case 8:
			case 9:
				ani[i].frames = 14;
				break;
			}
		}

		width = ani[0].image.width / ani[0].frames;
		height = ani[0].image.height;
		x = _x - (width / 2);
		y = _y - (height / 2);

		for (int i = 0; i < 4; i++)
		{
			direct[i] = false;
		}
	}

	void toggleCollision(bool to, Character& c)//lock the direction
	{
		isCollide = to;
		if (to)
		{
			float a = 0.0f, b = 0.0f;
			unitVector(a, b, c);
			if (a > 0)
				direct[0] = true;
			else if(a < 0)
				direct[2] = true;
			if (b > 0)
				direct[3] = true;
			else if(b < 0)
				direct[1] = true;
		}
		else {
			for (int i = 0; i < 4; i++)
			{
				direct[i] = false;
			}
		}

	}
	
	void hurt(int damage)
	{
		if (HP - damage > 0)
		{
			HP -= damage;
			SetState(isLeft + 4);
			isHurt = true;
			hurt_time = interval;
		}
		else
			die();
	}
	void die()
	{
		HP = 0;
		SetState(isLeft + 8);
	}
	void powerup(bool trigger)
	{
		if (trigger)
		{
			cooldown1 = cooldown1 / 5;
			N += N;
			AOERange += AOERange;
		}
		else
		{
			cooldown1 = cooldown1 * 5;
			N -= N / 2;
			AOERange -= AOERange/2;
		}
	}
	void update(Window& canvas, int _x, int _y) {

		x += _x;
		y += _y;

		if (interval - hurt_time==60)
			isHurt = false;
		if (killed >= 12 && ispowerup == false)
		{
			killed = 0;
			powerup_time = interval;
			powerup(true);
			ispowerup = true;
		}
		if (interval - powerup_time == 500 && ispowerup == true)
		{
			powerup(false);
			ispowerup = false;
		}
	}
	void draw(Window& canvas) {
		for (unsigned int k = 0; k < height; k++)
			if (k + y - camY > 0 && k + y - camY < canvas.getHeight()) {
				for (unsigned int j = ani[state].index * width; j <= (ani[state].index + 1) * width; j++)
				{
					//std::cout << "frmae: " << j << std::endl;
					if (j - ani[state].index * width + x - camX > 0 && j - ani[state].index * width + x - camX < canvas.getWidth())
						if (ani[state].image.alphaAt(j, k) > 200)
							canvas.draw(j - ani[state].index * width + x - camX, k + y - camY, ani[state].image.at(j, k));
				}
			}
		if (interval % 5 == 0)
		{
			//std::cout << ani[state].index <<" "<<state<< std::endl;
			if (ani[state].index < ani[state].frames - 1)
				ani[state].index++;
			else if(state != isLeft + 8)
			{
				ani[state].index = 0;
				if (state > 3)
				{
					SetState(isLeft);
				}//set to default state
			}
		}
	}
};

class UI
{
public:
	Image image;
	int x;
	int y;
	UI(std::string filename = " ", int _x = 0, int _y = 0)
	{
		// std::cout << filename << std::endl;
		image.load(filename);
		x = _x;
		y = _y;
	}
	UI(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	void draw(Window& canvas) {
		for (unsigned int k = 0; k < image.height; k++)
		{
			if (k + y > 0 && k + y < canvas.getHeight()) {
				for (unsigned int j = 0; j < image.width; j++)
				{
					if (j + x > 0 && j + x < canvas.getWidth())
					{
						if (image.alphaAt(j, k) > 200)
							canvas.draw(j + x, k + y, image.at(j, k));
					}
				}
			}
		}
	}
};

class Number :public UI
{
public:
	Image numbers[10];
	int number;
	Number(int _x, int _y, std::string _color) :UI(_x, _y) {
		int size = 10;
		if (_color == "level")
			size = 2;
		for (int i = 0; i < size; i++)
		{
			numbers[i].load("Resources/UI/" + _color + std::to_string(i) + ".png");
		}
		number = 0;
	}
	void draw(Window& canvas) {
		int figures = 1;
		if (number != 0)
			figures = 1 + log10(number);
		int posX = x;
		std::string tmp = std::to_string(number);
		for (int i = 0; i < figures; i++)
		{
			int index = tmp[i] - '0';
			for (unsigned int k = 0; k < numbers[index].height; k++)
			{
				for (unsigned int j = 0; j < numbers[index].width; j++)
				{
					if (numbers[index].alphaAt(j, k) > 200)
						canvas.draw(j + posX, k + y, numbers[index].at(j, k));
				}
			}
			posX += numbers[index].width;
		}
	}
};

class valueBar :public UI
{
public:
	float percentage;
	valueBar(std::string filename, int _x, int _y, float _percantage) :UI(filename, _x, _y)
	{
		percentage = _percantage;
	}
	void draw(Window& canvas) {
		for (unsigned int k = 0; k < image.height; k++)
		{
			if (k + y > 0 && k + y < canvas.getHeight()) {
				for (unsigned int j = 0; j < image.width * percentage; j++)
				{
					if (j + x > 0 && j + x < canvas.getWidth())
					{
						if (image.alphaAt(j, k) > 200)
							canvas.draw(j + x, k + y, image.at(j, k));
					}
				}
			}
		}
	}
};

class Enemy :public Character {
public:

	int score;
	UI* HPframe = nullptr;
	valueBar* HPbar = nullptr;
	int type;

	Enemy() { 
		score = HP; spriteNum = 10; 
		HPframe = new UI("Resources/UI/enemyHPframe.png", 0, 0);
		HPbar = new valueBar("Resources/UI/enemyHP.png", 0, 0,1);
	}
	Enemy(int _x, int _y, std::string filename) : Character(_x, _y, filename) { 
		score = HP; spriteNum = 10;
		HPframe = new UI("Resources/UI/enemyHPframe.png", 0, 0);
		HPbar = new valueBar("Resources/UI/enemyHP.png", 0, 0, 1);
	}
	~Enemy() {
		delete HPframe;
		delete HPbar;
	}
	virtual void Attack()
	{
		SetState(isLeft + 4);

		//std::cout << ani[state].index << std :: endl;
	}
	virtual std::string getClassName()
	{
		return "Enemy";
	}
	bool collide(Character& c) {
		int dx = (x + width / 2) - (c.getX() + c.getWidth() / 2);
		int dy = (y + height / 2) - (c.getY() + c.getHeight() / 2);

		// this should be set in constructor
		float radius = static_cast<float>(height / 2.5 + c.getHeight() / 2.5);
		if (getClassName()=="Bat")
		{
			radius *= 3;
		}
		float d = sqrtf(dx * dx + dy * dy);
		return d < radius;
	}
	void hurt(int damage,Hero& h)
	{
		isHurt = true;
		if (HP - damage > 0)
		{
			SetState(isLeft + 6);
			HP -= damage;
		}
		else
		{
			die();
		}
	}
	void die()
	{ 
		HP = 0;
		SetState(isLeft + 8);
	}
	void update(int _x, int _y) {
		
		//std::cout << !!isHurt << std::endl;
		if (isCollide == false&&!!isHurt==false)
		{
			x += _x;
			y += _y;

			//set animation
			if (_x > 0)
				isLeft = false;
			else
				isLeft = true;
			SetState(isLeft);
		}
	}
	void draw(Window& canvas) {
		
		//draw enemy
		for (unsigned int k = 0; k < height; k++)
			if (k + y - camY > 0 && k + y - camY < canvas.getHeight()) {
				for (unsigned int j = ani[state].index * width; j <= (ani[state].index + 1) * width; j++)
				{
					//std::cout << "frmae: " << j << std::endl;
					if (j - ani[state].index * width + x - camX > 0 && j - ani[state].index * width + x - camX < canvas.getWidth())
					{
						if (ani[state].image.alphaAt(j, k) > 200)
							canvas.draw(j - ani[state].index * width + x - camX, k + y - camY, ani[state].image.at(j, k));
					}
				}
			}
		 
		//draw UI
		if (isHurt)
		{
			HPframe->x = x - camX; HPframe->y = y - camY; HPbar->x = HPframe->x + 3; HPbar->y = HPframe->y + 3; HPbar->percentage = float(HP) / float(score);
			/*std::cout << HPframe->x<<", "<< HPframe->y<<std::endl;
			std::cout << x << ", " << y << std::endl;*/
			HPframe->draw(canvas);
			HPbar->draw(canvas);
		}
		if (interval % (5 * ani[state].speed) == 0)
		{
			//std::cout << ani[state].index << " " << state << std::endl;
			if (ani[state].index < ani[state].frames - 1)
				ani[state].index++;
			else
			{
				ani[state].index = 0;
				if (state == isLeft + 6)
				{
					isHurt = false;
				}
				if (state > 3&&state<8)
					SetState(isLeft);//set to default state
			}
		}
	}
};
class Purple : public Enemy {
public:
	Purple(int _x, int _y, std::string filename) : Enemy(_x, _y, filename) { 
		speed = 3; HP = 3; attackInterval = 20.0f; damage = 5; score = HP; type = 0;

		for (int i = 0; i < spriteNum; i++)
		{
			//std::cout << filename + "/sprite" + std::to_string(i + 1) +".png" << std::endl;
			ani[i].image.load(filename + "/sprite" + std::to_string(i) + ".png");
			switch (i) {
			case 0:
			case 1:
				ani[i].frames = 9;
				break;
			case 2:
			case 3:
				ani[i].frames = 4;
				ani[i].speed = 2;
				break;
			case 4:
			case 5:
			case 6:
			case 7:
				ani[i].frames = 10;
				break;
			case 8:
			case 9:
				ani[i].frames = 8;
				break;
			}
		}

		width = ani[0].image.width / ani[0].frames;
		height = ani[0].image.height;
		x = _x - (width / 2);
		y = _y - (height / 2);
	}
};
class Green : public Purple {
public:
	Green(int x, int y, std::string filename) : Purple(x, y, filename) { speed = 4; HP = 5; attackInterval = 15.0f; damage = 7; score = HP; type = 1;
	}
};
class Red : public Purple {
public:
	Red(int x, int y, std::string filename) : Purple(x, y, filename) { speed = 5; HP = 7; attackInterval = 13.0f; damage = 10; score = HP; type = 2;
	}
};
class Bat : public Enemy {
public:
	Bat(int _x, int _y, std::string filename) : Enemy(_x, _y, filename) { 
		speed = 7; HP = 10;  attackInterval = 20.0f; score = HP; type = 3;
		for (int i = 0; i < spriteNum; i++)
		{
			//std::cout << filename + "/sprite" + std::to_string(i + 1) +".png" << std::endl;
			ani[i].image.load(filename + "/sprite" + std::to_string(i) + ".png");
			ani[i].frames = 4;
			if(i>7)
				ani[i].frames = 11;
		}

		width = ani[0].image.width / ani[0].frames;
		height = ani[0].image.height;
		x = _x - (width / 2);
		y = _y - (height / 2);
	}
	std::string getClassName() override
	{
		return "Bat";
	}
};

class Projectile{
public:
	int x;
	int y;
	Animation ani;
	int width;
	int height;
	int speed;
	Enemy* target = new Enemy();
	bool isActived;
	int damage;
	float unia;
	float unib;
	int targetx;
	int targety;

	Projectile(int type,int _damage){
		//std::cout << "11" << std::endl;
		ani.image.load("Resources/Projectiles/projectile" + std::to_string(type)+".png");
		switch (type)
		{
			case 0:
				ani.frames = 5;
				speed = 10;
				break;
			case 1:
				ani.frames = 4;
				speed = 5;
				break;
		}
		ani.index = 0;
		width = ani.image.width / ani.frames;
		height = ani.image.height;
		x = 0;
		y = 0;
		isActived = false;
		damage = _damage;
		unia = 0;
		unib = 0;
		targetx = 0;
		targety = 0;
	}
	void draw(Window& canvas) {
		//std::cout << width << " x " << height << std::endl;
		for (unsigned int k = 0; k < height; k++)
			if (k + y - camY > 0 && k + y - camY < canvas.getHeight()) {
				for (unsigned int j = ani.index * width; j <= (ani.index + 1) * width; j++)
				{
					if (j - ani.index * width + x - camX > 0 && j - ani.index * width + x - camX < canvas.getWidth())
						if (ani.image.alphaAt(j, k) > 200)
							canvas.draw(j - ani.index * width + x - camX, k + y - camY, ani.image.at(j, k));
				}
			}
		if (interval % 3 == 0)
		{
			if (ani.index < ani.frames - 1)
				ani.index++;
			else
				ani.index = 0;
		}
	}
	void update(int _x, int _y) {
		x += unia * _x;
		y += unib * _y;
	}
	bool collide(Character& c) {
		int dx = (x + width / 2) - (c.getX() + c.getWidth() / 2);
		int dy = (y + height / 2) - (c.getY() + c.getHeight() / 2);

		// this should be set in constructor
		float radius = static_cast<float>(height / 3 + c.getHeight() /3);
		float d = sqrtf(dx * dx + dy * dy);
		return d < radius;
	}
	void unitVector(float& a, float& b, Character& c)
	{
		float A = (c.getX() + c.getWidth() / 2) - (x + width / 2);
		float B = (c.getY() + c.getHeight() / 2) - (y + height / 2);
		float AB = sqrtf(A * A + B * B);
		a = A / AB;
		b = B / AB;
	}
	void unitVector()
	{
		/*std::cout << "targetx: " << targetx << std::endl;
		std::cout << "targety: " << targety << std::endl;*/
		float A = targetx - (x + width / 2);
		float B = targety - (y + height / 2);
		float AB = sqrtf(A * A + B * B);
		unia = A / AB;
		unib = B / AB;
	}
	void Activate(int _x,int _y)
	{
		isActived = true;
		x = _x;
		y = _y;
	}
	void unActivate()
	{
		isActived = false;
	}
};

class tile {
public:
	Image image;
	tile() {}
	void load(std::string filename) {
		image.load(filename);
	}
	void draw(GamesEngineeringBase::Window& canvas, int x, int y) {
		for (unsigned int i = 0; i < image.height; i++)
			// bounds checking goes here
			if (y + i - camY > 0 && (y + i - camY) < (canvas.getHeight()))
				for (unsigned int n = 0; n < image.width; n++)
					if (x + n - camX > 0 && (x + n - camX) < (canvas.getWidth()))
						canvas.draw(n + x - camX, y + i - camY, image.atUnchecked(n, i));
	}
};

const unsigned int tileNum = 11;
class tileSet {
	tile t[tileNum];
public:
	// create and load tiles here
	tileSet(std::string pre = "") {
		for (unsigned int i = 0; i < tileNum; i++) {
			std::string filename;
			filename = "Resources/tile/" + pre + std::to_string(i) + ".png";
			t[i].load(filename);
		}
	}
	// access individual tile here
	tile& operator[](unsigned int index) { return t[index]; }
};

class World {
public:
	tileSet ts;
	tileSet alphas;
	int* tarray;
	int width;
	int height;
	World() : ts(), alphas("a") {
		std::ifstream file("Resources/tile/tiles" + std::to_string(level) + ".txt");
		std::string tmp;
		file >> tmp;
		width = std::stoi(tmp);
		file >> tmp;
		height = std::stoi(tmp);
		tarray = (int*)malloc(sizeof(*tarray) * width * height);
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				file >> tmp;
				tarray[i * width + j] = std::stoi(tmp);
			}
		}
		file.close();
	}
	~World()
	{
		delete tarray;
	}
	void draw(Window& canvas) {
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int tilex = j * 1024 - camX;
				if (width * 1024 < camX + canvas.getWidth() - j * 1024)//right
				{
					tilex = tilex % 4096 + 4096;
				}
				else if (width * 1024 < j * 1024 - camX)
				{
					tilex = tilex % 4096;
				}
				ts[tarray[j + i * width]].draw(canvas, tilex, i * 768-camY);
			}
		}
	}
	void drawThread(Window& canvas, int startX, int endX) {
		for (int i = 0; i < height; i++)
		{
			for (int j = startX; j < endX; j++)
			{
				ts[tarray[j + i * width]].draw(canvas, j * 1024, i * 768);
			}
		}
	}
	//0:passable 1:impassable 2:speed++ 3:speed--
	int collision(GamesEngineeringBase::Window& canvas, int x, int y) {
		if (x <= 0 || x >= (width * 1024 - 134) || y <= 134 || y >= (height * 768))
		{
			return 1;
		}

		unsigned int offsetx = x % 1024;
		int X = x / 1024;
		unsigned int offsety = y % 768;
		unsigned int Y = y / 768;
		unsigned char r = alphas[tarray[Y * width + X]].image.at(offsetx, offsety, 0);
		unsigned char g = alphas[tarray[Y * width + X]].image.at(offsetx, offsety, 1);
		/*std::cout << "offsetx: " << offsetx << " offsety: " << offsety << std::endl;
		std::cout << "X: " << X << " Y: " << Y << std::endl;*/
		
		if (r == 255 && g == 0)//red
			return 2;
		if (r == 0 && g == 255)//green
			return 3;
		if (r == 0 && g ==0)//black
			return 1;
		return 0;//white

	}
};

const unsigned int maxSize = 100;

class Manager {
public: 
	Enemy* enemy[maxSize];
	float timeElapsed = 0;
	unsigned int currentSize = 0;
	unsigned int currentNum = 0;
	float createThreshold = 3.f;
	Projectile* Bullet_hero[10];
	Projectile* Bullet_bat[20];
	float timer_bullet = 0;
	float cooldown;
	Number* num[7] = { nullptr };
	valueBar* bar[3] = { nullptr };
	int fps = 0;

	void createEnemy(Window& canvas) {
		if (currentSize < maxSize && currentNum <11)
		{
			if (timeElapsed > createThreshold) {
				//location to spawn
				int x = 0, y = 0, z = canvas.getWidth(), w = canvas.getHeight();
				x = rand() % (z + 256 + 1) - 128;
				y = rand() % (w + 256 + 1) - 128;
				int tmp = rand() % 4;
				//std::cout << tmp << std::endl;
				switch (tmp)
				{
				case 0:
					y = -128;
					break;
				case 1:
					y = w;
					//std::cout << x << ", " << y << std::endl;
					break;
				case 2:
					x = -128;
					break;
				case 3:
					x = z;
					break;
				}

				x += camX;
				y += camY;

				//type to spawn
				int type = rand() % 4 + 1;
				//int type = 4;
				//std::cout << type << std::endl;
				std::string filename = "Resources/enemy" + std::to_string(type);
				switch (type) {
				case 1:
					enemy[currentSize++] = new Purple(x, y, filename);
					break;
				case 2:
					enemy[currentSize++] = new Green(x, y, filename);
					break;
				case 3:
					enemy[currentSize++] = new Red(x, y, filename);
					break;
				case 4:
					enemy[currentSize++] = new Bat(x, y, filename);
					break;
				}
				timeElapsed = 0.f;
				createThreshold -= 0.1f;
				createThreshold = max(createThreshold, 0.5f);
				currentNum++;
			}
		}
	}
	void ActivateBullet(Window& canvas,Hero& h)
	{
		if (timer_bullet >= h.cooldown1 && currentNum > 0)
		{
			for (int i = 0; i < 10; i++)
			{
				if (Bullet_hero[i]->isActived == false)
				{
					
					Bullet_hero[i]->Activate(h.getX() + h.getWidth() / 2.5, h.getY()+h.getHeight()/2.5);	
					break;
				}
			}
			timer_bullet = 0.f;
		}
	}

	void ActivateBatBullet(Enemy& e,Hero& h)
	{
		for (int i = 0; i < 20; i++)
		{
			if (Bullet_bat[i]->isActived == false)
			{
				Bullet_bat[i]->Activate(e.getX() + e.getWidth() / 2.5, e.getY() + e.getHeight() / 2.5);
				Bullet_bat[i]->targetx = h.getX()+h.getWidth();
				Bullet_bat[i]->targety = h.getY()+h.getHeight();
				Bullet_bat[i]->unitVector();
				break;
			}
		}
	}

	void checkDeleteEnemy(GamesEngineeringBase::Window& canvas, unsigned int i) {
	}

	void checkDeleteBullet(GamesEngineeringBase::Window& canvas, unsigned int i) {
		if (Bullet_hero[i]->y - camY<0 || Bullet_hero[i]->y - camY >canvas.getHeight() || Bullet_hero[i]->x - camX<0 || Bullet_hero[i]->x - camX >canvas.getWidth()) {
			Bullet_hero[i]->unActivate();
		}
	}
	void checkDeleteBatBullet(Window& canvas,unsigned int i)
	{
		if (Bullet_bat[i]->y - camY<0 || Bullet_bat[i]->y - camY >canvas.getHeight() || Bullet_bat[i]->x - camX<0 || Bullet_bat[i]->x - camX >canvas.getWidth())
			Bullet_bat[i]->unActivate();
	}

public:
	Manager(Hero& h) {
		//precreate bullets from hero
		for (int i = 0; i < 20; i++)
		{
			Bullet_bat[i] = new Projectile(1, 10);
			if(i<10)
				Bullet_hero[i]=new Projectile(0,1);
		}
		cooldown = h.cooldown2;

		 //initiate ui
		num[0] = new Number(398, 50, "hp");
		num[1] = new Number(398, 72, "mp");
		num[2] = new Number(818, 56, "level");
		num[3] = new Number(908, 50, "score");
		num[4] = new Number(908, 69, "score");
		num[5] = new Number(445, 50, "hp");
		num[6] = new Number(445, 72, "mp");
		num[5]->number = h.HP;
		num[6]->number=h.cooldown2;

		bar[0] = new valueBar("Resources/UI/HP.png", 135, 50, 1);
		bar[1] = new valueBar("Resources/UI/MP.png", 135, 78, 0);
		bar[2] = new valueBar("Resources/UI/powerup.png", 408, 710, 0);
		
	}

	~Manager() {
		for (unsigned int i = 0; i < currentSize; i++)
			if (enemy[i])
				delete enemy[i];
	}

	void update(GamesEngineeringBase::Window& canvas, float dt, Hero &h) {

		timeElapsed += dt;
		timer_bullet += dt;

		//std::cout << "timer: " << timer_bullet << std::endl;

		//spawn enemy
		createEnemy(canvas);

		//std::cout <<"currentsize: " << currentSize << std::endl;
		for (unsigned int i = 0; i < currentSize; i++) {
		// check for nullptr cause might have been deleted
			if (enemy[i]) {
				//std::cout <<"enemy" <<i<<" HP:" << enemy[i]->HP << std::endl;
				if (!enemy[i]->isCollide)//if enemy is not colliding with hero
				{
					float a = 0.0f, b = 0.0f;
					enemy[i]->unitVector(a, b, h);
					enemy[i]->update(a * enemy[i]->getSpeed(), b * enemy[i]->getSpeed());
				}
				else if (enemy[i]->getClassName() == "Enemy" && enemy[i]->state == enemy[i]->isLeft + 4 && enemy[i]->ani[enemy[i]->state].index == 7 && !h.isHurt)
				{
					//hero hurt at frame7
					h.hurt(enemy[i]->damage);
				}
				//delete enemy after playing the final frame of dying animation
				if (enemy[i]->state == enemy[i]->isLeft + 8 && enemy[i]->ani[enemy[i]->state].index == enemy[i]->ani[enemy[i]->state].frames-1)
				{
					h.score += enemy[i]->score;
					delete enemy[i];
					enemy[i] = nullptr;
					currentNum--;
					if (currentNum == maxSize)
					{
						currentSize = 0;
						currentNum = 0;
					} 
					if (h.ispowerup == false)
					{
						h.killed++;
					}
					
				}
			}
		}
		
		
		//spawn bullet from hero
		ActivateBullet(canvas, h);

		float halfWidth = canvas.getWidth() / 2, halfHeight = canvas.getHeight() / 2;
		for (unsigned int i = 0; i < 10; i++) {
			if (Bullet_hero[i]->isActived) {
				if (currentSize > 0)
				{
					//find the closest enemy
					int closest_enemy = 0;
					float min = sqrtf(halfWidth * halfWidth + halfHeight * halfHeight);
					for (int j = 0; j < currentSize; j++)
					{
						if (enemy[j] != nullptr)
						{
							float _x = fabs(Bullet_hero[i]->x - enemy[j]->getX()), _y = fabs(Bullet_hero[i]->y - enemy[j]->getY());
							float _z = sqrtf(_x * _x + _y * _y);
							if (_z < min)
							{
								min = _z;
								closest_enemy = j;
							}
						}
					}

					//target the closest enemy
					if (enemy[closest_enemy] != nullptr)
					{
						Bullet_hero[i]->unitVector(Bullet_hero[i]->unia, Bullet_hero[i]->unib, *enemy[closest_enemy]);
						Bullet_hero[i]->target = enemy[closest_enemy];
					}
					Bullet_hero[i]->update(Bullet_hero[i]->speed,Bullet_hero[i]->speed);
				}
				checkDeleteBullet(canvas, i);
			}
		}
		for (int i = 0; i < 20; i++)
		{
			if (Bullet_bat[i]->isActived)
			{
				Bullet_bat[i]->update(Bullet_bat[i]->speed, Bullet_bat[i]->speed);
				//std::cout << Bullet_bat[i]->targetx;
				checkDeleteBatBullet(canvas, i);
			}
		}

		//calculate cooldown
		//std::cout << cooldown << std::endl;
		if (cooldown > 0)
			cooldown--;
		//active attck
		if (canvas.keyPressed(VK_SPACE))
		{
			if (cooldown==0 && h.state != h.isLeft + 8)
			{
				h.SetState(h.isLeft + 6);
				//find the index of all the enemyies inside the range
				int index[maxSize];
				int size = 0;
				for (unsigned int i = 0; i < currentSize; i++)
				{
					if (enemy[i]!=nullptr)
					{
						bool tmp;
						//std::cout << "cooldown" << std::endl;
						if (h.isLeft)
							tmp = enemy[i]->getX() <= h.getX();
						else
							tmp = enemy[i]->getX() >= h.getX();

						int dx = fabs(h.getX() - enemy[i]->getX());
						int dy = fabs(h.getY() - enemy[i]->getY());
						if (sqrt(dx * dx + dy * dy) <= h.AOERange && tmp)
						{
							index[size++] = i;
						}
					}
				}
				
				//sort the index, max->min
				if (size > h.N)
				{	
					for (int i = size - 1; i >= 0; i--)
					{
						for (int j = 0; j < i; j++)
						{
							if (enemy[index[i]]!=nullptr)
							{
								if (enemy[index[i]]->HP > enemy[index[j]]->HP)
								{
									int flag = index[i];
									index[i] = index[j];
									index[j] = flag;
								}
							}
						}
					}
					
				}

				//attck the top N enemy
				int min = (h.N < size ? h.N : size);
				for (int i = 0; i < min; i++)
				{
					if(enemy[index[i]]!=nullptr)
						enemy[index[i]]->hurt(h.AOE_damage,h);
				}
				cooldown = h.cooldown2;
			}
		}
		
		//update UI
		num[0]->number = h.HP;
		num[1]->number = h.cooldown2 - cooldown;	
		num[2]->number = level;
		num[3]->number = h.score;
		fps+= 1 / dt;
		if (interval % 10 == 0)
		{
			num[4]->number = fps / 10;
			fps = 0;
		}
			

		bar[0]->percentage = float(h.HP)/100.f;
		bar[1]->percentage = float(num[1]->number) / h.cooldown2;
		if (h.ispowerup == false)
		{
			bar[2]->percentage = float(h.killed) / 12.f;
		}
		else
			bar[2]->percentage = float(500 - (interval - h.powerup_time)) / 500;
		
	}

	void draw(Window& canvas) {
		for (unsigned int i = 0; i < currentSize; i++) {
			if (enemy[i])
				enemy[i]->draw(canvas);
		}
		for (int j = 0; j < 20; j++)
		{
			if (Bullet_bat[j]->isActived)
			{
				Bullet_bat[j]->draw(canvas);
			}
			if (j < 10)
			{
				if (Bullet_hero[j]->isActived)
					Bullet_hero[j]->draw(canvas);
			}
		}
	}

	void drawUI(Window& canvas)
	{
		//draw UI
		for (int i = 0; i < 7; i++)
		{
			num[i]->draw(canvas);
			if (i < 3)
				bar[i]->draw(canvas);
		}
	}

	void collision(Hero& h) {
		//enemy vs hero
		for (unsigned int i = 0; i < currentSize; i++)
			if (enemy[i] != nullptr) {
				bool tmp = enemy[i]->collide(h);
				enemy[i]->toggleCollision(tmp);
				if (tmp)
				{
					if(enemy[i]->state==enemy[i]->isLeft)
						enemy[i]->SetState(enemy[i]->isLeft + 2);
					if (enemy[i]->attaTimer >= (enemy[i]->attackInterval*5) && enemy[i]->isHurt==false)
					{
						enemy[i]->Attack();
						if (enemy[i]->getClassName() == "Bat")
						{
							ActivateBatBullet(*enemy[i],h);
						}
						enemy[i]->attaTimer = 0;
					}
					enemy[i]->attaTimer++;
				}
				else
					enemy[i]->attaTimer = 0;
			}
		//hero's bullet vs enemy
		for (unsigned int j = 0; j < 10; j++){
			if (Bullet_hero[j]->isActived) {
				if (Bullet_hero[j]->collide(*Bullet_hero[j]->target))
				{
					Bullet_hero[j]->unActivate();
					Bullet_hero[j]->target->hurt(Bullet_hero[j]->damage,h);
				}
			}
		}
		//bat's bullet vs hero
		for (unsigned int j = 0; j < 20; j++) {
			if (Bullet_bat[j]->isActived) {
				if (Bullet_bat[j]->collide(h))
				{
					Bullet_bat[j]->unActivate();
					h.hurt(Bullet_bat[j]->damage);
				}
			}
		}
	}
};

void ModifyLineData(std::string fileName, int lineNum, std::string lineData)
{
	std::ifstream in;
	in.open(fileName);
	std::string strFileData = "";
	int line = 1;
	char tmpLineData[1024] = { 0 };
	while (in.getline(tmpLineData, sizeof(tmpLineData)))
	{
		if (line == lineNum)
		{
			strFileData += std::string(lineData);
			strFileData += "\n";
		}
		else
		{
			strFileData += std::string(tmpLineData);
			strFileData += "\n";
		}
		line++;
	}
	in.close();
	std::ofstream out;
	out.open(fileName);
	out.flush();
	out << strFileData;
	out.close();
}

int main() {

	time_t tim = time(NULL);
	srand((unsigned int)tim);
	// Create a canvas window with dimensions 1024x768 and title “Tiles"
	GamesEngineeringBase::Window canvas;
	canvas.create(1024, 768, "Tiles");
	bool running = true; // Variable to control the main loop's running state.

	ShowCursor(FALSE);
	//main menu
	UI mainmenu("Resources/UI/mainmenu.png", 0, 0);
	UI selectbox("Resources/UI/selectbox.png", 0, 0);
	UI mouse("Resources/UI/mouse_potion.png", 0, 0);
	

	SoundManager sounds;
	sounds.loadMusic("Resources/mainmenu.wav");
	sounds.playMusic();

	Hero hero(camX + canvas.getWidth() / 2, camY + canvas.getHeight() / 2, "Resources/hero");
	Manager m(hero);

	while (running)
	{
		canvas.clear();

		mainmenu.draw(canvas);

		mouse.x = canvas.getMouseX();
		mouse.y= canvas.getMouseY();
		mouse.draw(canvas);

		int button = 0;
		if (mouse.x > 442 && mouse.x < 603)
		{
			if (mouse.y > 251 && mouse.y < 295)
			{
				button = 1;
				selectbox.x = 434, selectbox.y = 238;
				selectbox.draw(canvas);
			}
			if (mouse.y > 321 && mouse.y < 370)
			{
				button = 2;
				selectbox.x = 434, selectbox.y = 311;
				selectbox.draw(canvas);
			}
			if (mouse.y > 395 && mouse.y < 445)
			{
				button = 3;
				selectbox.x = 434, selectbox.y = 386;
				selectbox.draw(canvas);
			}
			if (mouse.y > 467 && mouse.y < 517)
			{
				button = 4;
				selectbox.x = 434, selectbox.y = 457;
				selectbox.draw(canvas);
			}
		}
		

		if (canvas.mouseButtonPressed(MouseButton::MouseLeft))
		{
			if (button == 1)//load file
			{
				std::ifstream file("Resources/save.txt");
				std::string tmp;
				file >> tmp;
				camX = std::stoi(tmp);
				file >> tmp;
				camY = std::stoi(tmp);
				hero.setX(camX + canvas.getWidth() / 2);
				hero.setY(camY + canvas.getHeight() / 2);
				file >> tmp;
				hero.HP = std::stoi(tmp);
				file >> tmp;
				m.cooldown = std::stof(tmp);
				file >> tmp;
				hero.score = std::stoi(tmp);
				file >> tmp;
				hero.killed = std::stoi(tmp);
				file >> tmp;
				level = std::stoi(tmp);
				file >> tmp;
				m.currentNum = std::stoi(tmp);
				file >> tmp;
				m.currentSize = std::stoi(tmp) - m.currentNum;

				for (int i = 0; i < m.currentSize; i++)
				{
					m.enemy[i] = nullptr;
				}
				for (int i = 0; i < m.currentNum; i++)
				{
					file >> tmp;
					int type = std::stoi(tmp) + 1;
					std::string filename = "Resources/enemy" + std::to_string(type);
					file >> tmp;
					int enemyx = std::stoi(tmp);
					file >> tmp;
					int enemyy = std::stoi(tmp);
					switch (type) {
					case 1:
						m.enemy[m.currentSize] = new Purple(enemyx, enemyy, filename);
						break;
					case 2:
						m.enemy[m.currentSize] = new Green(enemyx, enemyy, filename);
						break;
					case 3:
						m.enemy[m.currentSize] = new Red(enemyx, enemyy, filename);
						break;
					case 4:
						m.enemy[m.currentSize] = new Bat(enemyx, enemyy, filename);
						break;
					}
					file >> tmp;
					m.enemy[m.currentSize]->HP = std::stoi(tmp);
					//std::cout << m.currentSize << std::endl;
					m.currentSize++;
				}
				break;
			}
			if (button == 2)
			{
				level = 0;
				break;
			}
			if (button == 3)
			{
				level = 1;
				break;
			}
			if (button == 4)
			{
				return 0;
			}
		}

		canvas.present();
	}

	
	UI ui("Resources/UI/ui.png",0,0);
	
	sounds.~SoundManager();
	SoundManager gamesounds;
	gamesounds.loadMusic("Resources/theme.wav");
	gamesounds.playMusic();


	World world;

	//float speed = 500.0f;
	Timer timer;
	float dt = 0.0f;
	float distance=0.0f;


	while (running)
	{
		timingus(11000);
		// Check for input (key presses or window events)
		// Clear the window for the next frame rendering
		canvas.checkInput();
		
		dt = timer.dt();
		distance = hero.getSpeed() * dt;
		
		//std::cout << "DT: " << dt << " Distance: " << distance << std::endl;
		if (canvas.keyPressed(VK_ESCAPE))
		{
			//hero
			ModifyLineData("Resources/save.txt", 1, std::to_string(camX) + " " + std::to_string(camY) + " " + std::to_string(hero.HP));
			ModifyLineData("Resources/save.txt", 2, std::to_string(int(m.cooldown)) + " " + std::to_string(hero.score) + " " + std::to_string(hero.killed) + " " + std::to_string(level));
			//enemy
			ModifyLineData("Resources/save.txt", 3, std::to_string(m.currentNum) + " " + std::to_string(m.currentSize));
			int count = 4;
			for (int i = 0; i < m.currentSize; i++)
			{
				if (m.enemy[i] != nullptr)
				{
					ModifyLineData("Resources/save.txt", count++, std::to_string(m.enemy[i]->type) + " " + std::to_string(m.enemy[i]->getX()) 
						+ " " + std::to_string(m.enemy[i]->getY()) + " " + std::to_string(m.enemy[i]->HP));
				}
				
			}
			
			break;
		}
		if (hero.HP <= 0)
		{
			hero.SetState(hero.isLeft + 8);
		}
		//std::cout << "HP:" << hero.HP << std::endl;

		int herox = camX;
		int heroy = camY;

		//std::cout << hero.state <<" "<< hero.isLeft + 1 << std::endl;
		if (hero.state == hero.isLeft+2)
		{
			hero.state = hero.isLeft;
		}
		bool walkable=true;
		if (hero.state >= hero.isLeft + 4)
		{
			walkable = false;
		}
		//std::cout << walkable << std::endl;
		if (canvas.keyPressed('A') && walkable)
		{
			hero.isLeft = true;
			herox -= distance;
			hero.SetState(hero.isLeft + 2);
		}
		if (canvas.keyPressed('D') && walkable)
		{
			//std::cout << "background.w: " << background.width << std::endl;
			//std::cout << "herox1: " << herox << std::endl;
			hero.isLeft = false;
			herox += distance;
			hero.SetState(hero.isLeft + 2);
		}
		if (canvas.keyPressed('W') && walkable)
		{
			
			heroy -= distance;
			hero.SetState(hero.isLeft + 2);
		}
		if (canvas.keyPressed('S') && walkable)
		{
			heroy += distance;
			hero.SetState(hero.isLeft + 2);
		}
		
		//update
		//hero
		int dx = herox - camX;
		int dy = heroy - camY;
		int type = world.collision(canvas, hero.getX() + dx, hero.getY() + hero.getHeight() + dy);
		std::cout << " type: " << type << std::endl;
		switch (type)
		{
			case 0:
				break;
			case 1:
				dx = 0;
				dy = 0;
				break;
			case 2:
				dx *= 1.5;
				dy *= 1.5;
				break;
			case 3:
				dx /= 2;
				dy /= 2;
				break;
		}
		
		hero.update(canvas, dx, dy);
		/*std::cout << hero.getX() << "," << hero.getY()<<std::endl;*/
		//camera
		if (hero.getX() >= canvas.getWidth() / 2 - 67 && hero.getX() <= world.width * 1024 - canvas.getWidth() / 2 - 67)
		{
			camX = hero.getX() + hero.getWidth() / 2 - canvas.getWidth() / 2;
		}
		if (hero.getY() >= canvas.getHeight() / 2 - 67 && hero.getY() <= world.height * 768 - canvas.getHeight() / 2 - 67)
		{

			camY = hero.getY() + hero.getHeight() / 2 - canvas.getHeight() / 2;
		}
		
		m.update(canvas, dt, hero);
		m.collision(hero);

		//draw
		//world.draw(canvas);
		std::thread t1(&World::drawThread, &world, std::ref(canvas), 0, 1);
		std::thread t2(&World::drawThread, &world, std::ref(canvas), 1, 2);
		std::thread t3(&World::drawThread, &world, std::ref(canvas), 2, 3);
		std::thread t4(&World::drawThread, &world, std::ref(canvas), 3, 4);
		t1.join();
		t2.join();
		t3.join();
		t4.join();

		m.draw(canvas);
		hero.draw(canvas);
		ui.draw(canvas);
		m.drawUI(canvas);

		interval++;

		canvas.present();

		//std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return 0;
}
