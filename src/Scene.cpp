#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "Enemy.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "GuiControlButton.h"


Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
	player = nullptr;
	camera = new Camera();
	checkpointAnim = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	//L04: TODO 3b: Instantiate the player using the entity manager
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
		
	

	//PRUEBAS BOTONES ----------------------------------------------------------------------------------------------------------------------------------------------------
	SDL_Rect startbutton = { (Engine::GetInstance().window.get()->width / 2) - 60, 300, 120,20 };
	SDL_Rect settingbutton = { (Engine::GetInstance().window.get()->width / 2) - 60, 350, 120,20 };
	SDL_Rect exitbutton = { (Engine::GetInstance().window.get()->width / 2) - 60, 400, 120,20 };

	/*SDL_Rect layoutBoundsMM = { 0, 0, Engine::GetInstance().window.get()->width, Engine::GetInstance().window.get()->height };
	menuLayout = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 0, "Layout", layoutBoundsMM, this);
	menuLayout->isLayout = true;
	menuLayout->isMenu = true;
	menuLayout->SetTexture(mainMenuTex);*/

	/*startbt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "START", startbutton, this);
	startbt->Isvisible = true;
	guiButtons.push_back(startbt);

	settingsbt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 2, "SETTINGS", settingbutton, this);
	settingsbt->Isvisible = true;
	guiButtons.push_back(settingsbt);

	exitbt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 3, "EXIT", exitbutton, this);
	exitbt->Isvisible = true;
	guiButtons.push_back(exitbt);*/
	//PRUEBAS BOTONES ----------------------------------------------------------------------------------------------------------------------------------------------------


	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		std::string name = enemyNode.attribute("name").as_string();
		if (name == "skeleton") {
			if (enemyNode.attribute("active").as_bool() == false) {
				Skeleton* enemy = (Skeleton*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SKELETON);
				enemy->SetParameters(enemyNode);
				enemyList.push_back(enemy);
			}
		}
		else if (name == "firespirit") {
			FireSpirit* enemy = (FireSpirit*)Engine::GetInstance().entityManager->CreateEntity(EntityType::FIRE_SPIRIT);
			enemy->SetParameters(enemyNode);
			enemyList.push_back(enemy);
		}
		else if (name == "boss") {
			Boss* enemy = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
			enemy->SetParameters(enemyNode);
			enemyList.push_back(enemy);
		}

		//FireSpirit* enemy2 = (FireSpirit*)Engine::GetInstance().entityManager->CreateEntity(EntityType::FIRE_SPIRIT);
		//enemy2->SetParameters(enemyNode);
		//enemyList.push_back(enemy2);
	}

	// L16: TODO 2: Instantiate a new GuiControlButton in the Scene
	//SDL_Rect btPos = { 520, 520, 120, 25 };
	//guiBt = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, 1, "BOTÓN PRUEBA", btPos, this);

	//Load sounds
	menuFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/menu.wav");
	bossFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Music/boss.ogg");
	

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	helpmenu = Engine::GetInstance().textures.get()->Load("Assets/Textures/helpMenu.png");
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Textures/mouse_tile.png");

	for (pugi::xml_node checkpointNode = configParameters.child("sprites").child("checkpoint"); checkpointNode; checkpointNode = checkpointNode.next_sibling("checkpoint"))
	{
		const char* checkpoint_path = checkpointNode.attribute("texture").as_string();
		checkpointTex = Engine::GetInstance().textures.get()->Load(checkpoint_path);
		checkpointAnimData.LoadAnimations(checkpointNode.child("animations").child("idle"));
		initialcheckpointAnimData.LoadAnimations(checkpointNode.child("animations").child("initial"));
	}
	SDL_QueryTexture(helpmenu, NULL, NULL, &helpmenuWidth, &helpmenuHeight);

	mainMenuTex = Engine::GetInstance().textures.get()->Load("Assets/Menus & UI/Title screen.png");	

	//Play background music
	Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/bg_song.ogg");

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

//to manage delay for the camera to respawn with the character
void Scene::DelayTimeCamera() {

	deathTimeCam += 0.031f;
}

void Scene::PlayerDeath(bool death)
{
	player->isDead = death;
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}

std::string Scene::LoadEnemyName(pugi::xml_node configParameters)
{
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		std::string enemyName = enemyNode.child("name").text().as_string();
		return enemyName;
	}
}

Player* Scene::GetPlayer()
{
	return player;
}

