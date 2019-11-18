#include <utility>
#include "VulkanDescriptorSet.h"
#include <new>

VulkanDescriptorSet::VulkanDescriptorSet(VkDevice device, VkDescriptorType type, VkDescriptorSetLayout setLayout)
    : device(device)
{
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.descriptorCount = 1;
    descriptorPoolSize.type = type;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.pNext = nullptr;
    descriptorPoolCreateInfo.flags = 0;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.pNext = nullptr;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &setLayout;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;

    vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet);
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    if(descriptorSet != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
        descriptorSet = VK_NULL_HANDLE;
    }
    if(descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    device = nullptr;
}

void
VulkanDescriptorSet::updateUniformBuffer(VkDescriptorBufferInfo* descriptorBufferInfo, uint32_t binding)
{
    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
    writeDescriptorSet.pImageInfo = nullptr;
    writeDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSet::updateImage(VkImageView imageView, uint32_t binding)
{
    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = imageView;
    descriptorImageInfo.sampler = VK_NULL_HANDLE;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSet::updateSampler(VkSampler sampler, uint32_t binding)
{
    VkDescriptorImageInfo descriptorImageInfo = {};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = VK_NULL_HANDLE;
    descriptorImageInfo.sampler = sampler;

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.pNext = nullptr;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.dstSet = descriptorSet;
    writeDescriptorSet.pBufferInfo = nullptr;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;
    writeDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanDescriptorSet::bind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t offset)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            layout, offset, 1, &descriptorSet, 0, nullptr);
}

VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& other) : descriptorPool(std::exchange(other.descriptorPool, (VkDescriptorPool)VK_NULL_HANDLE)),
    device(std::exchange(other.device, nullptr)), descriptorSet(std::exchange(other.descriptorSet, (VkDescriptorSet)VK_NULL_HANDLE))
{
}

VulkanDescriptorSet& VulkanDescriptorSet::operator=(VulkanDescriptorSet&& rhs)
{
    this->~VulkanDescriptorSet();

    new (this) VulkanDescriptorSet(std::move(rhs));

    return *this;
}
