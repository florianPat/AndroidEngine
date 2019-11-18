#pragma once

#include "Types.h"
#include "VulkanTexture.h"
#include "GraphicsVK.h"
#include "Mat4x4.h"

class VulkanRenderTexture
{
	static constexpr VkFormat FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VulkanTexture texture;
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	CommandBuffer commandBuffer;
	VulkanFence fence;
	VkClearValue clearValue = {};
    Mat4x4 orhtoProj;

	GraphicsVK* gfx = nullptr;

	//TODO: Think about sharing these!
	VkRenderPass renderPass;
	VkPipeline pipeline;
public:
    VulkanRenderTexture() = default;
    VulkanRenderTexture(const VulkanRenderTexture& other) = delete;
    VulkanRenderTexture(VulkanRenderTexture&& other) noexcept;
    VulkanRenderTexture& operator=(const VulkanRenderTexture& rhs) = delete;
    VulkanRenderTexture& operator=(VulkanRenderTexture&& rhs) noexcept;
    ~VulkanRenderTexture();
    void create(uint32_t width, uint32_t height, GraphicsVK* gfx);
    const VulkanTexture& getTexture() const;
    void begin();
    void end();
    explicit operator bool() const;
	void changeToShaderReadMode();
private:
	void createFramebufferTextureAndView(uint32_t width, uint32_t height);
};
