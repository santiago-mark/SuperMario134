/*
photo credit:
Speedster/Mariorocks21,
Jouw
*/
/* #includes left out */
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <GL/glew.h>
#include <SDL.h>
#include <math.h>
#include "DrawUtils.h"

#undef main

char shouldExit = 0;
bool right = true;
int temp = 0;

struct AABB {
	int x, y, w, h;
};
struct point{
	int x;
	int y;
};
bool AABBIntersect(const AABB* box1, const AABB* box2)
{
	// box1 to the right
	if (box1->x > box2->x + box2->w) {
		return false;
	}
	// box1 to the left
	if (box1->x + box1->w < box2->x) {
		return false;
	}
	// box1 below
	if (box1->y > box2->y + box2->h) {
		return false;
	}
	// box1 above
	if (box1->y + box1->h < box2->y) {
		return false;
	}
	return true;
}
struct Coin{
	AABB box;
	bool exist;
};
struct Player{
	int hp; int mp; int coin; int life;
	AABB box;
	void checkLife(){
		if (coin > 99){
			coin = coin % 100;
			life++;
		}
	};
};
// Define a single frame used in an animation
struct Enemy{
	AABB box;
	point mid;
};
struct bgBlock{
	bool col;
	bool breakable;
	GLuint tile;
};
struct Collision {
	int x;
	int y;
};
Collision bgCollision(Player *player, AABB *tile){
	int x = 0, y = 0;

	int playerLeft = player->box.x;
	int playerRight = player->box.x + player->box.w;
	int playerTop = player->box.y;
	int playerBottom = player->box.y + player->box.h;

	int tileLeft = tile->x;
	int tileRight = tile->x + tile->w;
	int tileTop = tile->y;
	int tileBottom = tile->y + tile->h;

	// box1 to the right
	if (playerRight > tileLeft) {
		if (playerTop < tileBottom){
			x = playerRight - tileLeft;
			y = tileBottom - playerTop;
		}
		else if (playerBottom > tileTop){
			x = playerRight - tileLeft;
			y = playerBottom - tileTop;
		}
		else if (playerTop < tileTop && playerBottom > tileBottom){
			x = playerRight - tileLeft;
		}
		else if (playerTop > tileTop && playerBottom < tileBottom){
			x = playerRight - tileLeft;
		}
	}
	// box1 to the left
	if (playerLeft < tileRight) {
		if (playerTop < tileBottom){
			x = tileRight - playerLeft;
			y = tileBottom - playerTop;
		}
		else if (playerBottom > tileTop){
			x = tileRight - playerLeft;
			y = playerBottom - tileTop;
		}
		else if (playerTop < tileTop && playerBottom > tileBottom){
			x = tileRight - playerLeft;
		}
		else if (playerTop > tileTop && playerBottom < tileBottom){
			x = tileRight - playerLeft;
		}
	}
	// box1 below 
	else if (playerTop < tileBottom) {
		if (player->box.x + player->box.w > tile->x + tile->w)
			y = player->box.y - tile->y + tile->h;
		if (player->box.x > tile->x  && player->box.x + player->box.w < tile->x + tile->w)
			y = player->box.y - tile->y + tile->h;
	}
	// box1 above
	else if (player->box.y + player->box.h > tile->y) {
		if (player->box.x < tile->x && player->box.x + player->box.w > tile->x + tile->w)
			y = tile->y - player->box.y + player->box.h;
		if (player->box.x > tile->x  && player->box.x + player->box.w < tile->x + tile->w)
			y = player->box.y - tile->y + tile->h;

	}
	else if (player->box.x < tile->x && player->box.x + player->box.w < tile->x + tile->w){

	}
	else if (player->box.x > tile->x && player->box.x + player->box.w < tile->x + tile->w){

	}

	return Collision{ x, y };
}
struct AnimFrameDef {
	// combined with the AnimDef's name to make
	// the actual texture name
	int frameNum;
	float frameTime;
};
struct AnimDef {
	const char* name;
	AnimFrameDef frames[20];
	int maxFrame, numFrames;
};
// Runtime state for an animation
struct AnimData {
	AnimDef* def;
	int curFrame;
	float timeToNextFrame;
	bool isPlaying;
};

