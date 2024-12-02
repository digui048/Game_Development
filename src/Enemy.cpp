#include "Enemy.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"

Enemy::Enemy() : Entity(EntityType::ENEMY,View::RIGHT)
{
	name = "enemy";
}

Enemy::~Enemy()
{
	delete pathfinding;
}

bool Enemy::Awake()
{
	return true;
}

bool Enemy::Start()
{
	// Initialize the enemy texture
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	// Load animations
	idleAnim.LoadAnimations(parameters.child("animations").child("idle"));
	currentAnim = &idleAnim;

	// Add a physics body to the enemy - initialise the physics body
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	// Assign a collider to the physics body
	pbody->ctype = ColliderType::ENEMY;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Initialize the pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}

bool Enemy::Update(float dt)
{
	// Pathfinding A* algorithm with different heuistics (Manhattan, Euclidean, Squared)
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_DOWN) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_B) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(MANHATTAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_DOWN) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_N) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(EUCLIDEAN);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_DOWN) {
		pathfinding->PropagateAStar(SQUARED);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_M) == KEY_REPEAT &&
		Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		pathfinding->PropagateAStar(SQUARED);
	}
	
	// Reset the path
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_R) == KEY_DOWN) {
		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
		pathfinding->ResetPath(tilePos);
	}

	ResetPath();
	while (pathfinding->pathTiles.empty())
	{
		pathfinding->PropagateAStar(SQUARED);
	}

	// Move towards the next tile in the path
	if (!pathfinding->pathTiles.empty()) {
		Vector2D nextTile = pathfinding->pathTiles.front();
		Vector2D nextTileWorldPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());
		Vector2D direction = nextTileWorldPos - GetPosition();

		if (direction.magnitude() > 1.0f) {
			direction = direction.normalized();
			b2Vec2 velocity = b2Vec2(direction.getX(), pbody->body->GetLinearVelocity().y);
			pbody->body->SetLinearVelocity(velocity);
		}
		else {
			// Reached the next tile, remove it from the path
			pathfinding->pathTiles.pop_front();
		}
	}
	else {
		// Stop the enemy if there are no more tiles in the path
		pbody->body->SetLinearVelocity(b2Vec2(0, 0));
	}

	// Update the enemy position
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Draw the enemy
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnim->GetCurrentFrame());
	currentAnim->Update();

	// Draw the path
	pathfinding->DrawPath();

	return true;
}

bool Enemy::CleanUp()
{
	return true;
}

void Enemy::SetParameters(pugi::xml_node parameters)
{
	this->parameters = parameters;
}

void Enemy::SetPosition(Vector2D position)
{
	position.setX(position.getX() + texW / 2);
	position.setY(position.getY() + texH / 2);
	b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY()));
	pbody->body->SetTransform(pos, 0);
}

Vector2D Enemy::GetPosition() const
{
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	return Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
}

void Enemy::ResetPath()
{
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::Walk()
{
	if (look == View::LEFT)
	{
		currentAnim = &walk_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &walk_right;
	}

	state = State::RUN;
}

void Enemy::Idle()
{
	if (look == View::LEFT)
	{
		currentAnim = &idle_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &idle_right;
	}

	state = State::IDLE;
}

void Enemy::Alert()
{
	if (look == View::LEFT)
	{
		currentAnim = &alert_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &alert_right;
	}

	state = State::IDLE;
}

bool Enemy::isAlert()
{
	return isAlerted;
}

void Enemy::Attack()
{
	if (look == View::LEFT)
	{
		currentAnim = &attack_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &attack_right;
	}

	state = State::IDLE;
}

void Enemy::Hit()
{
	if (look == View::LEFT)
	{
		currentAnim = &hit_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &hit_right;
	}

	state = State::IDLE;
}

void Enemy::Death()
{
	if (look == View::LEFT)
	{
		currentAnim = &death_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnim = &death_right;
	}

	state = State::IDLE;
}




