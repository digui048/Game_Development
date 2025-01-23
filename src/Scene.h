#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Skeleton.h"
#include "FireSpirit.h"
#include "Boss.h"
#include "Log.h"
#include <vector>
#include "GuiControlButton.h"

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

	void PlayerDeath(bool death);

	Vector2D GetPlayerPosition();

	std::string LoadEnemyName(pugi::xml_node configParameters);

	Player* GetPlayer();

	void SetParameters(pugi::xml_node parameters);

	void LoadState();

	void SaveState();

	StatsManager* GetStatsManager();

	void StatsLooseLife();

	void StatsResetLife();

	bool checkpoint = false;
	float respawnDelayCam = 3.0f;
	float deathTimeCam = 0.0f;
	bool debugCamera = false;
	bool fpsTo30 = false;
	bool helpMenu = false;
	bool enemDebug = false;
	bool pathDebug = false;
	int helpmenuWidth;
	int helpmenuHeight;

	//sound fx
	int	menuFxId;


	// Handles multiple Gui Event methods
	bool OnGuiMouseClickEvent(GuiControl* control);


	// Get tilePosDebug value
	std::string GetTilePosDebug() {
		return tilePosDebug;
	}

	GuiControlButton* startbt;
	GuiControlButton* exitbt;
	GuiControlButton* settingsbt;

	GuiControlButton* layout;

	std::vector<GuiControlButton*> guiButtons;
	GuiControlButton* menuLayout;
	


private:
	SDL_Texture* mouseTileTex = nullptr;
	SDL_Texture* img;
	SDL_Texture* helpmenu;
	SDL_Texture* checkpointTex;
	Animation* checkpointAnim;
	Animation checkpointAnimData;
	Animation initialcheckpointAnimData;
	pugi::xml_node parameters_checkpoint;		// Parameters of the checkpoint
	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	std::vector<Enemy*> enemyList;
	Camera* camera;
	std::string tilePosDebug = "[0,0]";
	bool once = false;
	int checkpoint_loop = 0;

	// L16: TODO 2: Declare a GUI Control Button 
	GuiControlButton* guiBt;


	SDL_Texture* mainMenuTex;
};