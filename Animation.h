// Define a single frame used in an animation
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