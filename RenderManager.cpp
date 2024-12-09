#include "RenderManager.h"

// These symbols point to the headers and code of the shader binaries linked into the sample's elf.
// They are declared inside the shader code. For example, the ShaderText::ps_header and ShaderText::ps_text symbols
// were declared in the shader by putting the attribute [CxxSymbol("ShaderText::ps")] in front of the pixel
// shader's entry point. Note: in the PSSL compiler you MUST turn Embed Shader Code to On for this to work.
namespace ShaderText
{
	extern char  ps_header[];
	extern const char  ps_text[];
	extern char  gs_header[];
	extern const char  gs_text[];
}
/////////////////////////////////
// Memory allocators
/////////////////////////////////
// Map a memory range so that the GPU can read it
void mapGpuMem(void const* start, size_t size)
{
	int ret = sceKernelMprotect(start, size, SCE_KERNEL_PROT_CPU_READ | SCE_KERNEL_PROT_GPU_READ);
	SCE_AGC_ASSERT_MSG(ret == SCE_OK, "sceKernelMprotect() returns error=0x%08x\n", ret);
}

// Allocate and map a memory range so that the GPU can read it
uint8_t* allocDmem(sce::Agc::SizeAlign sizeAlign)
{
	if (!sizeAlign.m_size)
	{
		return nullptr;
	}

	static uint32_t allocCount = 0;
	off_t offsetOut;

	const size_t alignment = (sizeAlign.m_align + 0xffffu) & ~0xffffu;
	const uint64_t size = (sizeAlign.m_size + 0xffffu) & ~0xffffu;

	int32_t ret = sceKernelAllocateMainDirectMemory(size, alignment, SCE_KERNEL_MTYPE_C_SHARED, &offsetOut);
	if (ret) {
		printf("sceKernelAllocateMainDirectMemory error:0x%x size:0x%zx\n", ret, size);
		return nullptr;
	}

	void* ptr = NULL;
	char namedStr[32];
	snprintf_s(namedStr, sizeof(namedStr), "TriangleWithController %d_%zuKB", allocCount++, size >> 10);
	ret = sceKernelMapNamedDirectMemory(&ptr, size, SCE_KERNEL_PROT_GPU_RW | SCE_KERNEL_PROT_CPU_RW, 0, offsetOut, alignment, namedStr);
	SCE_AGC_ASSERT_MSG(ret == SCE_OK, "Unable to map memory");
	return (uint8_t*)ptr;
}

//////////////////////////////////////////
// Utility functions for rendering
//////////////////////////////////////////

