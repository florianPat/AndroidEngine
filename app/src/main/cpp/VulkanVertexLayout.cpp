#include "VulkanVertexLayout.h"

void VulkanVertexLayout::addAttribute(uint32_t size, Type type)
{
    VkFormat format = getFormatFor(size, type);

	VkVertexInputAttributeDescription vertexInputAttributeDescription = {};
	vertexInputAttributeDescription.location = attributes.size();
	vertexInputAttributeDescription.binding = 0;
	vertexInputAttributeDescription.format = format;
	vertexInputAttributeDescription.offset = stride;
    attributes.push_back(std::move(vertexInputAttributeDescription));

    stride += size * getSizeof(type);
}

void VulkanVertexLayout::set(VkVertexInputRate inputRate)
{
	binding.binding = 0;
	binding.stride = stride;
	binding.inputRate = inputRate;
}

VkFormat VulkanVertexLayout::getFormatFor(uint32_t size, Type type)
{
    switch(type)
    {
        case Type::FLOAT:
        {
            switch(size)
            {
                case 4:
                {
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                }
                case 2:
                {
                    return VK_FORMAT_R32G32_SFLOAT;
                }
                default:
                {
                    InvalidCodePath;
                    break;
                }
            }
            break;
        }
        default:
        {
            InvalidCodePath;
            break;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

uint32_t VulkanVertexLayout::getSizeof(VulkanVertexLayout::Type type)
{
    switch(type)
    {
        case Type::FLOAT:
        {
            return sizeof(float);
        }
        default:
        {
            InvalidCodePath;
            break;
        }
    }

    return 0;
}
