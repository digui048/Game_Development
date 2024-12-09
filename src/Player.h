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

class Player : public Entity
{
public:

	Player(State state);
	
	virtual ~Player();

	bool Awake();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	Vector2D GetPosition() const;

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void TestWall(PhysBody* physA, PhysBody* physB);

	void TestPlatform(PhysBody* physA, PhysBody* physB);

	bool Start();

	void InitialState();

	void StartRunning();

	void StartJumping();

	void StartFalling();

	void StartDying();

	void DelayTime();

	bool Update(float dt);

	void SetPosition(Vector2D position);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

private:


public:

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH = 0;
	bool isGrounded_up = false;
	bool isGrounded_down = false;
	bool isWalled_left = false;
	bool isWalled_right = false;
	bool isAbove = false;
	
	//Audio fx
	int pickCoinFxId = 0;
	int jumpFxId;
	int fallFxId;
	int menuFxId;
	int walkingFxId;
	int checkpointFxId;


	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody = nullptr;
	b2Vec2 Pos = b2Vec2(0, 0);

	float jumpForce = 1.5f; 
	bool isJumping = false; 
	bool isFalling = false;
	
	bool menu = false;

	bool isDead = false;
	float respawnDelay = 3.0f;
	float deathTime = 0.0f;

	int numdeaths = 0;

	bool godMode = false;

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

	bool isRunningSoundPlaying;
};