#include "SDL2/SDL.h"
#include "Engine.h"

class StatsManager {
public:
	StatsManager();
	~StatsManager();

	void IncrementLife(int life);
	void loseLife();
	void IncrementScore(int score);

	int GetLife();
	int GetScore();

	SDL_Rect UpdateLife();
	SDL_Rect GetBar() { return bar; }

private:
	int life;
	int score;
	SDL_Rect lifeRect;
	SDL_Rect bar;
};

