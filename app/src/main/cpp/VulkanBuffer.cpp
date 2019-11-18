#include <utility>
#include "VulkanBuffer.h"
#include "Utils.h"
#include "vulkan_wrapper.h"
#include <new>

VulkanBuffer::VulkanBuffer(void* data, uint32_t size, VkDevice device, uint32_t graphicsQueueFamilyIndex,
        VkBufferUsageFlags usageFlag, const GraphicsVKIniter* gfx) : device(device), size(size)
{
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext = nullptr;
    bufferCreateInfo.queueFamilyIndexCount = 1;
    bufferCreateInfo.pQueueFamilyIndices = &graphicsQueueFamilyIndex;
    bufferCreateInfo.flags = 0;
    bufferCreateInfo.size = (uint64_t) size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage = usageFlag;
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
	assert(memoryRequirements.memoryTypeBits != 0);
	uint32_t memoryTypeIndex = gfx->getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory);

    vkBindBufferMemory(device, buffer, memory, 0);

    assert(size <= memoryRequirements.size);
	if (data != nullptr)
	{
		subData(0, size, data);
	}
}

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept : device(std::exchange(other.device, nullptr)),
	size(std::exchange(other.size, 0)),
    buffer(std::exchange(other.buffer, (VkBuffer)VK_NULL_HANDLE)), memory(std::exchange(other.memory, (VkDeviceMemory)VK_NULL_HANDLE))
{
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& rhs) noexcept
{
    this->~VulkanBuffer();

    new (this) VulkanBuffer(std::move(rhs));

    return *this;
}

VulkanBuffer::~VulkanBuffer()
{
    if(buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
		vkFreeMemory(device, memory, nullptr);
		memory = VK_NULL_HANDLE;
        device = nullptr;
    }
}

VulkanBuffer::operator bool() const
{
    return (buffer != VK_NULL_HANDLE);
}

void VulkanBuffer::bindVertexBuffer(VkCommandBuffer commandBuffer)
{
	VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
}

void VulkanBuffer::bindIndexBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, buffer, 0, VK_INDEX_TYPE_UINT16);
}

void VulkanBuffer::subData(uint32_t offset, uint32_t size, const void* data)
{
    uint8_t* mappedData = nullptr;
    uint8_t* storeData = (uint8_t*) data;
    vkMapMemory(device, memory, offset, size, 0, (void**) &mappedData);

    for(uint32_t i = 0; i < size; ++i)
    {
        mappedData[i] = storeData[i];
    }

	VkMappedMemoryRange mappedMemoryRange = {};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.pNext = nullptr;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = offset;
	mappedMemoryRange.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, memory);
}

void VulkanBuffer::cmdSubData(const CommandBuffer& commandBuffer, uint32_t offset, uint32_t size, const void* data)
{
	vkCmdUpdateBuffer(commandBuffer.commandBuffer, buffer, offset, size, data);
}

uint32_t VulkanBuffer::getSize() const
{
    return size;
}
