#pragma once

#include <sceconst.h>
#include <kernel.h>
#include <agc.h>
#include <video_out.h>
#include <math.h>
#include <stdio.h>
#include <vectormath.h>
#include <vector>
#include <agc_texture_tool/gnf.h>

using namespace std;
using namespace sce::Vectormath::Scalar::Aos;

typedef sce::Gnf::Header              GnfHeader;
typedef sce::Gnf::Contents            GnfContents;
typedef sce::TextureTool::Agc::TSharp TSharp;
// This is a magic number set at the start of every GNF file. It can be used to ensure
// that the file we're loading is a valid GNF file.
#define GNF_MAGIC 0x20464E47 
#define PATH_PREFIX "/app0/data/"

#define PI 3.14159

//	Define the width and height to render at the native resolution on
//	hardware.
#define DISPLAY_WIDTH				3840
#define DISPLAY_HEIGHT				2160
#define BUFFER_NUMBER				2

//// Data structure for basic geometry
//typedef struct BasicVertex {
//	float pos[3];
//	float color[3];
//
//	BasicVertex() { pos[0] = 0.0f; pos[1] = 0.0f; pos[2] = 0.0f; color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; }
//	BasicVertex(float x, float y, float z) { pos[0] = x; pos[1] = y; pos[2] = z; color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; }
//	BasicVertex(float x, float y, float z, float r, float g, float b) {
//		pos[0] = x; pos[1] = y; pos[2] = z; color[0] = r; color[1] = g; color[2] = b;
//	}
//	BasicVertex operator+(const BasicVertex& rhs) {
//		return BasicVertex(pos[0] + rhs.pos[0], pos[1] + rhs.pos[1], pos[2] + rhs.pos[2]);
//	}
//	BasicVertex operator+(const float& rhs) {
//		return BasicVertex(pos[1] + rhs, pos[1] + rhs, pos[2] += rhs);
//	}
//	BasicVertex operator*(const BasicVertex& rhs) {
//		return BasicVertex(pos[0] * rhs.pos[0], pos[1] * rhs.pos[1], pos[2] *= rhs.pos[2]);
//	}
//	BasicVertex operator*(const float& rhs) {
//		return BasicVertex(pos[0] * rhs, pos[1] * rhs, pos[2] * rhs);
//	}
//
//} BasicVertex;

// Data structure for basic geometry
typedef struct BasicVertex {
	float pos[3];
	float uvx, uvy;

	BasicVertex() { pos[0] = 0.0f; pos[1] = 0.0f; pos[2] = 0.0f; uvx = 1.0f; uvy = 1.0f; }
	BasicVertex(float x, float y, float z) { pos[0] = x; pos[1] = y; pos[2] = z; uvx = 1.0f; uvy = 1.0f; }
	BasicVertex(float x, float y, float z, float uvx, float uvy) {
		pos[0] = x; pos[1] = y; pos[2] = z; this->uvx = uvx; this->uvy = uvy;
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

// The Material struct will store the texture and sampler for each object
struct Material
{
	sce::Agc::Core::Texture texture;
	sce::Agc::Core::Sampler sampler;
};

enum ObjectType {
	EmptyPlace = 0,
	Wall = 1,
	Player = 2,
	Collectible =3,
	Ghost = 4,
	Screen = 5
};

// Base class for all games that need a rendering loop
class RenderManager
{
private:
	int windowX;
	int windowY;
	uint32_t backBufferIndex = 0;
	uint64_t frameNumber = 0;
	uint32_t numObjects = 0;

	sce::Agc::Core::Buffer vertexBuffer;

	sce::Agc::Core::Buffer matBuffer;

	// Texture info
	std::vector<Material> mats;

	// Rectangle Info
	uint32_t numRectangles = 0;
	uint32_t indicesPerRect;
	uint16_t* rectangleIndexBuffer = nullptr;

	// Rec objects under rectangle
	uint32_t numWall = 0;
	uint32_t numPlayer = 0;
	uint32_t numGhost = 0;
	uint32_t numCollectible = 0;

	// Circle Info
	uint32_t numCircles = 0;
	uint32_t indicesPerCircle;
	uint16_t* circleIndexBuffer = nullptr;

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

public:
	RenderManager();
	RenderManager(int windowSizeX, int windowSizeY);
	~RenderManager(void);
	void init();
	void drawScene(int mazeTemplate[][42], vector<pair<int,int>> collectibleIDs, int gameState); // draws all drawable components
	void setWindowSize(int winX, int winY) { windowX = winX; windowY = winY; }
	void setClearColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a) { 
		rtClearValue = sce::Agc::Core::Encoder::raw(r, g, b, a); }	
	void createBasicGeometry(float gridSize);
	void createRect(uint32_t numRect, ObjectType objType);
	void createCircle(uint32_t numRect);
	Matrix4* createViewMatrix();
	Matrix4 creatOriginViewMatrix();

	void LoadTextures();
}; 
