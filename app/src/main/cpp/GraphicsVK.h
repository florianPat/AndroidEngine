#pragma once

#include "GraphicsVKIniter.h"
#include "Mat4x4.h"
#include "RectangleShape.h"
#include "UniquePtr.h"
#include "CircleShape.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanFence.h"

class Sprite;

class GraphicsVK : public GraphicsVKIniter
{
    struct Vertex
    {
        Vector2f position;
        Vector2f tex;
        float colorR = 0.0f, colorG = 0.0f, colorB = 0.0f, colorA = 0.0f;
		float mvMatrix[6] = { 0.0f };
    };
private:
	VulkanFence fence;
	uint32_t currentSwapchainImage = 0;
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	VkPresentInfoKHR presentInfo = {};

	static constexpr int32_t NUM_SPRITES_TO_BATCH = 32;
	static constexpr int32_t NUM_VERTICES_TO_BATCH = NUM_SPRITES_TO_BATCH * 4;
	int32_t nSpritesBatched = 0;
	UniquePtr<Vertex[]> vertices;
	VkImageView currentBoundTexture = VK_NULL_HANDLE;
	VulkanTexture blackTexture;
	CommandBuffer commandBuffers[NUM_BUFFERS];

	VkSampler sampler = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	VkClearValue clearValue = {};
	VkClearAttachment clearAttachment = {};
	VkClearRect clearRect = {};
	Mat4x4 orthoProj;

	CommandBuffer* currentCommandBuffer = nullptr;
	VulkanFence* currentFence = nullptr;
	VkRenderPassBeginInfo* currentRenderPassBeginInfo = nullptr;
	VkPipeline* currentPipeline = nullptr;
	VkSemaphore firstTimePresentComplete = VK_NULL_HANDLE;
public:
	VulkanShader shader;
	VulkanDescriptorSetLayout descriptorSetLayout;
    VulkanBuffer vertexBuffer;
    VulkanBuffer indexBuffer;
    VulkanBuffer uniformBuffer;
public:
    GraphicsVK(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType);

    void setupGfxGpu();
    void freeGfxGpu();

    void clear();
    void bindOtherOrthoProj(const Mat4x4& otherOrthoProj);
    void unbindOtherOrthoProj();
	void bindOtherCommandBuffer(CommandBuffer* commandBuffer, VulkanFence* fence, VkRenderPassBeginInfo* renderPassBeginInfo,
		VkPipeline* pipeline, const VkExtent2D& extent);
	void unbinOtherCommandBuffer();
    void draw(const Sprite& sprite);
    //TODO: Not working at the moment!
    void draw(const RectangleShape& rect);
    void draw(const CircleShape& circle);
    void flush();
    void render();
private:
    int32_t nVerticesBatched() const;
    void setClearAndPresentStructs();
};
