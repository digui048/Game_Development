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

Enemy::Enemy() : Entity(EntityType::SKELETON,View::RIGHT)
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
	return true;
}

bool Enemy::Update(float dt)
{
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

	// Draw the enemy

	// Draw the path
	
	return true;
}

bool Enemy::CleanUp()
{
	return true;
}

void Enemy::ResetPath()
{
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

Vector2D Enemy::GetPosition() const
{
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	return Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
}

void Enemy::SetPosition(Vector2D position)
{
	position.setX(position.getX());
	position.setY(position.getY());
	b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY()));
	pbody->body->SetTransform(pos, 0);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) 
{
	
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	
}

bool Enemy::Death()
{
	return death;
}




