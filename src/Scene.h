#pragma once

#include "Module.h"
#include "Player.h"
#include "Log.h"

struct SDL_Texture;

class Camera {
public:
    Vector2D position; 

    Camera() : position(0, 0) {}

    void Update(const Vector2D& playerPosition, float dt) {

        float cameraBoundary = 650; 
        float leftLimit = position.getX();
        float rightLimit = position.getX() + cameraBoundary;
		LOG("%d", playerPosition.getX());
        if (playerPosition.getX() > rightLimit) {
			LOG("%d, %d", playerPosition.getX(),rightLimit);
			position.setX(position.getX() + cameraBoundary/2);
        }
        else if (playerPosition.getX() < leftLimit) {
			LOG("%d, %d", playerPosition.getX(), rightLimit);
            position.setX(position.getX() - cameraBoundary/2);
        }
    }
};

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	//to manage delay for the camera to respawn with the character
	void DelayTimeCamera();

	Vector2D GetPlayerPosition();

	float respawnDelayCam = 3.0f;
	float deathTimeCam = 0.0f;
	bool debugCamera = false;
	bool fpsTo30 = false;
	bool helpMenu = false;
	int helpmenuWidth;
	int helpmenuHeight;

private:
	SDL_Texture* img;
	SDL_Texture* helpmenu;
	//L03: TODO 3b: Declare a Player attribute
	Player* player;

	Camera* camera;
};