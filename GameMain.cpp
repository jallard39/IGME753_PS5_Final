

#include <scebase_common.h>
#include <pad.h>
#include <cmath>
#include <queue>
#include <chrono>
#include "RenderManager.h"

const uint32_t SCREEN_WIDTH = 3840;
const uint32_t SCREEN_HEIGHT = 2160;

// indices 
int I_PLAYER = 0;
int I_Ghost = 1;
int I_MAZE = 2;

#pragma region MazeSetUp

const float GRID_SIZE = 0.1f;
const int MAZE_WIDTH = 42;
const int MAZE_HEIGHT = 22;

const float START_X = 0 - (MAZE_WIDTH / 2) * GRID_SIZE;
const float START_Y = (MAZE_HEIGHT / 2) * GRID_SIZE;

int mazeTemplate[MAZE_HEIGHT][MAZE_WIDTH] =
{//  0   1  2  3  4  5   6  7  8  9  10  11 12 13 14 15 16  17 18 19 20 21 22 23 24 25  26 27 28 29 30  31 32 33 34 35  36 37 38 39 40   41 
	{1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1},  // 0

	{1,  3, 0, 0, 0, 0,  3, 3, 3, 3, 3,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},  // 1
	{1,  0, 1, 1, 1, 1,  1, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 1, 1,  1, 1, 0, 0, 0,  0, 0, 0, 0, 1,  1, 1, 1, 1, 0,  1},  // 2
	{1,  3, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 1, 1, 1, 1,  1, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  1},  // 3
	{1,  0, 1, 3, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  3, 3, 3, 3, 3,  0, 0, 0, 1, 0,  1},  // 4
	{1,  3, 1, 0, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  1},  // 5

	{1,  3, 1, 3, 0, 1,  1, 1, 1, 1, 1,  0, 0, 0, 3, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 1, 0, 0,  3, 0, 0, 1, 0,  1},  // 6
	{1,  0, 0, 0, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 3, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 1, 0, 0,  3, 0, 0, 0, 0,  1},  // 7
	{1,  3, 0, 0, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 3, 0,  0, 0, 3, 3, 0,  0, 3, 3, 0, 0,  0, 0, 0, 0, 0,  0, 0, 1, 0, 0,  3, 0, 0, 0, 0,  1},  // 8
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 3, 0,  0, 0, 1, 1, 0,  0, 1, 1, 0, 0,  0, 0, 0, 0, 0,  0, 0, 1, 0, 0,  3, 0, 0, 0, 0,  1},  // 9 
	{1,  3, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 3, 0,  0, 0, 1, 0, 0,  0, 0, 1, 0, 0,  0, 0, 0, 0, 0,  0, 0, 1, 0, 0,  3, 0, 0, 0, 0,  1},  // 10

	{1,  3, 0, 0, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  0, 0, 1, 0, 0,  0, 0, 1, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 3,  1},  // 11
	{1,  0, 0, 3, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  0, 0, 1, 1, 1,  1, 1, 1, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 3,  1},  // 12 
	{1,  3, 0, 0, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 3,  1},  // 13
	{1,  0, 0, 3, 0, 1,  0, 0, 0, 0, 0,  0, 0, 0, 1, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 3,  1},  // 14 
	{1,  3, 1, 0, 0, 1,  1, 1, 1, 1, 1,  0, 0, 0, 1, 1,  1, 1, 0, 0, 0,  0, 0, 0, 1, 1,  1, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 1, 3,  1},  // 15

	{1,  3, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1,  0, 0, 1, 0, 0,  0, 0, 0, 1, 0,  1},  // 16
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1,  0, 0, 1, 0, 0,  0, 0, 0, 1, 0,  1},  // 17
	{1,  3, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 1, 1, 1, 1,  1, 1, 1, 0, 0,  0, 0, 0, 0, 1,  0, 0, 1, 0, 0,  0, 0, 0, 1, 0,  1},  // 18 
	{1,  0, 1, 1, 1, 1,  1, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 1,  0, 0, 1, 0, 1,  1, 1, 1, 1, 0,  1},  // 19
	{1,  3, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},  // 20

	{1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1}	  // 21
};

