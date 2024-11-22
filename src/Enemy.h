#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

struct SDL_Texture;

class Enemy : public Entity
{
public:
	Enemy();
	virtual ~Enemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters);

	void SetPosition(Vector2D position);
	
	void ResetPath();

	Vector2D GetPosition() const;

private:
	
	SDL_Texture* texture;			// Texture of the enemy
	const char* texturePath;		// Path to the texture
	int texW, texH;					// Texture width and height
	pugi::xml_node parameters;		// Parameters of the enemy
	Animation* currentAnim;			// Current animation
	Animation idleAnim;				// Idle animation
	PhysBody* pbody;				// Physics body of the enemy
	Pathfinding* pathfinding;		// Pathfinding of the enemy
};