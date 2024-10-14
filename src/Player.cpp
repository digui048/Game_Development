#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	position = Vector2D(0, 0);
	return true;
}

bool Player::Initialise() {

	int i;
	const int n = PLAYER_FRAME_SIZE;

	ResourceManager& data = ResourceManager::Instance();
	if (data.LoadTexture(Resource::IMG_PLAYER, "images/Sloth.png") != AppStatus::OK)
	{
		return AppStatus::ERROR;
	}

	render = new Sprite(data.GetTexture(Resource::IMG_PLAYER));
	if (render == nullptr)
	{
		LOG("Failed to allocate memory for player sprite");
		return AppStatus::ERROR;
	}

	Sprite* sprite = dynamic_cast<Sprite*>(render);
	sprite->SetNumberAnimations((int)PlayerAnim::NUM_ANIMATIONS);

	sprite->SetAnimationDelay((int)PlayerAnim::IDLE_RIGHT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::IDLE_RIGHT, { 0, 0, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::IDLE_LEFT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::IDLE_LEFT, { 0, 0, -n, n });

	sprite->SetAnimationDelay((int)PlayerAnim::WALKING_RIGHT, ANIM_DELAY);
	for (i = 0; i < 2; ++i)
		sprite->AddKeyFrame((int)PlayerAnim::WALKING_RIGHT, { (float)i * n, 0, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::WALKING_LEFT, ANIM_DELAY);
	for (i = 0; i < 2; ++i)
		sprite->AddKeyFrame((int)PlayerAnim::WALKING_LEFT, { (float)i * n, 0, -n, n });

	sprite->SetAnimationDelay((int)PlayerAnim::FALLING_RIGHT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::FALLING_RIGHT, { 0, n, n, n });
	sprite->AddKeyFrame((int)PlayerAnim::FALLING_RIGHT, { 0, n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::FALLING_LEFT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::FALLING_LEFT, { 0, n, -n, n });
	sprite->AddKeyFrame((int)PlayerAnim::FALLING_LEFT, { 0, n, -n, n });

	sprite->SetAnimationDelay((int)PlayerAnim::JUMPING_RIGHT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::JUMPING_RIGHT, { 0, n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::JUMPING_LEFT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::JUMPING_LEFT, { 0, n, -n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::LEVITATING_RIGHT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::LEVITATING_RIGHT, { 0, n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::LEVITATING_LEFT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::LEVITATING_LEFT, { 0, n, -n, n });

	sprite->SetAnimationDelay((int)PlayerAnim::CLIMBING, ANIM_LADDER_DELAY);
	for (i = 0; i < 2; ++i)
		sprite->AddKeyFrame((int)PlayerAnim::CLIMBING, { (float)i * n, 3 * n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::CLIMBING_PRE_TOP, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::CLIMBING_PRE_TOP, { 0, 3 * n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::CLIMBING_TOP, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::CLIMBING_TOP, { n, 3 * n, n, n });

	sprite->SetAnimationDelay((int)PlayerAnim::PUNCHING_RIGHT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::PUNCHING_RIGHT, { 0, 2 * n, n, n });
	sprite->SetAnimationDelay((int)PlayerAnim::PUNCHING_LEFT, ANIM_DELAY);
	sprite->AddKeyFrame((int)PlayerAnim::PUNCHING_LEFT, { 0, 2 * n, -n, n });

	sprite->SetAnimation((int)PlayerAnim::IDLE_RIGHT);
}

bool Player::Start() {

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/player1.png");


	// L08 TODO 5: Add physics to the player - initialize physics body
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texW / 2, bodyType::DYNAMIC);

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	return true;
}

bool Player::Update(float dt)
{
	// L08 TODO 5: Add physics to the player - updated player position using physics
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * dt;
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * dt;
	}

	pbody->body->SetLinearVelocity(velocity);
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());
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