void Scene::LoadState()
{
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");
	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}
	pugi::xml_node sceneNode = loadFile.child("config").child("scene");
	//Read XML and restore information
	//Player position
	Vector2D playerPos = Vector2D(sceneNode.child("entities").child("player").attribute("x").as_int(),
		sceneNode.child("entities").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);
	//enemies
	pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
	for (pugi::xml_node enemyNode = enemiesNode.child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy")) {
		for (auto& enemy : enemyList) {
			if (enemiesNode.attribute("active").as_bool() == true) {
				enemy->SetPosition(Vector2D(enemyNode.child("position").attribute("x").as_float(),
					enemyNode.child("position").attribute("y").as_float()));
			}
		}
	}
}

void Scene::SaveState()
{
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");
	if (result == NULL)
	{
		LOG("Could not load file. Pugi error: %s", result.description());
		return;
	}
	pugi::xml_node sceneNode = loadFile.child("config").child("scene");
	//Save info to XML 
	//Player position
	sceneNode.child("entities").child("player").attribute("x").set_value(player->GetPosition().getX());
	sceneNode.child("entities").child("player").attribute("y").set_value(player->GetPosition().getY());
	//enemies
	pugi::xml_node enemiesNode = sceneNode.child("entities").child("enemies");
	if (!enemiesNode) {
		enemiesNode = sceneNode.child("entities").child("enemies");
	}
	for (auto& enemy : enemyList) {
		pugi::xml_node enemyNode = enemiesNode.child("enemy");
		if (enemy->Death() == false) {
			enemyNode.attribute("x").set_value(enemy->GetPosition().getX());
			enemyNode.attribute("y").set_value(enemy->GetPosition().getY());
		}
		else if (enemy->Death() == true) {
			enemyNode.attribute("active").set_value(true);
		}
	}
	
	//Saves the modifications to the XML 
	loadFile.save_file("config.xml");
}

StatsManager* Scene::GetStatsManager()
{
	return player->stats;
}

void Scene::StatsLooseLife()
{
	player->stats->loseLife();
}

