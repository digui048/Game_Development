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
	UNKNOWN
};
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

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void TestIsAbove(PhysBody* physA, PhysBody* physB);

private:

	void Walk();	// Walk state

	void Idle();	// Idle state
		
	void Alert();	// Alert state
	bool isAlert();	// Check if the enemy is alert

	void Attack();	// Attack state

	void Hit();		// Hit state

	void Death();	// Death state

	SDL_Texture* texture;			// Texture of the enemy
	const char* texturePath;		// Path to the texture
	int texW, texH;					// Texture width and height
	pugi::xml_node parameters;		// Parameters of the enemy
	bool isAlerted = false;			// Check if the enemy is alerted
	Animation* currentAnim;			// Current animation
	Animation idleAnim;				// Idle animation
	PhysBody* pbody;				// Physics body of the enemy
	Pathfinding* pathfinding;		// Pathfinding of the enemy
	State state;					// State of the enemy
	Animation idle_left;			// Idle left animation
	Animation idle_right;			// Idle right animation
	Animation walk_left;			// Run left animation
	Animation walk_right;			// Run right animation
	Animation alert_left;			// Alert left animation
	Animation alert_right;			// Alert right animation
	Animation attack_left;			// Attack left animation
	Animation attack_right;			// Attack right animation
	Animation hit_left;				// Hit left animation
	Animation hit_right;			// Hit right animation
	Animation death_left;			// Death left animation
	Animation death_right;			// Death right animation
};