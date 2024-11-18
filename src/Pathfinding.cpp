#include "Pathfinding.h"
#include "Engine.h"
#include "Textures.h"
#include "Map.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"

Pathfinding::Pathfinding()
{
	//Loads texture to draw the path
	pathTex = Engine::GetInstance().textures.get()->Load("assets/path.png");
	tileX = Engine::GetInstance().textures.get()->Load("assets/tileX.png");
	map = Engine::GetInstance().map.get();
	layerNav = map->GetNavigationLayer();

	// Initialize the costSoFar with all elements set to 0
	costSoFar = std::vector<std::vector<int>>(map->GetWidth(), std::vector<int>(map->GetHeight(), 0));
}

Pathfinding::~Pathfinding()
{
	delete layerNav;
}

void Pathfinding::ResetPath(Vector2D pos)
{
	//Clears the frontier and visited list
	while (!frontier.empty())
	{
		frontier.pop();
	}

	// Clears the frontier Djkstra and visited list
	while (!frontierDijkstra.empty())
	{
		frontierDijkstra.pop();
	}

	// Clears the frontier AStar and visited list
	while (!frontierAStar.empty())
	{
		frontierAStar.pop();
	}
	
	visited.clear();		// Clear the visited list
	breadcrumbs.clear();	// Clear the breadcrumbs list
	pathTiles.clear();		// Clear the pathTiles list

	// Inserts the first position in the queue and visited list
	frontier.push(pos);									// BFS	
	frontierDijkstra.push(std::make_pair(0, pos));		// Dijkstra
	frontierAStar.push(std::make_pair(0, pos));			// AStar
	visited.push_back(pos);
	breadcrumbs.push_back(pos);

	costSoFar = std::vector<std::vector<int>>(map->GetWidth(), std::vector<int>(map->GetHeight(), 0));
}

void Pathfinding::DrawPath()
{
	Vector2D point;
	
	// Draw the visited points
	for (const auto& pathTile : visited)
	{
		Vector2D pathTileWorld = Engine::GetInstance().map.get()->MapToWorld(pathTile.getX(), pathTile.getY());
		SDL_Rect rect = { 32,0,32,32 };
		Engine::GetInstance().render.get()->DrawTexture(pathTex, pathTileWorld.getX(), pathTileWorld.getY(), &rect);
	}

	// ---------------- Draw frontier BFS----------------
	// Create a copy of the frontier queue
	std::queue<Vector2D> frontierCopy = frontier;

	// Iterate the frontier queue
	while (!frontierCopy.empty())
	{
		Vector2D frontierTile = frontierCopy.front();
		Vector2D pointWorld = Engine::GetInstance().map.get()->MapToWorld(frontierTile.getX(), frontierTile.getY());
		SDL_Rect rect = { 0,0,32,32 };
		Engine::GetInstance().render.get()->DrawTexture(pathTex, pointWorld.getX(), pointWorld.getY(), &rect);
		frontierCopy.pop();
	}

	// ---------------- Draw frontier Dijkstra----------------
	// Create a copy of the frontier queue
	std::priority_queue<std::pair<int, Vector2D>, std::vector<std::pair<int, Vector2D>>, std::greater<std::pair<int, Vector2D>>> frontierCopyDijkstra = frontierDijkstra;

	// Iterate the frontier queue
	while (!frontierCopyDijkstra.empty())
	{
		Vector2D frontierTile = frontierCopyDijkstra.top().second;
		Vector2D pointWorld = Engine::GetInstance().map.get()->MapToWorld(frontierTile.getX(), frontierTile.getY());
		SDL_Rect rect = { 0,0,32,32 };
		Engine::GetInstance().render.get()->DrawTexture(pathTex, pointWorld.getX(), pointWorld.getY(), &rect);
		frontierCopyDijkstra.pop();
	}

	// ---------------- Draw frontier AStar----------------
	// Create a copy of the frontier queue
	std::priority_queue<std::pair<int, Vector2D>, std::vector<std::pair<int, Vector2D>>, std::greater<std::pair<int, Vector2D>>> frontierCopyAStar = frontierAStar;

	// Iterate the frontier queue
	while (!frontierCopyAStar.empty())
	{
		Vector2D frontierTile = frontierCopyAStar.top().second;
		Vector2D pointWorld = Engine::GetInstance().map.get()->MapToWorld(frontierTile.getX(), frontierTile.getY());
		SDL_Rect rect = { 0,0,32,32 };
		Engine::GetInstance().render.get()->DrawTexture(pathTex, pointWorld.getX(), pointWorld.getY(), &rect);
		frontierCopyAStar.pop();
	}

	// Draw the path
	for (const auto& pathTile : pathTiles)
	{
		Vector2D pathTileWorld = map->MapToWorld(pathTile.getX(), pathTile.getY());
		Engine::GetInstance().render.get()->DrawTexture(tileX, pathTileWorld.getX(), pathTileWorld.getY());
	}
}