int num_enemies = 1;
int num_walls = 0;
int num_collectibles = 0;


// Change the world pos to the grid indices
pair<int, int> worldToGrid(float realX, float realY) {
	// Calculate grid indices
	int row = static_cast<int>(std::floor((START_Y - realY) / GRID_SIZE));
	int col = static_cast<int>(std::floor((realX - START_X) / GRID_SIZE));

	return { row, col };
}

// Change the grid to world translation matrix
Matrix4 gridToMatrix(int row, int col)
{
	// Calculate the world coordinates corresponding to the grid position
	float realX = START_X + (col * GRID_SIZE);
	float realY = START_Y - (row * GRID_SIZE);

	return Matrix4::translation({ realX, realY, 0.0f });
}

#pragma endregion

//                                row col
pair<int, int> enemyPos_Grid =  { 11, 20 }; // Enemy Start position
pair<int, int> playerPos_Grid = { 14, 21 }; // Player Start position 

const float PLAYER_SIZE = 0.09f;
const float PLAYER_SPEED = 0.015f;

float xPlayerPos = 0.0f;
float yPlayerPos = 0.0f;

int playerDirectionX = 1;
int playerDirectionY = 0;
int prevPlayerDirectionX = 0;
int prevPlayerDirectionY = 0;

SceUserServiceUserId userId; // user information
int32_t controllerHandle; 