// Create the scanout video handle
int createScanoutBuffers(const sce::Agc::CxRenderTarget * rts, uint32_t count)
{
	// First we need to select what we want to display on, which in this case is the TV, also known as SCE_VIDEO_OUT_BUS_TYPE_MAIN.
	int videoHandleLocal = sceVideoOutOpen(SCE_USER_SERVICE_USER_ID_SYSTEM, SCE_VIDEO_OUT_BUS_TYPE_MAIN, 0, NULL);
	SCE_AGC_ASSERT_MSG(videoHandleLocal >= 0, "sceVideoOutOpen() returns handle=%d\n", videoHandle);

	// Next we need to inform scan-out about the format of our buffers. This can be done by directly talking to VideoOut or
	// by letting Agc::Core do the translation. To do so, we first need to get a RenderTargetSpec, which we can extract from
	// the list of CxRenderTargets passed into the function.
	sce::Agc::Core::RenderTargetSpec spec;
	SceError error = sce::Agc::Core::translate(&spec, &rts[0]);
	SCE_AGC_ASSERT(error == SCE_OK);

	// Next, we use this RenderTargetSpec to create a SceVideoOutBufferAttribute2 which tells VideoOut how it should interpret
	// our buffers. VideoOut needs to know how the color data in the target should be interpreted, and since our pixel shader has
	// been writing linear values into an sRGB RenderTarget, the data VideoOut will find in memory are sRGB encoded.
	SceVideoOutBufferAttribute2 attribute;
	error = sce::Agc::Core::translate(&attribute, &spec, sce::Agc::Core::Colorimetry::kSrgb, sce::Agc::Core::Colorimetry::kBt709);
	SCE_AGC_ASSERT(error == SCE_OK);

	// Ideally, all buffers should be registered with VideoOut in a single call to sceVideoOutRegisterBuffers2.
	// The reason for this is that the buffers provided in each call get associated with one attribute slot in the API.
	// Even if consecutive calls pass the same SceVideoOutBufferAttribute2 into the function, they still get assigned
	// new attribute slots. When processing a flip, there is significant extra cost associated with switching attribute
	// slots, which should be avoided.
	SceVideoOutBuffers* addresses = (SceVideoOutBuffers*)calloc(count, sizeof(SceVideoOutBuffers));
	for (uint32_t i = 0; i < count; ++i)
	{
		// We could manually call into VideoOut to set up the scan-out buffers, but Agc::Core provides a helper for this.
		addresses[i].data = rts[i].getDataAddress();
	}

	// VideoOut internally groups scan-out buffers in sets. Every buffer in a set has the same attributes and switching (flipping) between
	// buffers of the same set is a light-weight operation. Switching to a buffer from a different set is significantly more expensive
	// and should be avoided. If an application wants to change the attributes of a scan-out buffer or wants to unregister buffers,
	// these operations are done on whole sets and affect every buffer in the set. This sample only registers one set of buffers and never
	// modifies the set.
	const int32_t setIndex = 0; // Call sceVideoOutUnregisterBuffers with this.
	error = sceVideoOutRegisterBuffers2(videoHandleLocal, setIndex, 0, addresses, count, &attribute, SCE_VIDEO_OUT_BUFFER_ATTRIBUTE_CATEGORY_UNCOMPRESSED, nullptr);
	SCE_AGC_ASSERT(error == SCE_OK);
	free(addresses);

	return videoHandleLocal;
}

// Create a set of render targets
void CreateRenderTargets(sce::Agc::CxRenderTarget* rts, sce::Agc::Core::RenderTargetSpec* spec, uint32_t count)
{
	// First, retrieve the size of the render target. We can of course do this before we have any pointers.
	sce::Agc::SizeAlign rtSize = sce::Agc::Core::getSize(spec);
	// Then we can allocate the required memory backing and assign it to the spec.
	spec->m_dataAddress = allocDmem(rtSize);
	memset((void*)spec->m_dataAddress, 0x80, rtSize.m_size);

	// We can now initialize the render target. This will check that the dataAddress is properly aligned.
	SceError error = sce::Agc::Core::initialize(&rts[0], spec);
	SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize RenderTarget.");
	sce::Agc::Core::registerResource(&rts[0], "Color %d", 0);

	// Now that we have the first RT set up, we can create the others. They are identical to the first material, except for the RT memory.
	for (uint32_t i = 1; i < count; ++i)
	{
		// You can just memcpy the CxRenderTarget, but doing so of course sidesteps the alignment checks in initialize().
		memcpy(&rts[i], &rts[0], sizeof(rts[0]));
		rts[i].setDataAddress(allocDmem(rtSize));
		memset(rts[i].getDataAddress(), 0x80, rtSize.m_size);
		sce::Agc::Core::registerResource(&rts[i], "Color %d", i);
	}
}

void CreateDepthRenderTarget(sce::Agc::CxDepthRenderTarget* drt, sce::Agc::Core::DepthRenderTargetSpec* spec)
{
	// First, retrieve the size of the depth render target. We can of course do this before we have any pointers.
	sce::Agc::SizeAlign drtSize = sce::Agc::Core::getSize(spec);
	// Then we can allocate the required memory backing and assign it to the spec.
	spec->m_depthReadAddress = spec->m_depthWriteAddress = allocDmem(drtSize);

	// Second, retrieve the size of htile buffer for this depth render target. We can also do this before we have any pointers.
	sce::Agc::SizeAlign htileSize = sce::Agc::Core::getSize(spec, sce::Agc::Core::DepthRenderTargetComponent::kHtile);
	// Then we can allocate the required memory backing and assign it to the spec.
	spec->m_htileAddress = allocDmem(htileSize);

	// We can now initialize the depth render target. This will check that the addresses are properly aligned.
	SceError error = sce::Agc::Core::initialize(drt, spec);
	SCE_AGC_ASSERT_MSG(error == SCE_OK, "Failed to initialize DepthRenderTarget.");
	sce::Agc::Core::registerResource(drt, "Depth");

	// The depth render targets also own the clear value, so we can just update the DRT directly now.
	drt->setDepthClearValue(1.0f);
}

