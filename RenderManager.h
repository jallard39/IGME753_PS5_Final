#pragma once

#include <sceconst.h>
#include <kernel.h>
#include <agc.h>
#include <video_out.h>
#include <math.h>
#include <stdio.h>
#include <vectormath.h>
#include <vector>

using namespace std;
using namespace sce::Vectormath::Scalar::Aos;

#define PI 3.14159

//	Define the width and height to render at the native resolution on
//	hardware.
#define DISPLAY_WIDTH				3840
#define DISPLAY_HEIGHT				2160
#define BUFFER_NUMBER				2

// Data structure for basic geometry
typedef struct BasicVertex {
	float pos[3];
	float color[3];

	BasicVertex() { pos[0] = 0.0f; pos[1] = 0.0f; pos[2] = 0.0f; color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; }
	BasicVertex(float x, float y, float z) { pos[0] = x; pos[1] = y; pos[2] = z; color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; }
	BasicVertex(float x, float y, float z, float r, float g, float b) {
		pos[0] = x; pos[1] = y; pos[2] = z; color[0] = r; color[1] = g; color[2] = b;
	}
	BasicVertex operator+(const BasicVertex& rhs) {
		return BasicVertex(pos[0] + rhs.pos[0], pos[1] + rhs.pos[1], pos[2] + rhs.pos[2]);
	}
	BasicVertex operator+(const float& rhs) {
		return BasicVertex(pos[1] + rhs, pos[1] + rhs, pos[2] += rhs);
	}
	BasicVertex operator*(const BasicVertex& rhs) {
		return BasicVertex(pos[0] * rhs.pos[0], pos[1] * rhs.pos[1], pos[2] *= rhs.pos[2]);
	}
	BasicVertex operator*(const float& rhs) {
		return BasicVertex(pos[0] * rhs, pos[1] * rhs, pos[2] * rhs);
	}

} BasicVertex;

struct Object
{
	BasicVertex* vertices;
	uint32_t vertexCount = 0;
	sce::Agc::Core::Buffer vertexBuffer;
	uint16_t* indexBuffer = nullptr;
	uint32_t indexCount = 0;
	Matrix4 transform;
	bool isActive = true;
};

// Base class for all games that need a rendering loop
class RenderManager
{
private:
	int windowX;
	int windowY;
	uint32_t backBufferIndex = 0;
	uint64_t frameNumber = 0;
	uint32_t numObjects = 1;
	uint32_t indicesPerObject;
	sce::Agc::Core::Buffer vertexBuffer;
	uint16_t* indexBuffer = nullptr;
	sce::Agc::Core::Buffer matBuffer;
	BasicVertex* triangle;
	vector<Object> allObjects;

	// sce variables
	SceError error;
	sce::Agc::Label* flipLabels;
	int videoHandle;
	SceVideoOutLatencyControl latencyControl{};
	sce::Agc::Core::BasicContext ctxs[BUFFER_NUMBER];
	sce::Agc::Core::RenderTargetSpec rtSpec;
	sce::Agc::CxRenderTarget rts[BUFFER_NUMBER];
	sce::Agc::Core::DepthRenderTargetSpec drtSpec;
	sce::Agc::CxDepthRenderTarget drt;
	sce::Agc::Core::Encoder::EncoderValue rtClearValue;
	sce::Agc::CxRenderTargetMask rtMask;
	sce::Agc::CxViewport vport;
	sce::Agc::CxPrimitiveSetup primSetup;
	sce::Agc::CxDepthStencilControl depthStencilControl;
	sce::Agc::Shader* gs, * ps;
	
	void updateObjectPosition(int i, float dx, float dy);

public:
	RenderManager();
	RenderManager(int windowSizeX, int windowSizeY);
	~RenderManager(void);
	void init();
	void drawScene(); // draws all drawable components
	void setWindowSize(int winX, int winY) { windowX = winX; windowY = winY; }
	void setClearColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a) { 
		rtClearValue = sce::Agc::Core::Encoder::raw(r, g, b, a); }
	Object createObject(BasicVertex* vertices, uint32_t numVerts, uint16_t* indices, uint32_t numIndices);	
	Object createRectangle(BasicVertex* vertices, uint32_t numVerts, uint16_t* indices, uint32_t numIndices);
	Matrix4* createViewMatrix();
	Matrix4 creatOriginViewMatrix();
	void updatePaddlePosition(float dx, float dy);
	void updateBallPosition(float dx, float dy);
	vector<Object>& GetAllObjects();
}; 
