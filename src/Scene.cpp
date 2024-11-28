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

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
	player = nullptr;
	camera = new Camera();
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

	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(900, 0);

	// Create a enemy using the entity manager 
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());
	helpmenu = Engine::GetInstance().textures.get()->Load("Assets/Textures/helpMenu.png");
	mouseTileTex = Engine::GetInstance().textures.get()->Load("Assets/Textures/mouse_tile.png");
	SDL_QueryTexture(helpmenu, NULL, NULL, &helpmenuWidth, &helpmenuHeight);
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

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
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
		helpMenu = !helpMenu;
		player->menu = !player->menu;
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
		enemDebug = !enemDebug;
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
	
	//L03 TODO 3: Make the camera movement independent of framerate
	//float camSpeed = 1;

	////if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	////	Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

	////if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	////	Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

	////if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	////	Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

	////if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	////	Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

	////Engine::GetInstance().render.get()->camera.x = -(player->position.getX()/6 + (Engine::GetInstance().render.get()->camera.w / 32));
	////Engine::GetInstance().render.get()->camera.y = -(player->position.getY()/6 + (Engine::GetInstance().render.get()->camera.y / 32));

	// //para hacer que la camara se mueva SMOOOOOOOTH ;)
	//float smoothValueX = 0.03f;
	//float smoothValueY = 0.04f;
	//
	//int screenWidth = Engine::GetInstance().render.get()->camera.w;		//tamaño screen
	//int screenHeight = Engine::GetInstance().render.get()->camera.h;
	//
	//float playerPosX = player->position.getX();		//posicion player
	//float playerPosY = player->position.getY();

	//float cameraPosX = Engine::GetInstance().render.get()->camera.x;	//posicion camara
	//float cameraPosY = Engine::GetInstance().render.get()->camera.y;

	//// a partir de donde sigue la camara
	//float cameraBoundary = screenWidth * 0.5f;
	//
	//// donde poner la camara para ir ajustandola
	//float targetCameraPosX = cameraPosX;
	//float targetCameraPosY = cameraPosY;

	//if (playerPosX > cameraPosX + cameraBoundary)
	//{
	//	targetCameraPosX = -(playerPosX - cameraBoundary);
	//	//LOG("right limit");
	//}
	//else if (playerPosX < cameraPosX - cameraBoundary)
	//{
	//	targetCameraPosX = -(playerPosX - cameraBoundary);
	//	//LOG("left limit");
	//}
	//if (playerPosY > cameraPosY + cameraBoundary)
	//{
	//	targetCameraPosY = -(playerPosY - cameraBoundary);
	//	//LOG("right limit");
	//}
	//else if (playerPosY < cameraPosY - cameraBoundary)
	//{
	//	targetCameraPosY = -(playerPosY - cameraBoundary);
	//	//LOG("left limit");
	//}

	//// retraso para movimiento SMOOOOOOOTH ;)
	//cameraPosX = cameraPosX + smoothValueX * (targetCameraPosX - cameraPosX);
	//cameraPosY = cameraPosY + smoothValueY * (targetCameraPosY - cameraPosY);
	////
	////
	//////delay para resetear la camara al morir
	////
	////float posX = player->Pos.x;
	////float posY = player->Pos.y;
	////Vector2D pos = Vector2D(posX, posY);
	////camera->Update(pos, dt);
	////Engine::GetInstance().render.get()->camera.x = (int)camera->position.getX();
	////LOG("Camera positionX: %d", Engine::GetInstance().render.get()->camera.x);
	////LOG("Camera positionY: %d", Engine::GetInstance().render.get()->camera.y);
	//
	//Engine::GetInstance().render.get()->camera.x = (int)cameraPosX;
	//Engine::GetInstance().render.get()->camera.y = (int)cameraPosY;

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
