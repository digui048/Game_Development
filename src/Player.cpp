#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Player::Player(State state) : Entity(EntityType::PLAYER, View::RIGHT)
{
	name = "Player";
	this->state = state;
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(0, 0);
	return true;
}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();
	
	//Load animations
	
	//Idle right
	idle_right.LoadAnimations(parameters.child("animations").child("idle_right"));
	//Idle left
	idle_left.LoadAnimations(parameters.child("animations").child("idle_left"));
	//Run
	run_right.LoadAnimations(parameters.child("animations").child("run_right"));
	run_left.LoadAnimations(parameters.child("animations").child("run_left"));
	//Jump
	jump_right.LoadAnimations(parameters.child("animations").child("jump_right"));
	jump_left.LoadAnimations(parameters.child("animations").child("jump_left"));
	//Fall
	fall_right.LoadAnimations(parameters.child("animations").child("fall_right"));
	fall_left.LoadAnimations(parameters.child("animations").child("fall_left"));
	//Die
	die_right.LoadAnimations(parameters.child("animations").child("die_right"));
	die_left.LoadAnimations(parameters.child("animations").child("die_left"));

	currentAnimation = &idle_right;

	// L08 TODO 5: Add physics to the player - initialize physics body
	/*Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);*/
	pbody = Engine::GetInstance().physics.get()->CreateRectangle((int)position.getX(), (int)position.getY(), (int)(texW/2.5f),(int)(texH/1.25f), bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");
	jumpFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/jump.wav");
	fallFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/fall.wav");
	walkingFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/step.wav");

	return true;
}

void Player::StartRunning()
{	

	if (look == View::LEFT)
	{
		currentAnimation = &run_left;
	}
	else if(look == View::RIGHT)
	{
		currentAnimation = &run_right;
	}

	state = State::RUN;
}

void Player::StartJumping()
{
	state = State::JUMP;
	
	if (look == View::LEFT)
	{
		currentAnimation = &jump_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnimation = &jump_right;
	}
}

void Player::StartFalling()
{
	//LOG("%s","FALL");
	state = State::FALL;

	if (look == View::LEFT)
	{
		currentAnimation = &fall_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnimation = &fall_right;
	}
}

void Player::StartDying()
{
	state = State::DIE;
	
	if (look == View::LEFT)
	{
		currentAnimation = &die_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnimation = &die_right;
	}
}

void Player::DelayTime()
{
	deathTime += 0.031f;
}

void Player::InitialState()
{
	//LOG("%s", "IDLE");
	state = State::IDLE;

	if (look == View::LEFT)
	{
		currentAnimation = &idle_left;
	}
	else if (look == View::RIGHT)
	{
		currentAnimation = &idle_right;
	}
}

