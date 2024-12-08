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
	idle_left.LoadAnimations(parameters.child("animations").child("idle_left"));
	idle_right.LoadAnimations(parameters.child("animations").child("idle_right"));
	walk_left.LoadAnimations(parameters.child("animations").child("walk_left"));
	walk_right.LoadAnimations(parameters.child("animations").child("walk_right"));
	alert_left.LoadAnimations(parameters.child("animations").child("alert_left"));
	alert_right.LoadAnimations(parameters.child("animations").child("alert_right"));
	attack_left.LoadAnimations(parameters.child("animations").child("attack_left"));
	attack_right.LoadAnimations(parameters.child("animations").child("attack_right"));
	hit_left.LoadAnimations(parameters.child("animations").child("hit_left"));
	hit_right.LoadAnimations(parameters.child("animations").child("hit_right"));
	death_left.LoadAnimations(parameters.child("animations").child("death_left"));
	death_right.LoadAnimations(parameters.child("animations").child("death_right"));

	currentAnim = &idle_left;

	// Add a physics body to the enemy - initialise the physics body
	pbody = Engine::GetInstance().physics->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW, texH, bodyType::DYNAMIC);

	// Assign a collider to the physics body
	pbody->ctype = ColliderType::ENEMY;

	// Set the collision listener
	pbody->listener = this;

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
	int steps = 0;
	while (pathfinding->pathTiles.empty() && steps < 100)
	{
		steps++;
		pathfinding->PropagateAStar(SQUARED);
	}

	// Ground enemy
	float dx = Engine::GetInstance().scene.get()->GetPlayerPosition().getX() - GetPosition().getX();
	// Update the enemy state
	if (!pathfinding->pathTiles.empty())
	{
		if (isAlert()) {
			// Get the next tile in the path
			Vector2D nextTile = pathfinding->pathTiles.back();
			Vector2D nextTileWorldPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());
			Vector2D direction = nextTileWorldPos - GetPosition();

			if (dx < 0) look = View::RIGHT;
			else look = View::LEFT;

			direction = direction.normalized();
			b2Vec2 velocity = b2Vec2(direction.getX(), pbody->body->GetLinearVelocity().y);
			Walk();

			if (abs(dx) > 300) {
				isAlerted = false;
				Idle();
				velocity = b2Vec2(0, 0);
			}
			else if (abs(dx) < 30)
			{
				Attack();
				if (currentAnim->HasFinished())
				{
					currentAnim->Reset();
				}
				velocity = b2Vec2(0, 0);
			}

			pbody->body->SetLinearVelocity(velocity);
		}
		else {
			// If player is in range, alert the enemy
			if (abs(dx) < 150) {
				if (dx < 0) look = View::LEFT;
				else look = View::RIGHT;
				Alert();
				if (currentAnim->HasFinished()) {
 					isAlerted = true;
					currentAnim->Reset();
				}
				pbody->body->SetLinearVelocity(b2Vec2(0, 0));
			}
			else {
				if (dx < 0) look = View::LEFT;
				else look = View::RIGHT;
				Idle();
			}
		}
	}

	//// Flying enemy
	//float dx = Engine::GetInstance().scene.get()->GetPlayerPosition().magnitude() - GetPosition().magnitude();
	//// Update the enemy state
	//if (!pathfinding->pathTiles.empty())
	//{
	//	if (isAlert()) {
	//		// Get the next tile in the path
	//		Vector2D nextTile = pathfinding->pathTiles.back();
	//		Vector2D nextTileWorldPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());
	//		Vector2D direction = nextTileWorldPos - GetPosition();

	//		if (dx < 0) look = View::RIGHT;
	//		else look = View::LEFT;

	//		direction = direction.normalized();
	//		b2Vec2 velocity = b2Vec2(direction.getX(), direction.getY());
	//		Walk();

	//		if (abs(dx) > 300) {
	//			isAlerted = false;
	//			Idle();
	//			velocity = b2Vec2(0, 0);
	//		}
	//		else if (abs(dx) < 30)
	//		{
	//			Attack();
	//			if (currentAnim->HasFinished())
	//			{
	//				currentAnim->Reset();
	//			}
	//			velocity = b2Vec2(0, 0);
	//		}

	//		pbody->body->SetLinearVelocity(velocity);
	//	}
	//	else {
	//		// If player is in range, alert the enemy
	//		if (abs(dx) < 150) {
	//			if (dx < 0) look = View::LEFT;
	//			else look = View::RIGHT;
	//			Alert();
	//			if (currentAnim->HasFinished()) {
	//				isAlerted = true;
	//				currentAnim->Reset();
	//			}
	//			pbody->body->SetLinearVelocity(b2Vec2(0, 0));
	//		}
	//		else {
	//			if (dx < 0) look = View::LEFT;
	//			else look = View::RIGHT;
	//			Idle();
	//		}
	//	}
	//}

	// Update the enemy position
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Draw the enemy
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnim->GetCurrentFrame());
	currentAnim->Update();

	// Draw the path
	if (Engine::GetInstance().scene.get()->pathDebug) {
		pathfinding->DrawPath();
	}
	return true;
}

bool Enemy::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
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

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player - DESTROY");
		TestIsAbove(physA, physB);
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		Engine::GetInstance().scene.get()->PlayerDeath(false);
		break;
	}
}

void Enemy::TestIsAbove(PhysBody* physA, PhysBody* physB)
{
	b2Transform transform_A = physA->body->GetTransform();
	b2Vec2 position_A = transform_A.p;

	b2Transform transform_B = physB->body->GetTransform();
	b2Vec2 position_B = transform_B.p;

	if (position_A.y > position_B.y)
	{
		Engine::GetInstance().entityManager.get()->DestroyEntity(this);
	}
	else {
		Engine::GetInstance().scene.get()->PlayerDeath(true);
	}
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
		//LOG("Walk left");
		currentAnim = &walk_left;
	}
	else if (look == View::RIGHT)
	{
		//LOG("Walk right");
		currentAnim = &walk_right;
	}

	state = State::RUN;
}

void Enemy::Idle()
{
	if (look == View::LEFT)
	{
		//LOG("Idle left");
		currentAnim = &idle_left;
	}
	else if (look == View::RIGHT)
	{
		//LOG("Idle right");
		currentAnim = &idle_right;
	}

	state = State::IDLE;
}

void Enemy::Alert()
{
	if (look == View::LEFT)
	{
		LOG("Alert left");
		currentAnim = &alert_left;
	}
	else if (look == View::RIGHT)
	{
		LOG("Alert right");
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
		LOG("Attack left");
		currentAnim = &attack_left;
	}
	else if (look == View::RIGHT)
	{
		LOG("Attack right");
		currentAnim = &attack_right;
	}

	state = State::IDLE;
}

void Enemy::Hit()
{
	if (look == View::LEFT)
	{
		LOG("Hit left");
		currentAnim = &hit_left;
	}
	else if (look == View::RIGHT)
	{
		LOG("Hit right");
		currentAnim = &hit_right;
	}

	state = State::IDLE;
}

void Enemy::Death()
{
	if (look == View::LEFT)
	{
		LOG("Death left");
		currentAnim = &death_left;
	}
	else if (look == View::RIGHT)
	{
		LOG("Death right");
		currentAnim = &death_right;
	}

	state = State::IDLE;
}




