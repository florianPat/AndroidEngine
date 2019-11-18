#include "VulkanTexture.h"
#include "Ifstream.h"
#include "CommandBuffer.h"
//NOTE: In pngrutil.c I made a note!
#include <png.h>
#include "VulkanFence.h"
#include "PNGReadCallback.h"

const GraphicsVKIniter* VulkanTexture::gfx = nullptr;

VulkanTexture::operator bool() const
{
    return (texture != VK_NULL_HANDLE);
}

VulkanTexture::~VulkanTexture()
{
	fence.~VulkanFence();

    if(textureView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gfx->device, textureView, nullptr);
        textureView = VK_NULL_HANDLE;
    }
    if(texture != VK_NULL_HANDLE)
    {
        vkDestroyImage(gfx->device, texture, nullptr);
        texture = VK_NULL_HANDLE;
    }
	if (memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(gfx->device, memory, nullptr);
		memory = VK_NULL_HANDLE;
	}
}

VulkanTexture::VulkanTexture(VulkanTexture&& other) noexcept : width(std::exchange(other.width, 0)),
	height(std::exchange(other.height, 0)), texture(std::exchange(other.texture, (VkImage)VK_NULL_HANDLE)),
	textureView(std::exchange(other.textureView, (VkImageView)VK_NULL_HANDLE)),
    memory(std::exchange(other.memory, (VkDeviceMemory)VK_NULL_HANDLE)),
	commandBuffer(std::move(other.commandBuffer)),
	fence(std::move(other.fence))
{
}

VulkanTexture& VulkanTexture::operator=(VulkanTexture&& rhs) noexcept
{
    this->~VulkanTexture();

    new (this) VulkanTexture(std::move(rhs));

    return *this;
}

VulkanTexture::VulkanTexture(const String& filename)
{
    Ifstream asset;
    asset.open(filename);

    if (!asset)
        InvalidCodePath;

    png_byte header[8];
    png_structp png = nullptr;
    png_infop info = nullptr;
    bool transparency;

    asset.read(header, sizeof(header));
    if (png_sig_cmp(header, 0, 8) != 0)
        InvalidCodePath;

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
        InvalidCodePath;
    info = png_create_info_struct(png);
    if (!info)
    {
        png_destroy_read_struct(&png, nullptr, nullptr);
        InvalidCodePath;
    }

    png_set_read_fn(png, &asset, callbackReadPng);
    png_set_read_user_transform_fn(png, callbackReadTransform);

    png_set_sig_bytes(png, 8);
    png_read_info(png, info);
    png_int_32 depth, colorType;
    png_uint_32 width, height;
    png_get_IHDR(png, info, &width, &height, &depth, &colorType, nullptr, nullptr, nullptr);
	this->width = width;
	this->height = height;

    transparency = false;
    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png);
        transparency = true;
    }

    if (depth < 8)
    {
        png_set_packing(png);
    }
    else if (depth == 16)
    {
        png_set_strip_16(png);
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    switch (colorType)
    {
        case PNG_COLOR_TYPE_PALETTE:
        {
            png_set_palette_to_rgb(png);
            format = transparency ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM;
            break;
        }
        case PNG_COLOR_TYPE_RGB:
        {
            format = transparency ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM;
            break;
        }
        case PNG_COLOR_TYPE_RGBA:
        {
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        case PNG_COLOR_TYPE_GRAY:
        {
            png_set_expand_gray_1_2_4_to_8(png);
            format = transparency ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8_UNORM;
            break;
        }
        case PNG_COLOR_TYPE_GA:
        {
            png_set_expand_gray_1_2_4_to_8(png);
            format = VK_FORMAT_R8G8_UNORM;
            break;
        }
        default:
        {
            png_destroy_read_struct(&png, &info, nullptr);
            InvalidCodePath;
            break;
        }
    }
    png_read_update_info(png, info);

    png_size_t rowSize = png_get_rowbytes(png, info);
	assert(rowSize > 0 && rowSize >= (width * 4));

	png_byte* image = (png_byte*) malloc(rowSize * height * sizeof(png_byte));
    png_bytep* rowPtrs = (png_bytep*) malloc(height * sizeof(png_bytep));

    for (uint32_t i = 0; i < height; ++i)
    {
        rowPtrs[i] = &image[i * rowSize];
    }

    png_read_image(png, rowPtrs);

    uint8_t* mappedData = startCreateGpuTexture(gfx->graphicsQueueFamilyIndex, format);
    uint32_t gpuRowSize = getRowPitch();
	assert(gpuRowSize >= rowSize);

    for(uint32_t y = 0; y < height; ++y)
    {
        for(uint32_t x = 0; x < rowSize; ++x)
        {
			mappedData[y * gpuRowSize + x] = image[y * rowSize + x];
        }
    }

    finishCreateGpuTexture(gfx->graphicsQueueFamilyIndex, commandBuffer, gfx->queue);
	free(rowPtrs);
	free(image);
	png_destroy_read_struct(&png, &info, nullptr);
    asset.close();
}

VulkanTexture::VulkanTexture(const void* buffer, uint32_t width, uint32_t height, VkFormat internalFormat)
        : width(width), height(height)
{
    uint8_t* mappedData = startCreateGpuTexture(gfx->graphicsQueueFamilyIndex, internalFormat);
	if (buffer != nullptr)
	{
		const uint8_t* storeData = (const uint8_t*)buffer;
		uint32_t rowSize = getRowPitch();

		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				mappedData[y * rowSize + x] = storeData[y * height + x];
			}
		}
	}

    finishCreateGpuTexture(gfx->graphicsQueueFamilyIndex, commandBuffer, gfx->queue);
}

