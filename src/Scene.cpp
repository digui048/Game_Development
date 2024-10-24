#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
	player = nullptr;
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
	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER, View::RIGHT);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	//L08 Create a new item using the entity manager and set the position to (200, 672) to test
	Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM, View::RIGHT);
	item->position = Vector2D(0, 0);
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	//L06 TODO 3: Call the function to load the map. 
	Engine::GetInstance().map->Load(configParameters.child("map").attribute("path").as_string(), configParameters.child("map").attribute("name").as_string());

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{

	//L03 TODO 3: Make the camera movement independent of framerate
	//float camSpeed = 1;

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.y += ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.y -= ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.x += ceil(camSpeed * dt);

	//if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	//	Engine::GetInstance().render.get()->camera.x -= ceil(camSpeed * dt);

	//Engine::GetInstance().render.get()->camera.x = -(player->position.getX()/6 + (Engine::GetInstance().render.get()->camera.w / 32));
	//Engine::GetInstance().render.get()->camera.y = -(player->position.getY()/6 + (Engine::GetInstance().render.get()->camera.y / 32));

	//return true;

	// para hacer que la camara se mueva SMOOOOOOOTH ;)
	float smoothValue = 0.03f;
	
	int screenWidth = Engine::GetInstance().render.get()->camera.w;		//tamaño screen
	int screenHeight = Engine::GetInstance().render.get()->camera.h;
	
	float playerPosX = player->position.getX();		//posicion player
	float playerPosY = player->position.getY();

	float cameraPosX = Engine::GetInstance().render.get()->camera.x;	//posicion camara
	float cameraPosY = Engine::GetInstance().render.get()->camera.y;

	// a partir de donde sigue la camara
	float cameraBoundary = screenWidth * 0.5f;
	
	// donde poner la camara para ir ajustandola
	float targetCameraPosX = cameraPosX;
	float targetCameraPosY = cameraPosY;

	if (playerPosX > cameraPosX + cameraBoundary)
	{
		targetCameraPosX = -(playerPosX - cameraBoundary);
		/*LOG("right limit");*/
	}
	else if (playerPosX < cameraPosX - cameraBoundary)
	{
		targetCameraPosX = -(playerPosX - cameraBoundary);
		//LOG("left limit");
	}
	if (playerPosY > cameraPosY + cameraBoundary)
	{
		targetCameraPosY = -(playerPosY - cameraBoundary);
		//LOG("right limit");
	}
	else if (playerPosY < cameraPosY - cameraBoundary)
	{
		targetCameraPosY = -(playerPosY - cameraBoundary);
		//LOG("left limit");
	}

	// retraso para movimiento SMOOOOOOOTH ;)
	cameraPosX = cameraPosX + smoothValue * (targetCameraPosX - cameraPosX);
	cameraPosY = cameraPosY + smoothValue * (targetCameraPosY - cameraPosY);
	
	Engine::GetInstance().render.get()->camera.x = (int)cameraPosX;
	Engine::GetInstance().render.get()->camera.y = (int)cameraPosY;

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

	return true;
}
