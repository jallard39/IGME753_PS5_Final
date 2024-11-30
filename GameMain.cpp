

#include <scebase_common.h>
#include <pad.h>
#include <cmath>
#include "RenderManager.h"

const uint32_t SCREEN_WIDTH = 3840;
const uint32_t SCREEN_HEIGHT = 2160;

const float PADDLE_SPEED = 0.05f;
const float BALL_SPEED = 0.015f;
float curentPos = 0.0f;

const float PLAYER_START[2] = { 0.0f, 0.0f };
const float PLAYER_SIZE = 0.1f;
int playerDirectionX = 1;
int playerDirectionY = 0;

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
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_UP) != 0 && !upPressed)
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
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_DOWN) != 0 && !downPressed)
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
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_LEFT) != 0 && !leftPressed)
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
		if ((data.buttons & ScePadButtonDataOffset::SCE_PAD_BUTTON_RIGHT) != 0 && !rightPressed)
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


const float WHITE[3] = { 1.0f, 1.0f, 1.0f };
const float RED[3] = { 1.0f, 0.0f, 0.0f };
const float GREEN[3] = { 0.0f, 1.0f, 0.0f };
const float BLUE[3] = { 0.0f, 0.0f, 1.0f };
const float YELLOW[3] = { 1.0f, 1.0f, 0.0f };
const float* COLORS[] = { RED, GREEN, BLUE, YELLOW, WHITE };

uint16_t rectangleIndices[6] = { 0, 1, 2, 0, 2, 3 };
const float RADIUS = 0.05f;
//const uint16_t SEGMENTS = 12;