// Update the animation for time passing
void animTick(AnimData* data, float dt)
{
	if (!data->isPlaying) {
		return;
	}
	int numFrames = data->def->maxFrame;
	data->timeToNextFrame -= dt;
	if (data->timeToNextFrame < 0) {
		++data->curFrame;
		if (data->curFrame >= numFrames) {
			// end of the animation, stop it
			data->curFrame = numFrames - 1;
			data->timeToNextFrame = 0;
			data->isPlaying = false;
		}
		else {
			AnimFrameDef *curFrame = &(data->def->frames[data->curFrame]);
			data->timeToNextFrame += curFrame->frameTime;
		}
	}
}
void animSet(AnimData* anim, AnimDef* toPlay)
{
	anim->def = toPlay;
	anim->curFrame = 0;
	anim->timeToNextFrame
		= anim->def->frames[0].frameTime;
	anim->isPlaying = true;
}
void animReset(AnimData* anim)
{
	animSet(anim, anim->def);
}
void animDraw(AnimData* anim, int x, int y, int w, int h, GLuint textures[])
{
	int curFrameNum = anim->def->frames[anim->curFrame].frameNum;
	GLuint tex = textures[curFrameNum];
	glDrawSprite(tex, x, y, w, h);
}


int main(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}
	/* Create the window, OpenGL context */
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	const int NUM_OF_TILEX = 20; const int NUM_OF_TILEY = 10;
	const int TILE_SIZEX = 64, TILE_SIZEY = 64;
	const int WORLD_SIZEX = NUM_OF_TILEX* TILE_SIZEX; const int WORLD_SIZEY = NUM_OF_TILEY*TILE_SIZEY;
	int camX = 0; int camY = 0;
	int playerX = 0; int playerY = 0;
	AABB camera; AABB tile;
	Uint32 lastFrameMs;
	Uint32 currentFrameMs = SDL_GetTicks();
	bgBlock ground00, ground01, ground02, ground03, ground04, ground05, ground06, transparency;
	GLuint	coin1, coin2, coin3,
		mario_idle, mario_walk1, mario_walk2, mario_walk3, mario_walk4, mario_walk5, mario_walk6;
	GLuint goomba1, goomba2;

	SDL_Window* window = SDL_CreateWindow(
		"TestSDL",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 480,
		SDL_WINDOW_OPENGL);
	if (!window) {
		fprintf(stderr, "Could not create window.ErrorCode = %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_GL_CreateContext(window);

	/* Make sure we have a recent version of OpenGL */
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		fprintf(stderr, "Could not initialize glew.ErrorCode = %s\n", glewGetErrorString(glewError));
		SDL_Quit();
		return 1;
	}
	if (!GLEW_VERSION_3_0) {
		fprintf(stderr, "OpenGL max supported version is too low.\n");
		SDL_Quit();
		return 1;
	}
	/* Setup OpenGL state */
	glViewport(0, 0, 640, 480);
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 640, 480, 0, 0, 100);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* The previous frame's keyboard state */
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = { 0 };
	/* The current frame's keyboard state */
	const unsigned char* kbState = NULL;
	/* Keep a pointer to SDL's internal keyboard state */
	kbState = SDL_GetKeyboardState(NULL);

	/* load the textures */
	transparency = { false, false, glTexImageTGAFile("Mario/transparency.tga", NULL, NULL) };
	ground00 = { true, false, glTexImageTGAFile("Mario/ground00.tga", NULL, NULL) };
	ground01 = { true, false, glTexImageTGAFile("Mario/ground01.tga", NULL, NULL) };
	ground02 = { true, false, glTexImageTGAFile("Mario/ground02.tga", NULL, NULL) };
	ground03 = { true, false, glTexImageTGAFile("Mario/ground03.tga", NULL, NULL) };
	ground04 = { true, false, glTexImageTGAFile("Mario/ground04.tga", NULL, NULL) };
	ground05 = { true, false, glTexImageTGAFile("Mario/ground05.tga", NULL, NULL) };
	ground06 = { true, false, glTexImageTGAFile("Mario/ground06.tga", NULL, NULL) };
	coin1 = glTexImageTGAFile("Mario/coin1.tga", NULL, NULL);
	coin2 = glTexImageTGAFile("Mario/coin2.tga", NULL, NULL);
	coin3 = glTexImageTGAFile("Mario/coin3.tga", NULL, NULL);
	mario_idle = glTexImageTGAFile("Mario/mario_idle.tga", NULL, NULL);
	mario_walk1 = glTexImageTGAFile("Mario/mario_walk1.tga", NULL, NULL);
	mario_walk2 = glTexImageTGAFile("Mario/mario_walk2.tga", NULL, NULL);
	mario_walk3 = glTexImageTGAFile("Mario/mario_walk3.tga", NULL, NULL);
	mario_walk4 = glTexImageTGAFile("Mario/mario_walk4.tga", NULL, NULL);
	mario_walk5 = glTexImageTGAFile("Mario/mario_walk5.tga", NULL, NULL);
	mario_walk6 = glTexImageTGAFile("Mario/mario_walk6.tga", NULL, NULL);
	goomba1 = glTexImageTGAFile("Mario/goomba1.tga", NULL, NULL);
	goomba2 = glTexImageTGAFile("Mario/goomba2.tga", NULL, NULL);

	/*2D arrays for background1*/
	bgBlock backgroundTile1[NUM_OF_TILEX][NUM_OF_TILEY] = {
		//0			1		2		3			4		5		6		7		8		9		10		11		12		13		14		15		16		17		18		19
		{ transparency, transparency, transparency, transparency, ground00, ground04, ground04, ground04, ground04, ground04 }, //0
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //1
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //2
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //3
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //4
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //5
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //6
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //7
		{ transparency, transparency, ground00, ground04, ground01, ground05, ground05, ground05, ground05, ground05 }, //8
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //9
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //11
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //12
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //13
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //14
		{ transparency, transparency, ground01, ground05, ground01, ground05, ground05, ground05, ground05, ground05 }, //15
		{ transparency, transparency, ground02, ground06, ground01, ground05, ground05, ground05, ground05, ground05 }, //16
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //17
		{ transparency, transparency, transparency, transparency, ground01, ground05, ground05, ground05, ground05, ground05 }, //18
		{ transparency, transparency, transparency, transparency, ground02, ground06, ground06, ground06, ground06, ground06 } //19
	};
	/*array of textures of Mario walking*/
	GLuint mario_walking[7] = { mario_idle, mario_walk1, mario_walk2, mario_walk3, mario_walk4, mario_walk5, mario_walk6 };
	/*array of textures of coin sparkling*/
	GLuint coin_anim[3] = { coin1, coin2, coin3 };
	/*array of textures of coin sparkling*/
	GLuint goomba_walking[2] = { goomba1, goomba2 };

	/*animation definitions of coin*/
	AnimDef coin_sparkles{
		"coin",
		{ { 0, .1 }, { 1, .1 }, { 2, .1 }, { 1, .1 } },
		20,
		4
	};
	/*animation definition of mario walking*/
	AnimDef mario{
		"mario_walk",
		{ { 0, .1 }, { 1, .1 }, { 2, .1 }, { 3, .1 }, { 4, .1 }, { 5, .1 }, { 6, .1 } },
		20,
		7
	};
	/*animation definition for groomba*/
	AnimDef groomba{
		"groomba",
		{ { 0, .1 }, { 0, .1 } },
		20,
		2
	};
	/*animation of coin*/
	AnimData coin = {
		&coin_sparkles,
		0,
		0.1,
		true
	};
	/*animation of mario_walking*/
	AnimData m_walk{
		&mario,
		0,
		0.1,
		true
	};
	/*animation of groomba walking*/
	AnimData g_walk{
		&groomba,
		0,
		0.1,
		true
	};
	Coin c1 = {
		{ 192, 192, 32, 32 },
		true
	};
	Coin c2 = {
		{ 242, 192, 32, 32 },
		true
	};
	Coin c3 = {
		{ 292, 192, 32, 32 },
		true
	};
	Player player = {
		100, 20, 0, 1,
		{ playerX, playerY, 38, 76 }
	};
	Enemy goomba = {
		{192,192,32,32},
		{192+16, 192+16}
	};
	animSet(&coin, &coin_sparkles);
	animSet(&g_walk, &groomba);

	//animSet(&m_walk, &mario);
	/* The game loop */
	while (!shouldExit) {
		/******************************************************Save the last Frame's value******************************************************/
		lastFrameMs = currentFrameMs;
		// Handle OS message pump
		/* kbState is updated by the message pump. Copy over the old state before the pump! */
		memcpy(kbPrevState, kbState, sizeof(kbPrevState));
		currentFrameMs = SDL_GetTicks();
		float deltaTime = (currentFrameMs - lastFrameMs) / 1000.0f;
		/******************************************************OS message Pump******************************************************************/

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				shouldExit = 1;
			}
		}
		if (kbState[SDL_SCANCODE_W]){
			if (camY > 0){
				camY--;
			}
		}
		if (kbState[SDL_SCANCODE_S]){
			if (camY < (WORLD_SIZEY - 480)){
				camY++;
			}
		}
		if (kbState[SDL_SCANCODE_A]){
			if (camX > 0){
				camX--;
			}
		}
		if (kbState[SDL_SCANCODE_D]){
			if (camX < (WORLD_SIZEX - 640)){
				camX++;
			}
		}
		if (kbState[SDL_SCANCODE_LEFT]){
			if (!kbPrevState[SDL_SCANCODE_LEFT]){
				animSet(&m_walk, &mario);
			}
			else{
				if (!m_walk.isPlaying)
					animReset(&m_walk);
			}
			if (playerX > camX){
				playerX--;
			}
		}
		if (kbState[SDL_SCANCODE_RIGHT]){
			if (!kbPrevState[SDL_SCANCODE_RIGHT]){
				animSet(&m_walk, &mario);
			}
			else{
				if (!m_walk.isPlaying)
					animReset(&m_walk);
			}
			if (playerX + player.box.w < camX + 640){
				playerX++;
			}
		}
		if (kbState[SDL_SCANCODE_UP]){
			if (!kbPrevState[SDL_SCANCODE_UP]){
				animSet(&m_walk, &mario);
			}
			else{
				if (!m_walk.isPlaying)
					animReset(&m_walk);
			}
			if (playerY > camY){
				playerY--;
			}
		}
		if (kbState[SDL_SCANCODE_DOWN]){
			if (!kbPrevState[SDL_SCANCODE_DOWN]){
				animSet(&m_walk, &mario);
			}
			else{
				if (!m_walk.isPlaying)
					animReset(&m_walk);
			}
			if (playerY + player.box.h < camY + 480){
				playerY++;
			}
		}
		/******************************************************Update and draw game*************************************************************/

		glClearColor(161, 180, 255, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		/*draws the background1 of the game*/
		int i, j, worldW, worldH;
		Collision c = { 0, 0 };
		bool collided = false;
		for (i = 0; i < NUM_OF_TILEX; i++){
			for (j = 0; j < NUM_OF_TILEY; j++){
				worldW = i * 64;
				worldH = j * 64;
				tile = { worldW, worldH, 64, 64 };
				if (AABBIntersect(&player.box, &tile) && !collided){
					if (backgroundTile1[i][j].col){
						c = bgCollision(&player, &tile);
						if (kbState[SDL_SCANCODE_LEFT])
							playerX += c.x;
						if (kbState[SDL_SCANCODE_RIGHT])
							playerX -= c.x;
						if (kbState[SDL_SCANCODE_UP])
							playerY += c.y;
						if (kbState[SDL_SCANCODE_DOWN])
							playerY -= c.y;
						collided = true;
					}
				}
				if (AABBIntersect(&camera, &tile)){
					glDrawSprite(backgroundTile1[i][j].tile, worldW - camX, worldH - camY, 64, 64);
				}
			}
		}
		camera = { camX, camY, 640, 480 };
		player = {
			100, 20, 0, 1,
			{ playerX, playerY, 38, 76 }
		};
		/*draw sprites*/
		animTick(&coin, deltaTime);
		animTick(&m_walk, deltaTime);

		if (AABBIntersect(&c1.box, &player.box)){
			c1.exist = false;
			player.checkLife();
		}
		if (AABBIntersect(&(c2.box), &(player.box))){
			c2.exist = false;
			player.checkLife();
		}
		if (AABBIntersect(&(c3.box), &(player.box))){
			c3.exist = false;
			player.checkLife();
		}
		if (c1.exist)
			animDraw(&coin, c1.box.x - camX, c1.box.y - camY, c1.box.w, c1.box.h, coin_anim);
		if (c2.exist)
			animDraw(&coin, c2.box.x - camX, c2.box.y - camY, c2.box.w, c2.box.h, coin_anim);
		if (c3.exist)
			animDraw(&coin, c3.box.x - camX, c3.box.y - camY, c3.box.w, c3.box.h, coin_anim);
		if (!coin.isPlaying){ //coin keeps animating
			animReset(&coin);
		}
		if (goomba.box.x + goomba.box.w == WORLD_SIZEX){
			right = false;
		}
		else if (goomba.box.x + goomba.box.w == 0)
			right = true;
		
		if (right && temp%5 == 0 ){
			goomba.box.x++;
		}
		else if (!right && temp%5 == 0){
			goomba.box.x --;
		}
		animDraw(&g_walk, goomba.box.x - camX, goomba.box.y - camY, 32, 32, goomba_walking);
		temp++;
		if (!g_walk.isPlaying)
			animReset(&g_walk);
		animDraw(&m_walk, player.box.x - camX, player.box.y - camY, player.box.w, player.box.h, mario_walking);
		/*draw foreground of the game*/
		/* Present to the player */
		SDL_GL_SwapWindow(window);
	}
}