bool Player::Update(float dt)
{
	b2Vec2 velocity = b2Vec2(0, pbody->body->GetLinearVelocity().y);

	if (godMode && !menu)
	{
		pbody->body->SetType(b2_kinematicBody);

		// Horizontal movement
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT) {
			velocity.x = -0.35f * 16;
			InitialState();
			look = View::LEFT;
		}
		else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT) {
			velocity.x = 0.35f * 16;
			InitialState();
			look = View::RIGHT;
		}
		else {
			velocity.x = 0.0f;  // Stop horizontal movement if no key is pressed
		}

		// Vertical movement
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT) {
			velocity.y = -0.35f * 16;  // Move up
		}
		else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT) {
			velocity.y = 0.35f * 16;  // Move down
		}
		else {
			velocity.y = 0.0f;  // Stop vertical movement if no key is pressed
		}

	}

	if (!isDead && !godMode && !menu) {
		pbody->body->SetType(b2_dynamicBody);
		// Handle horizontal movement
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && !(velocity.y > 1.0E-5)) {
			if (!isWalled_left) {
				velocity.x = -0.2f * 16; // Move left
				look = View::LEFT;
				StartRunning();
			}
			else {
				InitialState();
			}
		}
		else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && !(velocity.y > 1.0E-5)) {
			if (!isWalled_right)
			{
				velocity.x = 0.2f * 16; // Move right
				look = View::RIGHT;
				StartRunning();
			}
			else {
				InitialState();
			}

		}
		else if (velocity.y > 1.0E-5)
		{
			StartFalling();
		}
		else {
			InitialState(); // Go idle if no keys are pressed
		}


		// Jumping logic
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping && !isFalling) {
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isJumping = true;
			// play jump sound
			Engine::GetInstance().audio.get()->PlayFx(jumpFxId);
			StartJumping();			
		}

		// Check if the player is falling
		if (isJumping) {
			velocity.y = pbody->body->GetLinearVelocity().y;

			if (velocity.y < 0) {
				// Still jumping
				if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
					velocity.x = -0.2 * 16; // Move left
					look = View::LEFT;
					StartJumping();
				}
				else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
					velocity.x = 0.2f * 16; // Move right
					look = View::RIGHT;
					StartJumping();
				}
				else {
					StartJumping();
				}
			}

			// If vertical velocity is down, switch to falling state
			else if (velocity.y > 0) {
				// Falling
				if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
					velocity.x = -0.2 * 16; // Move left
					look = View::LEFT;
					StartFalling();
				}
				else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
					velocity.x = 0.2f * 16; // Move right
					look = View::RIGHT;
					StartFalling();
				}
				else {
					StartFalling();
				}
			}
		}
	}
	if (isDead && pbody->body != nullptr && !godMode) {
		StartDying();
		// Destroy the player's body in the physics world
		if(currentAnimation->HasFinished()) {
			if (Engine::GetInstance().scene.get()->checkpoint)
			{
				Engine::GetInstance().scene.get()->LoadState();
			}
			else {
				Player::position.setX(13);
				Player::position.setY(10);

				pbody->body->SetTransform(b2Vec2(position.getX(), position.getY()), 0);
				Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
				currentAnimation->Update();
			}
			
			die_right.Reset();
			die_left.Reset();

			isDead = false;
			return true;
		}
	}
	if (menu) {
		pbody->body->SetLinearVelocity(b2Vec2(0,0));
		return true;
	}
	// Set new velocity
	pbody->body->SetLinearVelocity(velocity);
	b2Transform pbodyPos = pbody->body->GetTransform();
	Pos = pbodyPos.p;

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texW / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
	// Render the current animation
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
}

void Player::SetPosition(Vector2D position)
{
	position.setX(position.getX());
	position.setY(position.getY());
	b2Vec2 pos = b2Vec2(PIXEL_TO_METERS(position.getX()), PIXEL_TO_METERS(position.getY()));
	pbody->body->SetTransform(pos, 0);
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		TestPlatform(physA, physB);
		break;
	case ColliderType::WALL:
		LOG("Collision WALL");
		TestWall(physB, physA);
		break;
	case ColliderType::SPIKE:
		LOG("Collision Spike");
		isDead = true;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		Engine::GetInstance().physics.get()->DeletePhysBody(physB); // Deletes the body of the item from the physics world
		break;
	case ColliderType::CHECKPOINT:
		LOG("Collision CHECKPOINT");
		Engine::GetInstance().scene.get()->checkpoint = true;
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

Vector2D Player::GetPosition() const
{
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		isGrounded_up = false;
		isGrounded_down = false;
		break;
	case ColliderType::WALL:
		LOG("End Collision WALL");
		isWalled_left = false;
		isWalled_right = false;
		break;
	case ColliderType::SPIKE:
		LOG("Collision Spike");
		isDead = false;
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::CHECKPOINT:
		LOG("End Collision CHECKPOINT");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::TestWall(PhysBody* physA, PhysBody* physB)
{
	b2Transform transform_A = physA->body->GetTransform();
	b2Vec2 position_A = transform_A.p;

	b2Transform transform_B = physB->body->GetTransform();
	b2Vec2 position_B = transform_B.p;

	if (position_A.x > position_B.x)
	{
		isWalled_right = true;
	}
	else {
		isWalled_left = true;
	}
}

void Player::TestPlatform(PhysBody* physA, PhysBody* physB)
{
	b2Transform transform_A = physA->body->GetTransform();
	b2Vec2 position_A = transform_A.p;

	b2Transform transform_B = physB->body->GetTransform();
	b2Vec2 position_B = transform_B.p;

	if (position_A.y < position_B.y)
	{
		isGrounded_up = true;
		isJumping = false;

		//play fall sound
		Engine::GetInstance().audio.get()->PlayFx(fallFxId);
	}
	else {
		isGrounded_down = true;
		isJumping = true;
	}
}