bool Pathfinding::IsWalkable(int x, int y)
{
	bool isWalkable = false;

	if (layerNav != nullptr) {
		
		if (x >= 0 && x < map->GetWidth() && y >= 0 && y < map->GetHeight())
		{
			int gid = layerNav->Get(x, y);
			if (gid != blockedGid) isWalkable = true;
		}
	}

	return isWalkable;
}

void Pathfinding::PropagateBFS()
{
	// Check if we have reached the destination
	bool foundDestination = false;
	if (frontier.size() > 0)
	{
		Vector2D frontierTile = frontier.front();
		Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
		Vector2D playerPosTile = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

		if (frontierTile == playerPosTile)
		{
			foundDestination = true;

			// Compute the path
			ComputePath(frontierTile.getX(), frontierTile.getY());
		}
	}

	// if frontier queue contains elements pop the first element and find the neighbours
	if (!foundDestination && !frontier.empty())
	{
		Vector2D currentTile = frontier.front();
		frontier.pop();

		// Get the neighbours of the current tile
		std::vector<Vector2D> neighbours;
		if (IsWalkable(currentTile.getX() + 1, currentTile.getY())) {
			neighbours.push_back(Vector2D(currentTile.getX() + 1, currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX() - 1, currentTile.getY())) {
			neighbours.push_back(Vector2D(currentTile.getX() - 1, currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() + 1)) {
			neighbours.push_back(Vector2D(currentTile.getX(), currentTile.getY() + 1));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() - 1)) {
			neighbours.push_back(Vector2D(currentTile.getX(), currentTile.getY() - 1));
		}

		// Iterate the neighbours
		for (const auto& neighbour : neighbours)
		{
			// Check if the neighbour is walkable and has not been visited
			if (std::find(visited.begin(), visited.end(), neighbour) == visited.end())
			{
				// Add the neighbour to the frontier and visited list
				frontier.push(neighbour);
				visited.push_back(neighbour);
				breadcrumbs.push_back(currentTile);
			}
		}
	}
}

int Pathfinding::MovementCost(int x, int y)
{
	int ret = -1;

	if (x >= 0 && x < map->GetWidth() && y >= 0 && y < map->GetHeight())
	{
		int gid = layerNav->Get(x, y);
		if (gid == highCostGid) ret = 5;
		else ret = 1;
	}

	return ret;
}

void Pathfinding::ComputePath(int x, int y)
{
	// Clear the pathTiles list
	pathTiles.clear();
	// Save the position received and stored in the current tile
	Vector2D currentTile = Vector2D(x, y);
	// Add the current tile to the pathTiles list
	pathTiles.push_back(currentTile);
	// Find the position of the current tile in the visited list
	int index = Find(visited, currentTile);

	// While the current tile and breadcrumbs are different it means that we have not reached the starting point
	while ((index >= 0) && (currentTile != breadcrumbs[index]))
	{
		// Update the current tile with the breadcrumb
		currentTile = breadcrumbs[index];
		// Add the current tile to the pathTiles list
		pathTiles.push_back(currentTile);
		// Find the position of the current tile in the visited list
		index = Find(visited, currentTile);
	}
}

void Pathfinding::PropagateDijkstra()
{
	// Check if we have reached the destination
	bool foundDestination = false;
	if (frontierDijkstra.size() > 0)
	{
		Vector2D frontierTile = frontierDijkstra.top().second;
		Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
		Vector2D playerPosTile = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

		if (frontierTile == playerPosTile)
		{
			foundDestination = true;

			// Compute the path
			ComputePath(frontierTile.getX(), frontierTile.getY());
		}
	}

	// if frontier queue contains elements pop the first element and find the neighbours
	if (!foundDestination && !frontierDijkstra.empty())
	{
		Vector2D currentTile = frontierDijkstra.top().second;
		frontierDijkstra.pop();

		// Get the neighbours of the current tile
		std::vector<Vector2D> neighbours;
		if (IsWalkable(currentTile.getX() + 1, currentTile.getY())) {
			neighbours.push_back(Vector2D(currentTile.getX() + 1, currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX() - 1, currentTile.getY())) {
			neighbours.push_back(Vector2D(currentTile.getX() - 1, currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() + 1)) {
			neighbours.push_back(Vector2D(currentTile.getX(), currentTile.getY() + 1));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() - 1)) {
			neighbours.push_back(Vector2D(currentTile.getX(), currentTile.getY() - 1));
		}

		// Iterate the neighbours
		for (const auto& neighbour : neighbours)
		{
			int cost = costSoFar[(int)currentTile.getX()][(int)currentTile.getY()] + MovementCost((int)neighbour.getX(), (int)neighbour.getY());
			// Check if the neighbour is walkable and has not been visited
			if (std::find(visited.begin(), visited.end(), neighbour) == visited.end() || cost < costSoFar[neighbour.getX()][neighbour.getY()])
			{
				// Add the neighbour to the frontier and visited list
				costSoFar[neighbour.getX()][neighbour.getY()] = cost;
				frontierDijkstra.push(std::make_pair(cost, neighbour));
				visited.push_back(neighbour);
				breadcrumbs.push_back(currentTile);
			}
		}
	}
}

void Pathfinding::PropagateAStar(ASTAR_HEURISTICS heuristic)
{
	// Check if we have reached the destination
	bool foundDestination = false;
	if (frontierAStar.size() > 0)
	{
		Vector2D frontierTile = frontierAStar.top().second;
		Vector2D playerPos = Engine::GetInstance().scene.get()->GetPlayerPosition();
		Vector2D playerPosTile = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());

		if (frontierTile == playerPosTile)
		{
			foundDestination = true;

			// Compute the path
			ComputePath(frontierTile.getX(), frontierTile.getY());
		}
	}

	// if frontier queue contains elements pop the first element and find the neighbours
	if (!foundDestination && !frontierAStar.empty())
	{
		Vector2D currentTile = frontierAStar.top().second;
		frontierAStar.pop();

		// Get the neighbours of the current tile
		std::vector<Vector2D> neighbours;
		if (IsWalkable(currentTile.getX() + 1, currentTile.getY())) {
			neighbours.push_back(Vector2D((int)currentTile.getX() + 1, (int)currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX() - 1, currentTile.getY())) {
			neighbours.push_back(Vector2D((int)currentTile.getX() - 1, (int)currentTile.getY()));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() + 1)) {
			neighbours.push_back(Vector2D((int)currentTile.getX(), (int)currentTile.getY() + 1));
		}
		if (IsWalkable(currentTile.getX(), currentTile.getY() - 1)) {
			neighbours.push_back(Vector2D((int)currentTile.getX(), (int)currentTile.getY() - 1));
		}

		// Iterate the neighbours
		for (const auto& neighbour : neighbours)
		{
			int cost = costSoFar[(int)currentTile.getX()][(int)currentTile.getY()] + MovementCost((int)neighbour.getX(), (int)neighbour.getY());
			// estimated movement cost from the current tile to the destination tile
			int heuristicCost = 0;

			switch (heuristic)
			{
			case::MANHATTAN:
				heuristicCost = abs(neighbour.getX() - destination.getX()) + abs(neighbour.getY() - destination.getY());
				break;
			case::EUCLIDEAN:
				heuristicCost = sqrt(pow(neighbour.getX() - destination.getX(), 2) + pow(neighbour.getY() - destination.getY(), 2));
				break;
			case::SQUARED:
				heuristicCost = pow(neighbour.getX() - destination.getX(), 2) + pow(neighbour.getY() - destination.getY(), 2);
				break;
			}

			// A* Priority function
			int priority = cost + heuristicCost;

			if (std::find(visited.begin(), visited.end(), neighbour) == visited.end() || cost < costSoFar[neighbour.getX()][neighbour.getY()])
			{
				// Add the neighbour to the frontier and visited list
				costSoFar[neighbour.getX()][neighbour.getY()] = cost;
				frontierAStar.push(std::make_pair(priority, neighbour));
				visited.push_back(neighbour);
				breadcrumbs.push_back(currentTile);
			}
		}
	}
}

int Pathfinding::Find(std::vector<Vector2D> vector, Vector2D elem)
{
	int index = 0;
	bool found = false;
	
	for (const auto& e : vector) {
		if (e == elem) {
			found = true;
			break;
		}
		index++;
	}

	if (found) return index;
	else return -1;
}