void Scene::StatsResetLife()
{
	player->stats->IncrementLife(100);
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	//Get mouse position and obtain the map coordinate
	Vector2D mousePos = Engine::GetInstance().input.get()->GetMousePosition();
	Vector2D mouseTile = Engine::GetInstance().map.get()->WorldToMap(mousePos.getX() - Engine::GetInstance().render.get()->camera.x / Engine::GetInstance().window.get()->GetScale(),
																	 mousePos.getY() - Engine::GetInstance().render.get()->camera.y / Engine::GetInstance().window.get()->GetScale());
	//Get the tile position in the map
	Vector2D highlightTile = Engine::GetInstance().map.get()->MapToWorld(mouseTile.getX(), mouseTile.getY());
	SDL_Rect rect = { 0,0,32,32 };

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		player->godMode = !player->godMode;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F11) == KEY_DOWN) {
		fpsTo30 = !fpsTo30;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F8) == KEY_DOWN) {
		debugCamera = !debugCamera;
	}
	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
		//play helpmenu sound
		Engine::GetInstance().audio.get()->PlayFx(menuFxId);

		helpMenu = !helpMenu;
		player->menu = !player->menu;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
		enemDebug = !enemDebug;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F4) == KEY_DOWN) {
		pathDebug = !pathDebug;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		LoadState();
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		SaveState();
	}
	if (fpsTo30) {
		Engine::GetInstance().FPSCapto(32);
	}
	else if (!fpsTo30)
	{
		Engine::GetInstance().FPSCapto(16);
	}
	if (!debugCamera) {
		float camSpeed = Engine::GetInstance().window.get()->scale;
		float smoothFactor = 0.1f; // smoothness

		// target camera position
		float targetCamX = (-player->position.getX() * camSpeed) + Engine::GetInstance().window.get()->width / 2;
		float targetCamY = (-player->position.getY() * camSpeed) + Engine::GetInstance().window.get()->height / 2;

		// camera position towards the target position
		Engine::GetInstance().render.get()->camera.x += (targetCamX - Engine::GetInstance().render.get()->camera.x) * smoothFactor;
		Engine::GetInstance().render.get()->camera.y += (targetCamY - Engine::GetInstance().render.get()->camera.y) * smoothFactor;
	}
	else if (debugCamera) {
		float camSpeed = 1;

		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

		if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);
	}
	if (helpMenu) {
		//el centro de la window
		int windowWidth = Engine::GetInstance().window.get()->width;
		int windowHeight = Engine::GetInstance().window.get()->height;

		//donde se tiene que dibujar
		int centerX = player->position.getX();
		int centerY = player->position.getY();

		int drawX = centerX - (helpmenuWidth / 2);
		int drawY = centerY - (helpmenuHeight / 2);

		Engine::GetInstance().render.get()->DrawTexture(helpmenu, drawX, drawY);
		/*Engine::GetInstance().render.get()->DrawTexture(helpmenu, (player->position.getX()) - Engine::GetInstance().window.get()->width / 10,
			(player->position.getY())- Engine::GetInstance().window.get()->height / 5);*/
	}
	if (player->isDead)
	{	//en un futuro habra que poner la camara en las posiciones de los checkpoints
		DelayTimeCamera();

		if (deathTimeCam >= respawnDelayCam) {

			LOG("Reset Camera");
			Engine::GetInstance().render.get()->camera.x = 0;
			Engine::GetInstance().render.get()->camera.y = 0;
			deathTimeCam = 0.0f;
		}

		StatsResetLife();

	}

	//Render a texture where the mouse is over to highlight the tile, use the texture 'mouseTileTex'
	if(enemDebug)
	{
		Engine::GetInstance().render.get()->DrawTexture(mouseTileTex, highlightTile.getX(), highlightTile.getY(), &rect);
		
		//If mouse button is pressed modify enemy position
		if (Engine::GetInstance().input.get()->GetMouseButtonDown(1) == KEY_DOWN) {
			enemyList[0]->SetPosition(Vector2D(highlightTile.getX(), highlightTile.getY()));
			enemyList[0]->ResetPath();
		}
	}

	// saves the tile pos for debugging purposes
	if (mouseTile.getX() >= 0 && mouseTile.getY() >= 0 || once) {
		tilePosDebug = "[" + std::to_string((int)mouseTile.getX()) + "," + std::to_string((int)mouseTile.getY()) + "] ";
		once = true;
	}

	if (checkpoint && checkpoint_loop != 0)
	{
		checkpointAnim = &checkpointAnimData;
		Engine::GetInstance().render.get()->DrawTexture(checkpointTex, 21 * 32, 27 * 32, &checkpointAnim->GetCurrentFrame());
	}
	else if (checkpoint && checkpoint_loop == 0)
	{
		SaveState();
		checkpointAnim = &checkpointAnimData;
		Engine::GetInstance().render.get()->DrawTexture(checkpointTex, 21 * 32, 27 * 32, &checkpointAnim->GetCurrentFrame());
		checkpoint_loop++;
	}
	else {
		checkpointAnim = &initialcheckpointAnimData;
		Engine::GetInstance().render.get()->DrawTexture(checkpointTex, 21 * 32, 27 * 32, &checkpointAnim->GetCurrentFrame());
	}

	Engine::GetInstance().render.get()->DrawText("Life", 25, 5, 50, 50);
	Engine::GetInstance().render.get()->DrawRectangle(player->stats->GetBar(), 255, 255, 255, 255, true, false);
	Engine::GetInstance().render.get()->DrawRectangle(player->stats->UpdateLife(), 0, 255, 0, 255,true,false);
	
	// Rectangle Life
	int maxLife = 100;
	int life = GetPlayer()->lifeBoss;
	float health = (float)life / maxLife;  // Ensure floating-point division
	SDL_Rect lifeRect = player->stats->UpdateLife();
	lifeRect.w = 350 * health;    // Bar.width is assumed to be float already
	lifeRect.y = 60;

	SDL_Rect bar_life = player->stats->GetBar();
	bar_life.y = 56;
	Engine::GetInstance().render.get()->DrawText("Boss Life", 25, 50, 70, 50);
	Engine::GetInstance().render.get()->DrawRectangle(bar_life, 255, 255, 255, 255, true, false);
	Engine::GetInstance().render.get()->DrawRectangle(lifeRect, 0, 255, 0, 255, true, false);

	//Check player position to play music
	if (player->position.getX() > 3136 && !bossMusicPlayed)
	{
		Engine::GetInstance().audio.get()->StopMusic(); // Stop any currently playing music
		Engine::GetInstance().audio.get()->PlayMusic("Assets/Audio/Music/boss.ogg");
		bossMusicPlayed = true;
	}

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);
	SDL_DestroyTexture(helpmenu);

	delete camera;

	return true;
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method
	LOG("Press Gui Control: %d", control->id);

	//Engine::GetInstance().guiManager->DeleteButtons();

	return true;
}