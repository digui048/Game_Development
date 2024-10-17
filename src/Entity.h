#pragma once

#include "Input.h"
#include "Render.h"

enum class EntityType
{
	PLAYER,
	ITEM,
	UNKNOWN
};

enum class View {
	
	LEFT,
	RIGHT
};

class PhysBody;

class Entity
{
public:

	Entity(EntityType type, View look) : type(type), active(true), look(look) {}

	virtual bool Awake()
	{
		return true;
	}

	virtual bool Start()
	{
		return true;
	}

	virtual bool Update(float dt)
	{
		return true;
	}

	virtual bool CleanUp()
	{
		return true;
	}

	void StartLookingLeft()
	{
		look = View::LEFT;
	}

	void StartLookingRight()
	{
		look = View::RIGHT;
	}

	void Enable()
	{
		if (!active)
		{
			active = true;
			Start();
		}
	}

	void Disable()
	{
		if (active)
		{
			active = false;
			CleanUp();
		}
	}

	virtual void OnCollision(PhysBody* physA, PhysBody* physB) {

	};

	virtual void OnCollisionEnd(PhysBody* physA, PhysBody* physB) {
	
	};

public:

	std::string name;
	EntityType type;
	bool active = true;
	View look;

	// Possible properties, it depends on how generic we
	// want our Entity class, maybe it's not renderable...
	Render* render;
	Vector2D position;       
	bool renderable = true;
};