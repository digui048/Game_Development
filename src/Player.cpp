#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Player::Player(State state) : Entity(EntityType::PLAYER, View::RIGHT)
{
	name = "Player";
	this->state = state;
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(32, 256);
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


	// L08 TODO 5: Add physics to the player - initialize physics body
	/*Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);*/
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio.get()->LoadFx("Assets/Audio/Fx/retro-video-game-coin-pickup-38299.ogg");

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

void Player::InitialState()
{
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

	// Handle horizontal movement
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && !(velocity.y > 0)) {
		velocity.x = -0.2 *dt; // Move left
		look = View::LEFT;
		StartRunning();
	}
	else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && !(velocity.y > 0)) {
		velocity.x = 0.2f *dt; // Move right
		look = View::RIGHT;
		StartRunning();
	}
	else if (velocity.y > 0)
	{
		StartFalling();
	}
	else {
		InitialState(); // Go idle if no keys are pressed
	}

	// Jumping logic
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && !isJumping && velocity.y == 0) {
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
		isJumping = true;
		StartJumping();
	}

	// Check if the player is falling
	if (isJumping) {
		velocity.y = pbody->body->GetLinearVelocity().y;
		
		if (velocity.y < 0) {
			// Still jumping
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
				velocity.x = -0.2 * dt; // Move left
				look = View::LEFT;
				StartJumping();
			}
			else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				velocity.x = 0.2f * dt; // Move right
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
				velocity.x = -0.2 * dt; // Move left
				look = View::LEFT;
				StartFalling();
			}
			else if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
				velocity.x = 0.2f * dt; // Move right
				look = View::RIGHT;
				StartFalling();
			}
			else {
				StartFalling();
			}
		}
	}

	// Set new velocity
	pbody->body->SetLinearVelocity(velocity);
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	// Render the current animation
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY(), &currentAnimation->GetCurrentFrame());
	currentAnimation->Update();

	return true;
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
		//reset the jump flag when touching the ground
		isJumping = false;
		break;
	case ColliderType::WALL:
		LOG("Collision WALL");
		//reset the jump flag when touching the ground
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}
void Player::CheckIdle()
{
	
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::WALL:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		Engine::GetInstance().audio.get()->PlayFx(pickCoinFxId);
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}