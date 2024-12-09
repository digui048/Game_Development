#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include "EntityManager.h"
struct SDL_Texture;
enum class EnemyType
{
	SKELETON,
	FIRE_SPIRIT,
	UNKNOWN
};
class Enemy : public Entity
{
public:
	Enemy();
	virtual ~Enemy();

	bool Awake();

	virtual bool Start();

	virtual bool Update(float dt);

	virtual bool CleanUp();
	
	void ResetPath();

	void SetPosition(Vector2D position);

	Vector2D GetPosition() const;

	virtual void OnCollision(PhysBody* physA, PhysBody* physB);

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool Death();


protected:

	PhysBody* pbody;				// Physics body of the enemy
	const char* texturePath;		// Path to the texture
	int texW, texH;					// Texture width and height
	bool death;						// If the enemy is dead
	pugi::xml_node parameters;		// Parameters of the enemy
	Animation* currentAnim;			// Current animation
	Animation idleAnim;				// Idle animation
	Pathfinding* pathfinding;		// Pathfinding of the enemy
};