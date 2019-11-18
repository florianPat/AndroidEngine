#include "VulkanRenderTexture.h"

VulkanRenderTexture::VulkanRenderTexture(VulkanRenderTexture&& other) noexcept
	: framebuffer(std::exchange(other.framebuffer, (VkFramebuffer)VK_NULL_HANDLE)),
    texture(std::move(other.texture)),
	renderPassBeginInfo(std::move(other.renderPassBeginInfo)),
	commandBuffer(std::move(other.commandBuffer)),
	fence(std::move(other.fence)),
	clearValue(std::move(other.clearValue)),
	orhtoProj(std::move(other.orhtoProj)),
	gfx(std::exchange(other.gfx, nullptr)),
	renderPass(std::exchange(other.renderPass, (VkRenderPass)VK_NULL_HANDLE)),
	pipeline(std::exchange(other.pipeline, (VkPipeline)VK_NULL_HANDLE))
{
	renderPassBeginInfo.pClearValues = &clearValue;
}

VulkanRenderTexture& VulkanRenderTexture::operator=(VulkanRenderTexture&& rhs) noexcept
{
    this->~VulkanRenderTexture();

    new (this) VulkanRenderTexture(std::move(rhs));

    return *this;
}

VulkanRenderTexture::~VulkanRenderTexture()
{
	texture.~VulkanTexture();
	fence.~VulkanFence();
	
	vkDestroyRenderPass(gfx->device, renderPass, nullptr);
	renderPass = VK_NULL_HANDLE;

	vkDestroyPipeline(gfx->device, pipeline, nullptr);
	pipeline = VK_NULL_HANDLE;

	if (framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(gfx->device, framebuffer, nullptr);
		framebuffer = VK_NULL_HANDLE;
	}

    texture.~VulkanTexture();
}

const VulkanTexture& VulkanRenderTexture::getTexture() const
{
    return texture;
}

VulkanRenderTexture::operator bool() const
{
    return (framebuffer != VK_NULL_HANDLE);
}

void VulkanRenderTexture::changeToShaderReadMode()
{
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
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.image = texture.getTexture();
	texture.changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		true);
}

void VulkanRenderTexture::createFramebufferTextureAndView(uint32_t width, uint32_t height)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.format = FORMAT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent = VkExtent3D{ width, height, 1 };
	new (&texture) VulkanTexture(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

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
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.image = texture.getTexture();
	texture.changeVkImageLayout(imageMemoryBarrier, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		true);
}

void VulkanRenderTexture::create(uint32_t width, uint32_t height, GraphicsVK* gfxIn)
{
	assert(!(*this));

	gfx = gfxIn;

	createFramebufferTextureAndView(width, height);

	VkImageView textureView = texture.getTextureView();

	renderPass = gfx->createRenderPass(FORMAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	pipeline = gfx->createPipeline(gfx->shader, gfx->descriptorSetLayout, renderPass);

    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;
    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = width;
    framebufferCreateInfo.height = height;
    framebufferCreateInfo.flags = 0;
    framebufferCreateInfo.layers = 1;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.pAttachments = &textureView;

    vkCreateFramebuffer(gfx->device, &framebufferCreateInfo, nullptr, &framebuffer);

	new (&commandBuffer) CommandBuffer(gfx->device, gfx->commandPool);
	new (&fence) VulkanFence(gfx->device);

	for (uint32_t i = 0; i < 4; ++i)
	{
		clearValue.color.uint32[i] = 0;
	}

	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;
	renderPassBeginInfo.renderArea.offset = VkOffset2D{0, 0};
	renderPassBeginInfo.renderArea.extent = VkExtent2D{width, height};
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = framebuffer;

    orhtoProj = Mat4x4::orthoProjVK(0.0f, 0.0f, (float)width, (float)height);
}

void VulkanRenderTexture::begin()
{
	assert(*this);

	VkExtent2D extent = { texture.getWidth(), texture.getHeight() };
	gfx->bindOtherCommandBuffer(&commandBuffer, &fence, &renderPassBeginInfo, &pipeline, extent);
	gfx->bindOtherOrthoProj(orhtoProj);
}

void VulkanRenderTexture::end()
{
	assert(*this);

	gfx->unbinOtherCommandBuffer();
	gfx->unbindOtherOrthoProj();
}