VulkanTexture::VulkanTexture(VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags,
	bool setDefaultGraphicsFamilyIndex) : width(imageCreateInfo.extent.width), height(imageCreateInfo.extent.height)
{
	if (setDefaultGraphicsFamilyIndex)
	{
		imageCreateInfo.queueFamilyIndexCount = 1;
		imageCreateInfo.pQueueFamilyIndices = &gfx->graphicsQueueFamilyIndex;
	}

	createVkImageAndView(imageCreateInfo, memoryPropertyFlags);
}

VkDeviceSize VulkanTexture::createVkImageAndView(const VkImageCreateInfo& imageCreateInfo, VkMemoryPropertyFlagBits memoryPropertyFlags)
{
	new (&commandBuffer) CommandBuffer(gfx->device, gfx->commandPool);
	new (&fence) VulkanFence(gfx->device);

	vkCreateImage(gfx->device, &imageCreateInfo, nullptr, &texture);

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(gfx->device, texture, &memoryRequirements);
	uint32_t memoryTypeIndex = gfx->getMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	vkAllocateMemory(gfx->device, &memoryAllocateInfo, nullptr, &memory);
	assert(((uint64_t)width * height) <= memoryRequirements.size);

	vkBindImageMemory(gfx->device, texture, memory, 0);

	//NOTE: Is defined twice (not really cool!)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkComponentMapping componentMapping = {};
	componentMapping.r = VK_COMPONENT_SWIZZLE_R;
	componentMapping.g = VK_COMPONENT_SWIZZLE_G;
	componentMapping.b = VK_COMPONENT_SWIZZLE_B;
	componentMapping.a = VK_COMPONENT_SWIZZLE_A;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.image = texture;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.subresourceRange = subresourceRange;
	imageViewCreateInfo.components = componentMapping;

	vkCreateImageView(gfx->device, &imageViewCreateInfo, nullptr, &textureView);

	return memoryRequirements.size;
}

uint8_t* VulkanTexture::startCreateGpuTexture(uint32_t graphicsFamilyIndex, VkFormat internalFormat)
{
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    imageCreateInfo.format = internalFormat;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 1;
    imageCreateInfo.pQueueFamilyIndices = &graphicsFamilyIndex;
    imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent = VkExtent3D{ width, height, 1 };

	VkDeviceSize memoryRequirementsSize = createVkImageAndView(imageCreateInfo);

    uint8_t* mappedData = nullptr;
    vkMapMemory(gfx->device, memory, 0, memoryRequirementsSize, 0, (void**)&mappedData);

    return mappedData;
}

void VulkanTexture::finishCreateGpuTexture(uint32_t graphicsFamilyIndex, CommandBuffer& commandBuffer, VkQueue queue)
{
	VkMappedMemoryRange mappedMemoryRange = {};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedMemoryRange.pNext = nullptr;
	mappedMemoryRange.memory = memory;
	mappedMemoryRange.offset = 0;
	mappedMemoryRange.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(gfx->device, 1, &mappedMemoryRange);

    vkUnmapMemory(gfx->device, memory);

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	//To convert it to an optimized layout the gpu understands
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.image = texture;
	imageMemoryBarrier.srcQueueFamilyIndex = graphicsFamilyIndex;
	imageMemoryBarrier.dstQueueFamilyIndex = graphicsFamilyIndex;

	changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, false);
}

uint32_t VulkanTexture::getRowPitch()
{
    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    VkSubresourceLayout subresourceLayout;
    vkGetImageSubresourceLayout(gfx->device, texture, &subresource, &subresourceLayout);

    assert(height * subresourceLayout.rowPitch == subresourceLayout.size);

    return (uint32_t) subresourceLayout.rowPitch;
}

void VulkanTexture::setStaticParameters(const GraphicsVKIniter* gfxIn)
{
	gfx = gfxIn;
}

void VulkanTexture::changeVkImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage, bool setDefaultGraphicsFamilyIndex)
{
	if (setDefaultGraphicsFamilyIndex)
	{
		imageMemoryBarrier.srcQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
		imageMemoryBarrier.dstQueueFamilyIndex = gfx->graphicsQueueFamilyIndex;
	}

	commandBuffer.begin();

	vkCmdPipelineBarrier(commandBuffer.commandBuffer, srcStage, dstStage,
		0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

	commandBuffer.end();
	commandBuffer.submit(gfx->queue, fence.fence);

	fence.wait();
	fence.reset();
}

void VulkanTexture::reloadFromFile(const String& filename)
{
    this->~VulkanTexture();

    new (this) VulkanTexture(filename);
}
