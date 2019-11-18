#pragma once

#include "vulkan_wrapper.h"
#include "String.h"
#include "GraphicsVKIniter.h"
#include "VulkanFence.h"

class VulkanTexture
{
    uint32_t width = 0;
    uint32_t height = 0;
    VkImage texture = VK_NULL_HANDLE;
    VkImageView textureView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

	CommandBuffer commandBuffer;
	VulkanFence fence;

	static const GraphicsVKIniter* gfx;
public:
    void reloadFromFile(const String& filename);
public:
    VulkanTexture() = default;
    VulkanTexture(const String& filename);
    VulkanTexture(const void* buffer, uint32_t width, uint32_t height, VkFormat internalFormat = VK_FORMAT_R8G8B8A8_UNORM);
	VulkanTexture(VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags, bool setDefaultGraphicsFamilyIndex);
    VulkanTexture(const VulkanTexture& other) = delete;
    VulkanTexture(VulkanTexture&& other) noexcept;
    VulkanTexture& operator=(const VulkanTexture& rhs) = delete;
    VulkanTexture& operator=(VulkanTexture&& rhs) noexcept;
    ~VulkanTexture();
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    uint64_t getSize() const { return (((uint64_t)width) * height * sizeof(int32_t) + sizeof(VulkanTexture)); }
    explicit operator bool() const;
    const VkImageView getTextureView() const { return textureView; }
	const VkImage getTexture() const { return texture; }
    static void setStaticParameters(const GraphicsVKIniter* gfx);
	void changeVkImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage, bool setDefaultGraphicsFamilyIndex);
private:
	VkDeviceSize createVkImageAndView(const VkImageCreateInfo& imageCreateInfo,
		VkMemoryPropertyFlagBits memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uint8_t* startCreateGpuTexture(uint32_t graphicsFamilyIndex, VkFormat internalFormat);
    void finishCreateGpuTexture(uint32_t graphicsFamilyIndex, CommandBuffer& commandBuffer, VkQueue queue);
    uint32_t getRowPitch();
};
