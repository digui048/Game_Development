#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Box2D/Box2D.h"
#include "Animation.h"

struct SDL_Texture;

enum class PlayerAnim {
	IDLE_LEFT, IDLE_RIGHT,
	WALKING_LEFT, WALKING_RIGHT,
	JUMPING_LEFT, JUMPING_RIGHT,
	LEVITATING_LEFT, LEVITATING_RIGHT,
	FALLING_LEFT, FALLING_RIGHT,
	NUM_ANIMATIONS
};

enum class State {
	IDLE, RUN, JUMP, FALL, DIE
};

class Player : public Entity
{
public:

	Player(State state);
	
	virtual ~Player();

	bool Awake();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void CheckIdle();

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void TestWall(PhysBody* physA, PhysBody* physB);

	void TestPlatform(PhysBody* physA, PhysBody* physB);

	void TestSpike(PhysBody* physA, PhysBody* physB);

	void TestCornerPlatform(PhysBody* physA, PhysBody* physB);

	bool Start();

	void InitialState();

	void StartRunning();

	void StartJumping();

	void StartFalling();

	void StartDying();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

private:


public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;
	bool isGrounded_up = false;
	bool isGrounded_down = false;
	bool isWalled_left = false;
	bool isWalled_right = false;
	bool isDead = false;
	bool corner_right = false;
	bool corner_left = false;
	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;

	float jumpForce = 2.f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	bool isFalling = false;

	State state;

	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle_left;
	Animation idle_right;
	Animation run_left;
	Animation run_right;
	Animation jump_left;
	Animation jump_right;
	Animation fall_left;
	Animation fall_right;
	Animation die_left;
	Animation die_right;
};