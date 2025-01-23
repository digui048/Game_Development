#include "Boss.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"

Boss::Boss()
{
}

Boss::~Boss()
{
	delete pathfinding;
}

bool Boss::Start()
{
	// Initialize the enemy texture
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	death = parameters.attribute("active").as_bool();

	
	// Load animations
	idleAnim.LoadAnimations(parameters.child("animations").child("idle"));
	idle_left.LoadAnimations(parameters.child("animations").child("idle_left"));
	idle_right.LoadAnimations(parameters.child("animations").child("idle_right"));
	walk_left.LoadAnimations(parameters.child("animations").child("walk_left"));
	walk_right.LoadAnimations(parameters.child("animations").child("walk_right"));
	attack_left.LoadAnimations(parameters.child("animations").child("attack_left"));
	attack_right.LoadAnimations(parameters.child("animations").child("attack_right"));
	hit_left.LoadAnimations(parameters.child("animations").child("hit_left"));
	hit_right.LoadAnimations(parameters.child("animations").child("hit_right"));
	death_left.LoadAnimations(parameters.child("animations").child("death_left"));
	death_right.LoadAnimations(parameters.child("animations").child("death_right"));

	currentAnim = &idle_left;

	// Add a physics body to the enemy - initialise the physics body
	pbody = Engine::GetInstance().physics->CreateRectangle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW,texH, bodyType::DYNAMIC);
	attackLeft = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), (int)(texW * 1 / 5), (int)(texH * 7 / 6), bodyType::DYNAMIC);
	attackRight = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)position.getY(), (int)(texW * 3 / 5), (int)(texH * 7 / 6), bodyType::DYNAMIC);

	// Assign a collider to the physics body
	pbody->ctype = ColliderType::ENEMY;
	attackLeft->ctype = ColliderType::ENEMY_ATTACK_LEFT;
	attackRight->ctype = ColliderType::ENEMY_ATTACK_RIGHT;

	// Create joints
	pbody->CreateJoint(attackLeft, { (float)PIXEL_TO_METERS((texW)) ,(float)PIXEL_TO_METERS((-texH / 3)) });
	pbody->CreateJoint(attackRight, { (float)PIXEL_TO_METERS((-texW)) ,(float)PIXEL_TO_METERS((-texH / 3)) });

	// Set the collision listener
	pbody->listener = this;
	attackLeft->listener = this;
	attackRight->listener = this;

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Initialize the pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	enemydeathFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/enemydeath.wav");

	return true;
}

bool Boss::Update(float dt)
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
		// Get the next tile in the path
		Vector2D nextTile = pathfinding->pathTiles.back();
		Vector2D nextTileWorldPos = Engine::GetInstance().map.get()->MapToWorld(nextTile.getX(), nextTile.getY());
		Vector2D direction = nextTileWorldPos - GetPosition();

		if (dx < 0) look = View::RIGHT;
		else look = View::LEFT;

		direction = direction.normalized();
		b2Vec2 velocity = b2Vec2(direction.getX(), pbody->body->GetLinearVelocity().y);
		Walk();

		if (abs(dx) < 80)
		{
			Attack();

			CoolDown();

			if (canAttack && Engine::GetInstance().scene.get()->GetStatsManager()->GetLife() > 0 && (ctr >= maxctr)) {
				Engine::GetInstance().scene.get()->StatsLooseLife();
				ctr = 0;
			}
			else if (Engine::GetInstance().scene.get()->GetStatsManager()->GetLife() <= 0) {
				Engine::GetInstance().scene.get()->StatsResetLife();
				Engine::GetInstance().scene.get()->PlayerDeath(true);
			}

			if (currentAnim->HasFinished())
			{
				currentAnim->Reset();
			}
			velocity = b2Vec2(0, 0);
		}

		pbody->body->SetLinearVelocity(velocity);

	}

	Engine::GetInstance().scene.get()->GetPlayer()->lifeBoss = life;
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
	//LOG("Life: %d", life);
	return true;
}

bool Boss::CleanUp()
{
	Engine::GetInstance().physics.get()->DeletePhysBody(pbody);
	Engine::GetInstance().physics.get()->DeletePhysBody(attackLeft);
	Engine::GetInstance().physics.get()->DeletePhysBody(attackRight);
	return true;
}

void Boss::OnCollision(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collided with player - DESTROY");
		if ((physA->ctype == ColliderType::ENEMY_ATTACK_LEFT || physA->ctype == ColliderType::ENEMY_ATTACK_RIGHT)) {
			canAttack = true;
		}
		TestIsAbove(physA, physB);
		break;
	}
}

void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		LOG("Collision player");
		Engine::GetInstance().scene.get()->PlayerDeath(false);
		canAttack = false;
		break;
	}
}

void Boss::TestIsAbove(PhysBody* physA, PhysBody* physB)
{
	b2Transform transform_A = physA->body->GetTransform();
	b2Vec2 position_A = transform_A.p;

	b2Transform transform_B = physB->body->GetTransform();
	b2Vec2 position_B = transform_B.p;

	if (position_A.y > (position_B.y + 1.2f))
	{
		if (life <= 0) {
			Engine::GetInstance().audio.get()->PlayFx(enemydeathFxId);
			death = true;
			Engine::GetInstance().entityManager.get()->DestroyEntity(this);
		}
		else { 
			life -= 10;
		}
		
	}
	else {
		if (Engine::GetInstance().scene.get()->GetStatsManager()->GetLife() > 0)
			Engine::GetInstance().scene.get()->StatsLooseLife();
		else {
			Engine::GetInstance().scene.get()->StatsResetLife();
			Engine::GetInstance().scene.get()->PlayerDeath(true);
		}
	}
}
void Boss::Walk()
{
	if (look == View::LEFT)
	{
		//LOG("Walk left");
		currentAnim = &walk_right;
	}
	else if (look == View::RIGHT)
	{
		//LOG("Walk right");
		currentAnim = &walk_left;
	}

	state = State::RUN;
}

void Boss::Idle()
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

void Boss::Alert()
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

bool Boss::isAlert()
{
	return isAlerted;
}

void Boss::Attack()
{
	if (look == View::LEFT)
	{
		/*LOG("Attack left");*/
		currentAnim = &attack_right;
	}
	else if (look == View::RIGHT)
	{
		/*LOG("Attack right");*/
		currentAnim = &attack_left;
	}

	state = State::IDLE;
}

void Boss::Hit()
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

void Boss::Death()
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

void Boss::SetParameters(pugi::xml_node parameters)
{
	this->parameters = parameters;
}

void Boss::CoolDown()
{
	ctr += 0.150f;
}

void Boss::ResetPath()
{
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