void createMaze(RenderManager* renderManager)
{
	// Create top wall
	BasicVertex wallVerts[4] =
	{
		{ -2.0f, 0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2]},
		{ 2.0f, 0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] },
		{ 2.0f, 1.0f, -0.25f,  BLUE[0], BLUE[1], BLUE[2] },
		{ -2.0f, 1.0f, -0.25f, BLUE[0], BLUE[1], BLUE[2] }
	};
	renderManager->createObject(wallVerts, 4, rectangleIndices, 6);

	// Create bottom wall
	wallVerts[0] = { -2.0f, -1.0f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[1] = { 2.0f, -1.0f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[2] = { 2.0f, -0.9f, -0.25f,  BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[3] = { -2.0f, -0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	renderManager->createObject(wallVerts, 4, rectangleIndices, 6);

	// Create left wall
	wallVerts[0] = { -2.0f, -0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[1] = { -1.9f, -0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[2] = { -1.9f, 0.9f, -0.25f,  BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[3] = { -2.0f, 0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	renderManager->createObject(wallVerts, 4, rectangleIndices, 6);

	// Create right wall
	wallVerts[0] = { 1.9f, -0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[1] = { 2.0f, -0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[2] = { 2.0f, 0.9f, -0.25f,  BLUE[0], BLUE[1], BLUE[2] };
	wallVerts[3] = { 1.9f, 0.9f, -0.25f, BLUE[0], BLUE[1], BLUE[2] };
	renderManager->createObject(wallVerts, 4, rectangleIndices, 6);

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

		// Create player --> allObjects[0]
		
		BasicVertex ballVerts[4] =
		{
			{ PLAYER_START[0] - PLAYER_SIZE / 2, PLAYER_START[1] - PLAYER_SIZE / 2, -0.25f, WHITE[0], WHITE[1], WHITE[2] },
			{ PLAYER_START[0] + PLAYER_SIZE / 2, PLAYER_START[1] - PLAYER_SIZE / 2, -0.25f, WHITE[0], WHITE[1], WHITE[2] },
			{ PLAYER_START[0] + PLAYER_SIZE / 2, PLAYER_START[1] + PLAYER_SIZE / 2, -0.25f, WHITE[0], WHITE[1], WHITE[2] },
			{ PLAYER_START[0] - PLAYER_SIZE / 2, PLAYER_START[1] + PLAYER_SIZE / 2, -0.25f, WHITE[0], WHITE[1], WHITE[2] }
		};
		renderManager->createObject(ballVerts, 4, rectangleIndices, 6);

		// ===== Circle Code ======
		//BasicVertex ballVerts[SEGMENTS + 1];
		//ballVerts[0] = { 0.0f, 0.0f, -0.25f, WHITE[0], WHITE[1], WHITE[2] }; // Center
		//for (uint16_t i = 1; i <= SEGMENTS; ++i) {
		//	float angle = 2.0f * M_PI * i / SEGMENTS; // Angle in radians
		//	float x = RADIUS * cos(angle); // Radius of 0.05f for the circle
		//	float y = RADIUS * sin(angle);
		//	ballVerts[i] = { x, y, -0.25f, WHITE[0], WHITE[1], WHITE[2] };
		//}
		//uint16_t circleIndices[SEGMENTS * 3]; // Each segment contributes 3 indices to form a triangle
		//
		//for (int i = 0; i < SEGMENTS; ++i) {
		//	circleIndices[i * 3] = 0;            // Center vertex index (always 0)
		//	circleIndices[i * 3 + 1] = i + 1;    // Current vertex index (1 to SEGMENTS)
		//	circleIndices[i * 3 + 2] = (i + 1) % SEGMENTS + 1; // Next vertex index, wraps around
		//}

		// Create maze
		createMaze(renderManager);

		renderManager->createTestViewMatrix();

		printf("## Initialization has gone all right ##\n");

		bool done = false;

		vector<Object>& allObjects = renderManager->GetAllObjects();
		Object player = allObjects[0];
		
		
		// loop until exit
		while (!done) {

			renderManager->updateBallPosition(BALL_SPEED * playerDirectionX, BALL_SPEED * playerDirectionY);

			if (player.vertices[0].pos[0] - RADIUS < -2.15f) { playerDirectionX = 0; }
			if (player.vertices[0].pos[0] + RADIUS > 2.15f) { playerDirectionX = 0; }
			if (player.vertices[0].pos[1] - RADIUS < -1.25f) { playerDirectionY = 0; }
			if (player.vertices[0].pos[1] + RADIUS > 1.25f) { playerDirectionY = 0; }

			// ===================================================
			// Collision Detection - AABB from bottom left corner
			// ===================================================

			// Get player bounds
			float playerX = player.vertices[0].pos[0];
			float playerY = player.vertices[0].pos[1];

			for (int i = 1; i < allObjects.size(); i++) 
			{
				// Get rectangle bounds
				float wallX = allObjects[i].vertices[0].pos[0];
				float wallY = allObjects[i].vertices[0].pos[1];
				float wallWidth = allObjects[i].vertices[1].pos[0] - wallX;
				float wallHeight = allObjects[i].vertices[3].pos[1] - wallY;

				// Check for collision
				if (playerX < wallX + wallWidth && playerX + PLAYER_SIZE > wallX &&
					playerY < wallY + wallHeight && playerY + PLAYER_SIZE > wallY) 
				{
					// Differentiate between horizontal and vertical directions
				}
			}
			
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

			//		// ===== AABB Collision Code (2 rectangles) =====
			//		// 
			//		//if (ball.vertices[3].pos[0] < allObjects[i].vertices[3].pos[0] + BRICK_LENGTH &&
			//		//	ball.vertices[3].pos[0] + 0.1f > allObjects[i].vertices[3].pos[0] &&                 // TODO: Change the length of the ball
			//		//	ball.vertices[3].pos[1] < allObjects[i].vertices[3].pos[1] + BRICK_HEIGHT &&
			//		//	ball.vertices[3].pos[1] + 0.1f > allObjects[i].vertices[3].pos[1])
			//		//{

			//		//	float leftPenetration = allObjects[i].vertices[1].pos[0] - ball.vertices[0].pos[0];
			//		//	float rightPenetration = ball.vertices[1].pos[0] - allObjects[i].vertices[0].pos[0];
			//		//	float topPenetration = allObjects[i].vertices[2].pos[1] - ball.vertices[0].pos[1];
			//		//	float bottomPenetration = ball.vertices[2].pos[1] - allObjects[i].vertices[0].pos[1];

			//		//	if (i != 1)
			//		//	{
			//		//		allObjects[i].isActive = false;
			//		//	}

			//		//	if (leftPenetration < topPenetration && leftPenetration < bottomPenetration ||
			//		//		rightPenetration < topPenetration && rightPenetration < bottomPenetration)
			//		//	{
			//		//		directionX *= -1;
			//		//		printf("left or right collision\n");
			//		//	}
			//		//	else if (topPenetration < leftPenetration && topPenetration < rightPenetration ||
			//		//		bottomPenetration < leftPenetration && bottomPenetration < rightPenetration)
			//		//	{
			//		//		directionY *= -1;
			//		//		printf("top or bottom collision\n");
			//		//	}
			//		//	else
			//		//	{
			//		//		printf("Both???\n");
			//		//		directionX *= -1;
			//		//		directionY *= -1;
			//		//	}

			//		//}
			//	}
			//}

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

