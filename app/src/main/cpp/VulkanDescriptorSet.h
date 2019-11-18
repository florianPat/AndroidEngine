#pragma once

#include "vulkan_wrapper.h"

class VulkanDescriptorSet
{
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDevice device = nullptr;
public:
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
public:
    VulkanDescriptorSet() = default;
    VulkanDescriptorSet(VkDevice device, VkDescriptorType type, VkDescriptorSetLayout setLayout);
    VulkanDescriptorSet(const VulkanDescriptorSet& other) = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&& other);
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet& rhs) = delete;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&& rhs);
    ~VulkanDescriptorSet();
    void updateUniformBuffer(VkDescriptorBufferInfo* descriptorBufferInfo, uint32_t binding = 0);
    void updateImage(VkImageView imageView, uint32_t binding = 0);
    void updateSampler(VkSampler sampler, uint32_t binding = 0);
    void bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t offset);
};