//////////////////////////////////////////////////////
// Class to handle initialization and rendering
//////////////////////////////////////////////////////

// sets up the window size and item count to 0
RenderManager::RenderManager(int windowSizeX, int windowSizeY)
{
	this->windowX = windowSizeX;
	this->windowY = windowSizeY;
}

// sets up a default window size along with double buffering
RenderManager::RenderManager() 
{
	RenderManager(DISPLAY_WIDTH, DISPLAY_HEIGHT); 
}

// default destructor: no longer need to shut things
// down since there's a suspend point in drawing now
RenderManager::~RenderManager(void)
{
}

Object RenderManager::createObject(BasicVertex* vertices, uint32_t numVerts, uint16_t* indices, uint32_t numIndices)
{
	Object obj;

	// copy vertices into local memory
	BasicVertex* vertBuf = (BasicVertex*)allocDmem({ numVerts * sizeof(BasicVertex), sce::Agc::Alignment::kBuffer });
	memcpy(vertBuf, vertices, numVerts * sizeof(BasicVertex));

	obj.vertices = vertBuf;
	obj.vertexCount = numVerts;
	// create vertex buffer from above vertices
	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(vertBuf, sizeof(BasicVertex), numVerts);
	SceError error = sce::Agc::Core::initialize(&obj.vertexBuffer, &bufSpec);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(&obj.vertexBuffer, "Vertices");

	// copy indices into new mesh index buffer
	obj.indexBuffer = (uint16_t*)allocDmem({ numIndices * sizeof(uint16_t), sce::Agc::Alignment::kBuffer });
	memcpy(obj.indexBuffer, indices, numIndices * sizeof(uint16_t));
	obj.indexCount = numIndices;

	// TODO: Change the Translation with the Matrix?
	obj.transform = Matrix4::translation({ 0, 0, 0 });

	allObjects.push_back(obj);
	return obj;
}



void RenderManager::createBasicGeometry()
{
	// ------- Constants --------------------------------
	// circle
	const uint32_t circleSegments = 12;
	const uint32_t numCircleVerts = circleSegments + 1;
	const uint32_t numCircleIndices = circleSegments * 3;
	indicesPerCircle = numCircleIndices;

	// rect
	const uint32_t numRectVerts = 4;
	const uint32_t numRectIndices = 6;
	indicesPerRect = numRectIndices;

	const uint32_t numVerts = numRectVerts + numCircleVerts;

	// ------- Allocate Memory for vertex and index buffer -----------------

	// Rect and Circle will use the same vertext buffer but different index buffer
	BasicVertex* vertices = (BasicVertex*)allocDmem({ numVerts * sizeof(BasicVertex), sce::Agc::Alignment::kBuffer });
	uint16_t* CircleIndices = (uint16_t*)allocDmem({ numCircleIndices * sizeof(uint16_t), sce::Agc::Alignment::kBuffer });
	uint16_t* RectIndices = (uint16_t*)allocDmem({ numRectIndices * sizeof(uint16_t), sce::Agc::Alignment::kBuffer });

	// -------- Making Vertex Buffer and Index buffer -------------------

	uint32_t v = 0;

	// circle
	vertices[v++] = { 0.0f, 0.0f, -0.25f, 1.0f, 1.0f, 1.0f }; // center

	for (uint16_t i = 1; i <= circleSegments; ++i) {
		float angle = 2.0f * M_PI * i / circleSegments; // Angle in radians
		float x = 0.05f * cos(angle); // Radius of 0.05f for the circle
		float y = 0.05f * sin(angle);
		vertices[v++] = { x, y, -0.25f, 1.0f, 1.0f, 1.0f };
	}

	for (int i = 0; i < circleSegments; ++i) {
		CircleIndices[i * 3] = 0;            // Center vertex index (always 0)
		CircleIndices[i * 3 + 1] = i + 1;    // Current vertex index (1 to SEGMENTS)
		CircleIndices[i * 3 + 2] = (i + 1) % circleSegments + 1; // Next vertex index, wraps around
	}

	circleIndexBuffer = CircleIndices;

	// rect
	vertices[v++] = { -0.25f, -0.25f, -0.25f, 0.25f, 1.0f, 0.25f };
	vertices[v++] = { 0.25f, -0.25f, -0.25f, 0.25f, 1.0f, 0.9f };
	vertices[v++] = { -0.25f, 0.25f, -0.25f, 0.9f, 1.0f, 0.25f };
	vertices[v++] = { 0.25f, 0.25f, -0.25f, 0.9f, 1.0f, 0.9f };

	// Index buffer
	// Rect Triangle 1
	RectIndices[0] = 0 + numCircleVerts;
	RectIndices[1] = 1 + numCircleVerts;
	RectIndices[2] = 2 + numCircleVerts;
	// Rect Triangle 2
	RectIndices[3] = 3 + numCircleVerts;
	RectIndices[4] = 2 + numCircleVerts;
	RectIndices[5] = 1 + numCircleVerts;

	rectangleIndexBuffer = RectIndices;

	// --------- Bind Vertex buffer --------------------------------------------
	sce::Agc::Core::BufferSpec bufSpec;
	bufSpec.initAsRegularBuffer(vertices, sizeof(BasicVertex), numVerts);

	SceError error = sce::Agc::Core::initialize(&vertexBuffer, &bufSpec);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(&vertexBuffer, "Vertex");
}

