#pragma once
#include "Enemy.h"


class Skeleton : public Enemy
{
public:
	Skeleton();
	~Skeleton();

	bool Start() override;

	bool Update(float dt) override;

	bool CleanUp() override;

	void OnCollision(PhysBody* physA, PhysBody* physB) override;

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB) override;

	void TestIsAbove(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters);

	void ResetPath();

private:

	bool isAlerted = false;			// Check if the enemy is alerted
	SDL_Texture* texture;			// Texture of the enemy
	Animation* currentAnim;			// Current animation
	Animation idleAnim;				// Idle animation
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
	
	void Idle();
	void Walk();
	void Alert();
	bool isAlert();
	void Attack();
	void Hit();
	void Death();
	// Death right animation

	int enemydeathFxId;
};