// initializes and controller and also default user info
int initialize() {
	int ret = SCE_OK;
	ret = scePadInit();
	if (ret < 0) {
		return ret;
	}
	ret = sceUserServiceInitialize(NULL);
	if (ret < 0) {
		return ret;
	}
	ret = sceUserServiceGetInitialUser(&userId);
	if (ret < 0) {
		// Failed to obtain user ID value
		return ret;
	}

	// Example that specifies SCE_PAD_PORT_TYPE_STANDARD to type
	// Specify 0 for index
	// pParam is a reserved area (specify NULL)
controllerHandle = scePadOpen(userId, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
if (controllerHandle < 0) {
	return controllerHandle;
}

return SCE_OK;
}

// closes down controller handle
int finalize() {
	int ret = SCE_OK;

	ret = scePadClose(controllerHandle);

	return ret;
}

#pragma region AI Section
// Directions for movement: up, down, left, right
const int DIRECTION_Y[] = { -1, 1, 0, 0 };
const int DIRECTION_X[] = { 0, 0, -1, 1 };

bool isValidMove(int row, int col, int grid[MAZE_HEIGHT][MAZE_WIDTH])
{
	return (row >= 0 && row < MAZE_HEIGHT &&
		col >= 0 && col < MAZE_WIDTH &&
		grid[row][col] == 0 || grid[row][col] == 3);
	//  This is empty space OR  Collectible
}

// BFS function to find the shortest path
vector<pair<int, int>> findPath(int grid[MAZE_HEIGHT][MAZE_WIDTH], pair<int, int> start, pair<int, int> target) {
	queue<pair<int, int>> q;
	bool visited[MAZE_HEIGHT][MAZE_WIDTH] = {};
	pair<int, int> parent[MAZE_HEIGHT][MAZE_WIDTH];

	for (int i = 0; i < MAZE_HEIGHT; i++) {
		for (int j = 0; j < MAZE_WIDTH; j++) {
			parent[i][j] = { -1, -1 }; // Initialize parent array
		}
	}

	q.push(start);
	visited[start.first][start.second] = true;

	while (!q.empty()) {
		auto current = q.front();
		q.pop();

		if (current == target) break;

		for (int i = 0; i < 4; i++) {
			int row = current.first + DIRECTION_Y[i];
			int col = current.second + DIRECTION_X[i];

			if (isValidMove(row, col, grid) && !visited[row][col]) {
				visited[row][col] = true;
				q.push({ row, col });
				parent[row][col] = current;
			}
		}
	}

	// Reconstruct the path from target to start
	vector<pair<int, int>> path;
	for (pair<int, int> at = target; at != make_pair(-1, -1); at = parent[at.first][at.second]) {
		path.push_back(at);
	}

	if (path.empty() || path.back() != start) return {};

	reverse(path.begin(), path.end());
	return path;
}

// Move the enemy based on the next position in the path
pair<int, int> moveEnemy(
	int grid[MAZE_HEIGHT][MAZE_WIDTH],
	pair<int, int>& enemyPos,
	pair<int, int> playerPos,
	float timeNeededToMoveOneGrid,
	float& elapsedTime
) {
	vector<pair<int, int>> path = findPath(grid, enemyPos, playerPos);

	// If enough time has passed, move the enemy
	if (path.size() > 1 && elapsedTime >= timeNeededToMoveOneGrid) {
		enemyPos = path[1]; // Move to the next position
		elapsedTime -= timeNeededToMoveOneGrid; // Reset timer for the next move
	}

	return enemyPos;
}
#pragma endregion


// accesses the controller and gets info
bool handleUserEvents(RenderManager* renderManager) {

	ScePadData data;
	int isOk;
	static bool triangleDown = false;
	static bool squareDown = false;
	static bool circleDown = false;
	static bool crossDown = false;

	static bool upPressed = false;
	static bool downPressed = false;
	static bool leftPressed = false;
	static bool rightPressed = false;

	// Obtain controller data state
	isOk = scePadReadState(controllerHandle, &data);

	if (isOk == SCE_OK && data.connected) {
		// Data was successfully obtained

		//   Triangle press and release test
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE) == 0 && !triangleDown) {
			// most common case - not pushing anything or waiting to release the button
		}
		else
			if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE) != 0) {
				// button is now down
				triangleDown = true;
			}
			else
				if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_TRIANGLE) == 0 && triangleDown) {
					printf("## Triangle!!!! ##\n");
					triangleDown = false;
				}

		//   Circle
		//   Triangle press test
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE) == 0 && !circleDown) {
			// most common case - not pushing anything or waiting to release the button
		}
		else
			if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE) != 0) {
				// button is now down
				circleDown = true;
			}
			else
				if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CIRCLE) == 0 && circleDown) {
					printf("## Circle!!!! ##\n");
					circleDown = false;
					return true;
				}

		//   Square
		//   Triangle press test
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE) == 0 && !squareDown) {
			// most common case - not pushing anything or waiting to release the button
		}
		else
			if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE) != 0) {
				// button is now down
				squareDown = true;
			}
			else
				if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_SQUARE) == 0 && squareDown) {
					printf("## Square!!!! ##\n");
					squareDown = false;
				}

		//   Cross
		//   Triangle press test
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS) == 0 && !crossDown) {
			// most common case - not pushing anything or waiting to release the button
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS) != 0) {
			// button is now down
			crossDown = true;
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_CROSS) == 0 && crossDown) {
			printf("## Cross!!!! ##\n");
			crossDown = false;
		}

		// DPAD
		// Up
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_UP) == 0 && !upPressed)
		{
			
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_UP) != 0)
		{
			playerDirectionX = 0;
			playerDirectionY = 1;
			upPressed = true;
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_UP) == 0 && upPressed)
		{
			printf("## Up!!! ##\n");
			upPressed = false;
		}
	
		// Down
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN) == 0 && !downPressed)
		{
			
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN) != 0) 
		{
			playerDirectionX = 0;
			playerDirectionY = -1;
			downPressed = true;
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN) == 0 && downPressed)
		{
			printf("## Down!!! ##\n");
			downPressed = false;
		}

		// Left
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT) == 0 && !leftPressed)
		{
			
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT) != 0) 
		{
			playerDirectionX = -1;
			playerDirectionY = 0;
			leftPressed = true;
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT) == 0 && leftPressed)
		{
			printf("## Left!!! ##\n");
			leftPressed = false;
		}

		// Right
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT) == 0 && !rightPressed)
		{
			
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT) != 0) 
		{
			playerDirectionX = 1;
			playerDirectionY = 0;
			rightPressed = true;
		}
		else if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT) == 0 && rightPressed)
		{
			printf("## Right!!! ##\n");
			rightPressed = false;
		}

	}
	
	return false;
}


