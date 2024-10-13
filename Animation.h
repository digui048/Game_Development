#pragma once
#include "SDL2/SDL_rect.h"
#define MAXIMUM_FRAMES 30

class Animation 
{
private:
	int last_frame = 0;
	float current_frame = 0.0f;
	int frame = -1;
	int repetitions = 0;


public:
	
	//Constructor
	Animation() {}

	//Destructor
	~Animation() {}
	//Adds a new frame to the animation
	void PushBack(const SDL_Rect& rect);

	//Updates the animation and gets the current frame
	SDL_Rect& GetCurrentFrame(float dt);
	
	//Returns the current frame number
	int GetCurrentFrame();

	//Check if the animation has finished
	bool Finished()const;

	//Reset the animation to its initial state
	void Restart();

	//Check if the animation is on a specific frame
	bool isInFrame(int x);

	//Static array to hold frames
	SDL_Rect frames[MAXIMUM_FRAMES];
	bool loop = true;
	float speed = 1.0f;
};