void RenderManager::createRect(uint32_t numRect)
{
	numObjects += numRect;
	numRectangles = numRect;
}

void RenderManager::createCircle(uint32_t numCircle)
{
	numObjects += numCircle;
	numCircles = numCircle;
}

// this view matrix looks on z axis and y is up.
Matrix4* RenderManager::createViewMatrix() {

	Matrix4* matrices = (Matrix4*)allocDmem({ sizeof(Matrix4) * numObjects, alignof(Matrix4) });
	auto aspect = (float)windowX / windowY;

	Matrix4 projMatrix = Matrix4::perspective(90.f * float(1.74532925199432957692e-2), aspect, 0.05, 10);

	Matrix4 cameraMatrix = Matrix4::lookAt({ 0,0,1 }, { 0,0,0 }, { 0,1,0 });

	for (uint i = 0; i < numObjects; i++)
	{
		matrices[i] = projMatrix * cameraMatrix * Matrix4::translation({ 0, 0, 0 }) * Matrix4::rotationZYX({ 0, 0, 0 }) * Matrix4::scale({ 1.0, 1.0, 1.0 });
	}

	sce::Agc::Core::BufferSpec spec;
	spec.initAsRegularBuffer(matrices, sizeof(Matrix4), numObjects);

	SceError error = sce::Agc::Core::initialize(&matBuffer, &spec);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(&matBuffer, "MatBuffer");
	return matrices;

}

Matrix4 RenderManager::creatOriginViewMatrix() {


	Matrix4 origin;// = (Matrix4*)allocDmem({ sizeof(Matrix4) * 1, alignof(Matrix4) });
	auto aspect = (float)windowX / windowY;

	Matrix4 projMatrix = Matrix4::perspective(90.f * float(1.74532925199432957692e-2), aspect, 0.05, 10);

	Matrix4 cameraMatrix = Matrix4::lookAt({ 0,0,1 }, { 0,0,0 }, { 0,1,0 });

	origin = projMatrix * cameraMatrix * Matrix4::translation({ 0, 0, 0 }) * Matrix4::rotationZYX({ 0, 0, 0 }) * Matrix4::scale({ 1.0, 1.0, 1.0 });
	return origin;
}

void RenderManager::updatePaddlePosition(float dx, float dy)
{
	updateObjectPosition(1, dx, dy);
}

void RenderManager::updateBallPosition(float dx, float dy)
{
	updateObjectPosition(0, dx, dy);
}

