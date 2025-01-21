#include "LifeManager.h"


StatsManager::StatsManager(): score(0), life(100)
{
	//Rectangle bar
	bar.x = 120;
	bar.y = 13;
	bar.w = 366;
	bar.h = 27;

	//Rectangle of the life
	lifeRect.x = 128;
	lifeRect.y = 17;
	lifeRect.w = 350;
	lifeRect.h = 20;
}

StatsManager::~StatsManager()
{
}

void StatsManager::IncrementScore(int score)
{
	this->score += score;
}

void StatsManager::IncrementLife(int life)
{
	this->life = life;
	if (this->life > 100) this->life = 100;
}

void StatsManager::loseLife()
{
	this->life -= 10;
	if (this->life < 0) this->life = 0;
}

int StatsManager::GetScore()
{
	return score;
}

int StatsManager::GetLife()
{
	return life;
}

SDL_Rect StatsManager::UpdateLife()
{
	// Rectangle Life
	int maxLife = 100;
	int life = GetLife();
	float health = (float)life / maxLife;  // Ensure floating-point division
	lifeRect.w = 350 * health;    // Bar.width is assumed to be float already
	return lifeRect;
}