void createMaze(Matrix4* matrices, Matrix4 origin)
{
	int maze = num_enemies;
	int collective = maze + num_walls;

	for (int i = 0; i < MAZE_HEIGHT; i++)
	{
		for (int j = 0; j < MAZE_WIDTH; j++)
		{
			if (mazeTemplate[i][j] == 1)
			{
				matrices[maze++] = origin * gridToMatrix(i,j);
			}
			else if (mazeTemplate[i][j] == 2)
			{
				matrices[collective++] = origin * gridToMatrix(i, j);
			}
		}
	}
}

int main(int argc, const char *argv[])
{
	// set up controller class
	if (initialize() != SCE_OK) {
		printf("## error in initialization! ##");
	}
	else {

		RenderManager* renderManager = new RenderManager(SCREEN_WIDTH,SCREEN_HEIGHT);
		renderManager->setClearColor(0, 0, 0, 255);
		renderManager->init();

		// Initialize the num_walls
		for (int i = 0; i < MAZE_HEIGHT; i++)
		{
			for (int j = 0; j < MAZE_WIDTH; j++)
			{
				if (mazeTemplate[i][j] == 1)
				{
					num_walls++;
				}
				else if (mazeTemplate[i][j] == 3)
				{
					num_collectibles++;
				}
			}
		}

		// Init geometry
		renderManager->createBasicGeometry(GRID_SIZE);
		renderManager->createRect(1, ObjectType::Player);
		renderManager->createRect(1, ObjectType::Ghost);
		renderManager->createRect(num_walls, ObjectType::Wall); 
		renderManager->createRect(num_collectibles, ObjectType::Collectible);

		xPlayerPos = START_X + (playerPos_Grid.second * GRID_SIZE);
		yPlayerPos = START_Y - (playerPos_Grid.first * GRID_SIZE);

		Matrix4* matrices = renderManager->createViewMatrix();
		Matrix4 origin = renderManager->creatOriginViewMatrix();

		// matrices[0] is the player position
		matrices[0] = origin * gridToMatrix(playerPos_Grid.first, playerPos_Grid.second) * Matrix4::scale({ PLAYER_SIZE / GRID_SIZE, PLAYER_SIZE / GRID_SIZE, 1.0 });

		// matrices[1] is the enemy position
		matrices[1] = origin * gridToMatrix(enemyPos_Grid.first, enemyPos_Grid.second);
	
		float enemySpeed = 0.15f; // Time in seconds to move one step
		float elapsedTime = 0.0f;
		auto lastTime = chrono::steady_clock::now();

		// Create maze: set the wall postion
		createMaze(matrices, origin);

		printf("## Initialization has gone all right ##\n");

		bool done = false;

		//vector<Object>& allObjects = renderManager->GetAllObjects();
		//Object player = allObjects[0];
		
		
		// loop until exit
		while (!done) {
			auto currentTime = chrono::steady_clock::now();
			auto duration = currentTime - lastTime;
			auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();  // in microseconds

			// Convert deltaTime to float seconds
			float deltaTimeInSeconds = static_cast<float>(deltaTime) / 1000000.0f;

			// Update elapsed time
			lastTime = currentTime;
			elapsedTime += deltaTimeInSeconds;

			// =========================================================
			// Wall Collision Detection
			// =========================================================

			// Need to do this twice: once to correct user input, once to check actual movement
			for (int i = 0; i < 2; i++)
			{
				// Check player position next frame
				float playerNextX = xPlayerPos + PLAYER_SPEED * playerDirectionX;
				float playerNextY = yPlayerPos + PLAYER_SPEED * playerDirectionY;
				pair<int, int> gridOverlap1;
				pair<int, int> gridOverlap2;

				float offset = (GRID_SIZE - PLAYER_SIZE) / 2;

				if (playerDirectionX == 1)
				{
					gridOverlap1 = worldToGrid(playerNextX + (PLAYER_SIZE + offset), playerNextY - (PLAYER_SIZE + offset));
					gridOverlap2 = worldToGrid(playerNextX + (PLAYER_SIZE + offset), playerNextY - offset);
				}
				if (playerDirectionX == -1)
				{
					gridOverlap1 = worldToGrid(playerNextX + offset, playerNextY - offset);
					gridOverlap2 = worldToGrid(playerNextX + offset, playerNextY - (PLAYER_SIZE + offset));
				}
				if (playerDirectionY == 1)
				{
					gridOverlap1 = worldToGrid(playerNextX + offset, playerNextY - offset);
					gridOverlap2 = worldToGrid(playerNextX + (PLAYER_SIZE + offset), playerNextY - offset);
				}
				if (playerDirectionY == -1)
				{
					gridOverlap1 = worldToGrid(playerNextX + offset, playerNextY - (PLAYER_SIZE + offset));
					gridOverlap2 = worldToGrid(playerNextX + (PLAYER_SIZE + offset), playerNextY - (PLAYER_SIZE + offset));
				}

				// Check if moving into a wall
				if (mazeTemplate[gridOverlap1.first][gridOverlap1.second] == 1 ||
					mazeTemplate[gridOverlap2.first][gridOverlap2.second] == 1)
				{
					// Differentiate between horizontal and vertical directions
					if (playerDirectionY == 0) // moving horizontally
					{
						if (prevPlayerDirectionX == 0) // switching directions
						{
							playerDirectionX = prevPlayerDirectionX;
							playerDirectionY = prevPlayerDirectionY;
						}
						else
						{
							// Snap player to grid
							if (playerDirectionX == 1)
							{
								xPlayerPos = START_X + ((gridOverlap1.second - 1) * GRID_SIZE);
							}
							else if (playerDirectionX == -1)
							{
								xPlayerPos = START_X + ((gridOverlap1.second + 1) * GRID_SIZE);
							}

							playerDirectionX = 0;
						}
					}
					else if (playerDirectionX == 0) // moving vertically
					{
						if (prevPlayerDirectionY == 0) // switching directions
						{
							playerDirectionX = prevPlayerDirectionX;
							playerDirectionY = prevPlayerDirectionY;
						}
						else
						{
							// Snap player to grid
							if (playerDirectionY == 1)
							{
								yPlayerPos = START_Y - ((gridOverlap1.first + 1) * GRID_SIZE);
							}
							else if (playerDirectionY == -1)
							{
								yPlayerPos = START_Y - ((gridOverlap1.first - 1) * GRID_SIZE);
							}

							playerDirectionY = 0;
						}
					}
				}

				if (playerDirectionX == 0 && playerDirectionY == 0) break;
			}

			xPlayerPos += PLAYER_SPEED * playerDirectionX;
			yPlayerPos += PLAYER_SPEED * playerDirectionY;

			// Sample rotation code, we can do do this based on the move direc
			matrices[0] = origin * Matrix4::translation({ xPlayerPos, yPlayerPos, 0 }) * Matrix4::scale({ PLAYER_SIZE / GRID_SIZE, PLAYER_SIZE / GRID_SIZE, 1.0 }); // * Matrix4::rotation(1.5, { 0, 0, 1 });
			playerPos_Grid = worldToGrid(xPlayerPos, yPlayerPos);

			enemyPos_Grid = moveEnemy(mazeTemplate, enemyPos_Grid, playerPos_Grid, enemySpeed, elapsedTime);
			matrices[1] = origin * gridToMatrix(enemyPos_Grid.first, enemyPos_Grid.second);

			//renderManager->updateBallPosition(PLAYER_SPEED * playerDirectionX, PLAYER_SPEED * playerDirectionY);

			prevPlayerDirectionX = playerDirectionX;
			prevPlayerDirectionY = playerDirectionY;

			renderManager->drawScene();
			// Check to see if user has requested to quit the loop and update simple app stuff
			done = handleUserEvents(renderManager);
		}

		if (finalize() == SCE_OK) {
			printf(" ## controller closed down correctly! ##\n");
		}
		else {
			printf(" ## controller closed down incorrectly! ##\n");
		}
	}

}



