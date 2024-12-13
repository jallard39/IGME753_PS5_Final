

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

const float GRID_SIZE = 0.1f;
const int MAZE_WIDTH = 42;
const int MAZE_HEIGHT = 22;
int mazeTemplate[MAZE_HEIGHT][MAZE_WIDTH] =
{
	{1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1},

	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},

	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},

	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},

	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},
	{1,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  1},

	{1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1}
};
int num_walls = 0;

const float PLAYER_START[2] = { 0.0f, 0.0f };
const float PLAYER_SPEED = 0.015f;

float xPlayerPos = 0.0f;
float yPlayerPos = 0.0f;

int playerDirectionX = 1;
int playerDirectionY = 0;
int prevPlayerDirectionX = playerDirectionX;
int prevPlayerDirectionY = playerDirectionY;

vector<Object> walls;

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
		grid[row][col] == 0 || grid[row][col] == 2);
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

pair<int, int> worldToGrid(float realX, float realY, float startX, float startY, float gridSize) {
	// Calculate grid indices
	int row = static_cast<int>(std::floor((startY - realY) / gridSize));
	int col = static_cast<int>(std::floor((realX - startX) / gridSize));

	return { row, col }; 
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
	//// Left wall
	//matrices[m++] = origin * Matrix4::translation({ -2.2, 0, 0.0f }) * Matrix4::scale({ 0.2,5,1.0f });
	//// Right wall
	//matrices[m++] = origin * Matrix4::translation({ 2.2, 0, 0.0f }) * Matrix4::scale({ 0.2,5,1.0f });
	//// Top wall
	//matrices[m++] = origin * Matrix4::translation({ 0, 1.2, 0.0f }) * Matrix4::scale({ 8.8,0.2,1.0f });
	//// Bottom wall
	//matrices[m++] = origin * Matrix4::translation({ 0, -1.2, 0.0f }) * Matrix4::scale({ 8.8,0.2,1.0f });

	int m = I_MAZE;
	float startX = 0 - (MAZE_WIDTH / 2) * GRID_SIZE;
	float startY = (MAZE_HEIGHT / 2) * GRID_SIZE;

	for (int i = 0; i < MAZE_WIDTH; i++)
	{
		for (int j = 0; j < MAZE_HEIGHT; j++)
		{
			if (mazeTemplate[j][i] == 1) 
			{
				matrices[m++] = origin * Matrix4::translation({ startX + (i * GRID_SIZE), startY - (j * GRID_SIZE), 0.0f});
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
		for (int i = 0; i < MAZE_WIDTH; i++)
		{
			for (int j = 0; j < MAZE_HEIGHT; j++)
			{
				if (mazeTemplate[j][i] == 1)
				{
					num_walls++;
				}
			}
		}

		// Init geometry
		renderManager->createBasicGeometry(GRID_SIZE);
		renderManager->createRect(1, ObjectType::Player);
		renderManager->createRect(1, ObjectType::Ghost);
		renderManager->createRect(num_walls, ObjectType::Wall); // The first one is the player, the second one is used to test with texture

		Matrix4* matrices = renderManager->createViewMatrix();
		Matrix4 origin = renderManager->creatOriginViewMatrix();

		//                         row col
		pair<int, int> enemyPos = { 1, 1 }; // Enemy position
		pair<int, int> playerPos_Grid = { 1, 33 }; // Player position 

		// matrices[0] is the player position
		//matrices[0] = origin * Matrix4::translation({ PLAYER_START[0], PLAYER_START[1], 0.0f });
		
		// TODO: have a function that can converts position to the index position in our maze grid
		// This is test code for the AI
		float startX = 0 - (MAZE_WIDTH / 2) * GRID_SIZE;
		float startY = (MAZE_HEIGHT / 2) * GRID_SIZE;
		playerPos_Grid = worldToGrid(xPlayerPos, yPlayerPos, startX, startY, GRID_SIZE);
		matrices[0] = origin * Matrix4::translation({ startX + playerPos_Grid.second * GRID_SIZE, startY - playerPos_Grid.first * GRID_SIZE, 0.0f});

		matrices[1] = origin * Matrix4::translation({ startX + enemyPos.second * GRID_SIZE, startY - enemyPos.first * GRID_SIZE, 0.0f });
	
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

			xPlayerPos += PLAYER_SPEED * playerDirectionX;
			yPlayerPos += PLAYER_SPEED * playerDirectionY;
																						// Sample rotation code, we can do do this based on the move direc
			matrices[0] = origin * Matrix4::translation({ xPlayerPos, yPlayerPos, 0 }) * Matrix4::rotation(1.5, { 0, 0, 1 });
			playerPos_Grid = worldToGrid(xPlayerPos, yPlayerPos, startX, startY, GRID_SIZE);
			
			enemyPos = moveEnemy(mazeTemplate, enemyPos, playerPos_Grid, enemySpeed, elapsedTime);
			matrices[1] = origin * Matrix4::translation({ startX + enemyPos.second * GRID_SIZE, startY - enemyPos.first * GRID_SIZE, 0.0f });

			// =========================================================
			// Wall Collision Detection - AABB from bottom left corner
			// =========================================================

			//// Get player location next frame
			//float playerNextX = player.vertices[0].pos[0] + PLAYER_SPEED * playerDirectionX;
			//float playerNextY = player.vertices[0].pos[1] + PLAYER_SPEED * playerDirectionY;

			//for (int i = 0; i < walls.size(); i++) 
			//{
			//	// Get rectangle bounds
			//	float wallX = walls[i].vertices[0].pos[0];
			//	float wallY = walls[i].vertices[0].pos[1];
			//	float wallWidth = walls[i].vertices[1].pos[0] - wallX;
			//	float wallHeight = walls[i].vertices[3].pos[1] - wallY;

			//	// Check for collision
			//	if (playerNextX < wallX + wallWidth && playerNextX + PLAYER_SIZE > wallX &&
			//		playerNextY < wallY + wallHeight && playerNextY + PLAYER_SIZE > wallY) 
			//	{
			//		// Differentiate between horizontal and vertical directions
			//		if (playerDirectionY == 0) // moving horizontally
			//		{
			//			if (prevPlayerDirectionX == 0) // switching directions
			//			{
			//				playerDirectionX = prevPlayerDirectionX;
			//				playerDirectionY = prevPlayerDirectionY;
			//			}
			//			else
			//			{
			//				playerDirectionX = 0;
			//			}
			//		}
			//		else if (playerDirectionX == 0) // moving vertically
			//		{
			//			if (prevPlayerDirectionY == 0) // switching directions
			//			{
			//				playerDirectionX = prevPlayerDirectionX;
			//				playerDirectionY = prevPlayerDirectionY;
			//			}
			//			else
			//			{
			//				playerDirectionY = 0;
			//			}
			//		}

			//	}
			//}

			//renderManager->updateBallPosition(PLAYER_SPEED * playerDirectionX, PLAYER_SPEED * playerDirectionY);

			//prevPlayerDirectionX = playerDirectionX;
			//prevPlayerDirectionY = playerDirectionY;
			
			{
			// =====================================
			// OLD COLLISION DETECTION
			// =====================================
			//for (int i = 1; i < allObjects.size(); i++)
			//{
			//	if (allObjects[i].isActive)
			//	{
			//		// Track if a collision is detected
			//		bool collisionDetected = false;

			//		// Get rectangle bounds
			//		float xMin = allObjects[i].vertices[0].pos[0];
			//		float xMax = allObjects[i].vertices[2].pos[0];
			//		float yMin = allObjects[i].vertices[0].pos[1];
			//		float yMax = allObjects[i].vertices[2].pos[1];

			//		// Get center of circle
			//		float circleCenter_X = player.vertices[0].pos[0];
			//		float circleCenter_Y = player.vertices[0].pos[1];

			//		// Vertical collisions
			//		if (circleCenter_X >= xMin && circleCenter_X <= xMax)
			//		{
			//			if (circleCenter_Y >= yMax && circleCenter_Y - yMax < RADIUS ||
			//				circleCenter_Y <= yMin && yMin - circleCenter_Y < RADIUS) 
			//			{
			//				collisionDetected = true;
			//				directionY *= -1;
			//			}
			//		}
			//		// Horizontal collisions
			//		else if (circleCenter_Y >= yMin && circleCenter_Y <= yMax)
			//		{
			//			if (circleCenter_X >= xMax && circleCenter_X - xMax < RADIUS ||
			//				circleCenter_X <= xMin && xMin - circleCenter_X < RADIUS)
			//			{
			//				collisionDetected = true;
			//				directionX *= -1;
			//			}
			//		}
			//		// Corner collisions (both horizontal and vertical)
			//		else
			//		{
			//			// Get closest x and y
			//			float closestX;
			//			if (circleCenter_X < xMin) closestX = xMin;
			//			else if (circleCenter_X > xMax) closestX = xMax;
			//			else if (xMax - circleCenter_X + xMin - circleCenter_X > 0) closestX = xMin;
			//			else closestX = xMax;

			//			float closestY;
			//			if (circleCenter_Y < yMin) closestY = yMin;
			//			else if (circleCenter_Y > yMax) closestY = yMax;
			//			else if (yMax - circleCenter_Y + yMin - circleCenter_Y > 0) closestY = yMin;
			//			else closestY = yMax;

			//			// Check if radius overlaps with closest corner
			//			float dx = circleCenter_X - closestX;
			//			float dy = circleCenter_Y - closestY;

			//			float distanceSquared = dx * dx + dy * dy;

			//			if (distanceSquared <= RADIUS * RADIUS)
			//			{
			//				collisionDetected = true;

			//				if ((dx < 0) ? -dx : dx > (dy < 0) ? -dy : dy) {
			//					//// Horizontal collision
			//					//if (dx > 0) return "Left";
			//					//else return "Right";

			//					directionX *= -1;
			//				}
			//				else {
			//					//// Vertical collision
			//					//if (dy > 0) return "Top";
			//					//else return "Bottom";

			//					directionY *= -1;
			//				}
			//			}
			//		}

			//		// If collision detected, destroy brick
			//		if (collisionDetected && i != 1)
			//		{
			//			allObjects[i].isActive = false;
			//		}
			//	}
			//}
			}

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