vector<Object>& RenderManager::GetAllObjects()
{
	return allObjects;
}

void RenderManager::updateObjectPosition(int i, float dx, float dy)
{
	for (int j = 0; j < allObjects[i].vertexCount; ++j) {
		allObjects[i].vertices[j].pos[0] += dx; // Update X position
		allObjects[i].vertices[j].pos[1] += dy; // Update Y position
	}
}

// sets up an ortho projection
// with a clear color of black
void RenderManager::init()
{ 
	// first buffer index is 0
	backBufferIndex = 0;

	// init all basic gpu functions
	error = sce::Agc::init();
	SCE_AGC_ASSERT(error == SCE_OK);

	size_t resourceRegistrationBufferSize;
	error = sce::Agc::ResourceRegistration::queryMemoryRequirements(&resourceRegistrationBufferSize, 128, 64);
	if (error != SCE_AGC_ERROR_RESOURCE_REGISTRATION_NO_PA_DEBUG)
	{
		SCE_AGC_ASSERT(error == SCE_OK);
		error = sce::Agc::ResourceRegistration::init(allocDmem({ resourceRegistrationBufferSize, sce::Agc::Alignment::kResourceRegistration }), resourceRegistrationBufferSize, 64);
		SCE_AGC_ASSERT(error == SCE_OK);
		error = sce::Agc::ResourceRegistration::registerDefaultOwner(nullptr);
		SCE_AGC_ASSERT(error == SCE_OK);
	}

	// toolkit used for things like clearing the screen
	error = sce::Agc::Toolkit::init();
	SCE_AGC_ASSERT(error == SCE_OK);

	// First, we load the shaders, since the size of the shader's register blocks is not known.
	error = sce::Agc::createShader(&gs, ShaderText::gs_header, ShaderText::gs_text);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(gs, "ShaderText::gs");

	error = sce::Agc::createShader(&ps, ShaderText::ps_header, ShaderText::ps_text);
	SCE_AGC_ASSERT(error == SCE_OK);
	sce::Agc::Core::registerResource(ps, "ShaderText::ps");

	// Set up the RenderTarget spec.
	rtSpec.init();
	rtSpec.m_width = windowX;
	rtSpec.m_height = windowY;
	rtSpec.m_format = { sce::Agc::Core::TypedFormat::k8_8_8_8Srgb, sce::Agc::Core::Swizzle::kRGBA_R4S4 };
	rtSpec.m_tileMode = sce::Agc::CxRenderTarget::TileMode::kRenderTarget;

	// Now we create a number of render targets from this spec. These are our scanout buffers.
	CreateRenderTargets(rts, &rtSpec, BUFFER_NUMBER);

	// Set up the DepthRenderTarget spec.
	drtSpec.init();
	drtSpec.m_width = windowX;
	drtSpec.m_height = windowY;
	drtSpec.m_depthFormat = sce::Agc::CxDepthRenderTarget::DepthFormat::k32Float;
	drtSpec.m_compression = sce::Agc::Core::MetadataCompression::kHtileDepth;

	// Now we create a DepthRenderTarget using the spec.
	CreateDepthRenderTarget(&drt, &drtSpec);

	// These labels are currently unused, but the intent is to use them for flip tracking.
	flipLabels = (sce::Agc::Label*)allocDmem({ sizeof(sce::Agc::Label) * BUFFER_NUMBER, sce::Agc::Alignment::kLabel });

	// We need the videoout handle to flip.
	videoHandle = createScanoutBuffers(rts, BUFFER_NUMBER);

	// Create a context for each buffered frame.
	const uint32_t dcb_size = 1024 * 1024;
	
	for (uint32_t i = 0; i < BUFFER_NUMBER; ++i)
	{
		// Contexts are manually initialized. The reason is that we have no fixed requirements
		// for how the components have to be hooked up, so there is considerable freedom for the 
		// developer here.
		// In this case, we simply make all components use the DCB for their storage.

		ctxs[i].m_dcb.init(
			allocDmem({ dcb_size, 4 }),
			dcb_size,
			nullptr,
			nullptr);
		ctxs[i].m_bdr.init(
			&ctxs[i].m_dcb,
			&ctxs[i].m_dcb);
		ctxs[i].m_sb.init(
			256, // This is the size of a chunk in the StateBuffer, defining the largest size of a load packet's payload.
			&ctxs[i].m_dcb,
			&ctxs[i].m_dcb);
		flipLabels[i].m_value = 1; // 1 means "not used by GPU"
		sce::Agc::Core::registerResource(&ctxs[i].m_dcb, "Context %d", i);
		sce::Agc::Core::registerResource(&flipLabels[i], "Flip label %d", i);
	}

	// Initialize some state. We don't have to do this every loop through the frame, so we do it here.
	rtMask = sce::Agc::CxRenderTargetMask().init().setMask(0, 0xf);

	// Set up a viewport using a helper function from Core.
	// note that -1 to 1 is a normal range for beginning graphics apps
	sce::Agc::Core::setViewport(& vport, windowX, windowY, 0, 0, -1.0f, 1.0f);

	// By default back-face culling is disabled, but for performance reasons it is beneficial to enable culling and so we
	// need to initialize this register in order to enable it.
	primSetup.init()
		.setCullFace(sce::Agc::CxPrimitiveSetup::CullFace::kBack);

	// By default depth testing is also disabled, but since we have a DepthRenderTarget we want to actually enable it.
	depthStencilControl.init();
	depthStencilControl.setDepthWrite(sce::Agc::CxDepthStencilControl::DepthWrite::kEnable);
	depthStencilControl.setDepth(sce::Agc::CxDepthStencilControl::Depth::kEnable);
	depthStencilControl.setDepthFunction(sce::Agc::CxDepthStencilControl::DepthFunction::kLess);

	// This example has very little controller input, but real game applications should try to minimize their input latency.
	// VideoOut provides a mechanism to measure the latency between a defined point on the CPU timeline and when the framebuffer
	// gets flipped. It also provides a mechanism to prevent the CPU from racing too far ahead of the GPU in GPU bound cases,
	// allowing you to control the input latency of your title. For this sample, we simply try to hit a 16.6ms latency.
	// Enable the "Performance Overlay" in Target Settings to show the latency on screen.
	latencyControl.control = SCE_VIDEO_OUT_LATENCY_CONTROL_WAIT_BY_FLIP_QUEUE_NUM; // This allows us to simply pass the frame number as the flipArg.
	latencyControl.targetNum = 1; // Allow for the CPU to get one frame ahead. This means the duration of CPU + GPU processing should be less than one frame to hold framerate. In your game application, you probably want this to be 2 or 3.
	latencyControl.extraUsec = 0; // For simplicity we're assuming the CPU + GPU processing of each frame takes the full frame time. You will want to adjust this in your game application to reduce latency.
}

void RenderManager::drawScene() {
	
	// First we identify the back buffer.
	const uint32_t buffer = backBufferIndex;

		// Check if the command buffer has been fully processed, if so it's safe for us to overwrite it on the CPU.
		while (flipLabels[buffer].m_value != 1)
		{
			sceKernelUsleep(1000);
		}

		// We can now set the flip label to 0, which the GPU will set back to 1 when it's done.
		flipLabels[buffer].m_value = 0;

		// Delay processing on the CPU to control latency. If this causes GPU stalls, you need to increase targetNum or decrease extraUsec in latencyControl.
		error = sceVideoOutLatencyControlWaitBeforeInput(videoHandle, &latencyControl, nullptr);
		SCE_AGC_ASSERT(error == SCE_OK);
		// Notify VideoOut that we are about to begin processing the frame. This the start point of the latency computation.
		error = sceVideoOutLatencyMeasureSetStartPoint(videoHandle, frameNumber); // Pass frame number as flipArg for both this an setFlip, so VideoOut knows they are the same frame.
		SCE_AGC_ASSERT(error == SCE_OK);

		sce::Agc::Core::BasicContext& ctx = ctxs[buffer];
		// First we reset the context, since we're writing a completely new DCB.
		// This is actually quite wasteful, since we could reuse the previous data, but the
		// point of this code is to demonstrate a Gnm-like approach to writing DCBs.
		ctx.reset();

		// This will stall the Command Processor (CP) until the buffer is no longer being displayed.
		// Note that we're actually pulling the DCB out of the context and accessing it
		// directly here. This is very much how Agc's contexts work. They do not hide away the underlying
		// components but mostly just try to remove redundant work.
		ctx.m_dcb.waitUntilSafeForRendering(videoHandle, buffer);

		// Clear our current RenderTarget by using Agc::Toolkit.
		sce::Agc::Toolkit::Result toolkitResult1 = sce::Agc::Toolkit::clearRenderTargetCs(&ctx.m_dcb, &rts[buffer], rtClearValue);
		SCE_AGC_ASSERT(toolkitResult1.m_errorCode == SCE_OK);

		// Clear our current DepthRenderTarget by using Agc::Toolkit.
		sce::Agc::Toolkit::Result toolkitResult2 = sce::Agc::Toolkit::clearDepthRenderTargetCs(&ctx.m_dcb, &drt);
		SCE_AGC_ASSERT(toolkitResult2.m_errorCode == SCE_OK);

		// Merge the two toolkit results together so that we can issue a single gpuSyncEvent for both toolkit
		// operations.
		toolkitResult1 = toolkitResult1 | toolkitResult2;

		// Before we start rendering the scene we need to insert the proper sync events to wait for the previous
		// Toolkit operation to complete. All Agc::Toolkit methods return a special value which encodes which caches
		// need flushed, what kind of synchronization is needed, and what shader state needs reset.
		//
		// Normally we could just call resetToolkitChangesAndSyncToGl2 on whichever Agc Context we are using,
		// but for this sample we are not using a Context object so we need to perform the same operations manually.
		// At this point in the frame no shaders have been set, so we don't need to worry about resetting any shader
		// state. However, we do need to flush out some caches and wait for the previous operation to finish. In this
		// case we can just use a gpuSyncEvent since the Agc::Toolkit::Result has a couple of helper functions which
		// can convert it to the parameters that gpuSyncEvent expects.
		//
		// Also, since all accesses to RenderTargets go through the Gl2 now there is no reason to flush all the way
		// out to memory.
		sce::Agc::Core::gpuSyncEvent(&ctx.m_dcb,
			toolkitResult1.getSyncWaitMode(),
			toolkitResult1.getSyncCacheOp(sce::Agc::Toolkit::Result::Caches::kGl2));

		// Setting state can be done in several ways, such as by directly interacting with the DCB.
		// For the most part, contexts are designed to use StateBuffers (SBs), which allow the user
		// to have their indirect state turn into something that behaves a lot like direct state.
		// The easiest way to pass state into the StateBuffer is with the setState template method. This method
		// will look for a static const RegisterType member called m_type to determine what type of register
		// is being set and automatically determines the size of the state from the type being passed in.
		ctx.m_sb.setState(rtMask);
		ctx.m_sb.setState(rts[buffer]);
		ctx.m_sb.setState(drt);
		ctx.m_sb.setState(vport);
		ctx.m_sb.setState(primSetup);
		ctx.m_sb.setState(depthStencilControl);

		// The easiest way to draw multiple objects is just to issue a draw call per object.
		// This has the advantage of being very simple to implement but will have higher CPU cost than the other
		// methods because of all the draw calls that need to be generated.

		// In this case we are using the same set of shaders for all draw calls, so we just set it once before we get to the draw loop.
		ctx.setShaders(nullptr, gs, ps, sce::Agc::UcPrimitiveType::Type::kTriList);

		uint32_t drawIndex = 0;
		//for (uint32_t i = 0; i < numObjects; i++)
		//{
		//	if (allObjects[i].isActive)
		//	{
		//		// Now we have to get the inputs for the draw calls ready. One thing that is slightly special is that we compiled the
		//		// vertex shader using the S_DRAW_ID semantic. The way this normally works is that a single User SGPR is reserved by
		//		// the shader compiler and the HW will automatically set this User SGPR as part of issuing the multi-draw. However,
		//		// in this case we are not using a multi-draw which means no free draw ID. Instead, we will use the Binder to set this 
		//		// User SGPR by calling setDrawIndex(). Since this is the per-draw ID it needs to be updated for every draw call.
		//		ctx.m_bdr.getStage(sce::Agc::ShaderType::kGs)
		//			.setBuffers(0, 1, &allObjects[i].vertexBuffer)
		//			.setBuffers(1, 1, &matBuffer)
		//			.setDrawIndex(drawIndex);

		//		// this would need to change if objects with differing numbers of indices were drawn
		//		ctx.drawIndex(allObjects[i].indexCount, allObjects[i].indexBuffer);

		//		drawIndex++;
		//	}
		//}

		for (uint32_t i = 0; i < numCircles; i++)
		{
			ctx.m_bdr.getStage(sce::Agc::ShaderType::kGs)
				.setBuffers(0, 1, &vertexBuffer)
				.setBuffers(1, 1, &matBuffer)
				.setDrawIndex(i);

			ctx.drawIndex(indicesPerCircle, circleIndexBuffer);
		}

		for (uint32_t i = 0; i < numRectangles; i++)
		{
			ctx.m_bdr.getStage(sce::Agc::ShaderType::kGs)
				.setBuffers(0, 1, &vertexBuffer)
				.setBuffers(1, 1, &matBuffer)
				.setDrawIndex(i + numCircles);

			ctx.drawIndex(indicesPerRect, rectangleIndexBuffer);
		}

		// Submit a flip via the GPU.
		// Note: on Prospero, RenderTargets write into the GL2 cache, but the scan-out
		// does not snoop any GPU caches. As such, it is necessary to flush these writes to memory before they can
		// be displayed. This flush is performed internally by setFlip() so we don't need to do it
		// on the application side.
		ctx.m_dcb.setFlip(videoHandle, buffer, SCE_VIDEO_OUT_FLIP_MODE_VSYNC, frameNumber); // Pass frame number as flipArg to match sceVideoOutLatencyMeasureSetStartPoint().

		// The last thing we do in the command buffer is write 1 to the flip label to signal that command buffer
		// processing has finished. 
		//
		// While Agc provides access to the lowest level of GPU synchronization faculties, it also provides
		// functionality that builds the correct synchronization steps in an easier fashion.
		// Since synchonization should be relatively rare, spending a few CPU cycles on letting the library
		// work out what needs to be done is generally a good idea.
		sce::Agc::Core::gpuSyncEvent(
			&ctx.m_dcb,
			// The SyncWaitMode controls how the GPU's Command Processor (CP) handles the synchronisation.
			// By setting this to kAsynchronous, we tell the CP that it doesn't have to wait for this operation
			// to finish before it can start the next frame. Instead, we could ask it to drain all graphics work
			// first, but that would be more aggressive than we need to be here.
			sce::Agc::Core::SyncWaitMode::kAsynchronous,
			// Since we are making the label write visible to the CPU, it is not necessary to flush any caches
			// and we set the cache op to 'kNone'.
			sce::Agc::Core::SyncCacheOp::kNone,
			// Write the flip label and make it visible to the CPU.
			sce::Agc::Core::SyncLabelVisibility::kCpu,
			&flipLabels[buffer],
			// We write the value "1" to the flip label.
			1);

		// Finally, we submit the work to the GPU. Since this is the only work on the GPU, we set its priority to normal.
		// The only reason to set the priority to kInterruptPriority is to make a submit expel work from the GPU we have previously
		// submitted. 
		error = sce::Agc::submitGraphics(
			sce::Agc::GraphicsQueue::kNormal,
			ctx.m_dcb.getSubmitPointer(),
			ctx.m_dcb.getSubmitSize());
		SCE_AGC_ASSERT(error == SCE_OK);

		// update the buffer we're dealing with
		backBufferIndex = (backBufferIndex + 1) % BUFFER_NUMBER;
		
		// update frame number
		frameNumber++;

		// If the application is suspended, it will happen during this call. As a side-effect, this is equivalent to
		// calling initializeDefaultHardwareState().
		error = sce::Agc::suspendPoint();
		SCE_AGC_ASSERT(error == SCE_OK);
}


