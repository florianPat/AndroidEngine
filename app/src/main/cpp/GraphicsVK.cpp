#include "GraphicsVK.h"
#include "VulkanRenderTexture.h"
#include "Sprite.h"

GraphicsVK::GraphicsVK(uint32_t renderWidth, uint32_t renderHeight, View::ViewportType viewportType)
        : GraphicsVKIniter(renderWidth, renderHeight, viewportType), vertices(makeUnique<Vertex[]>(NUM_VERTICES_TO_BATCH))
{
    Vector2f spriteVertices[] = { {0.0f, 1.0f},
                                  {1.0f, 1.0f},
                                  {1.0f, 0.0f},
                                  {0.0f, 0.0f} };
    for(int i = 0; i < NUM_VERTICES_TO_BATCH; i += 4)
    {
        vertices[i + 0].position = spriteVertices[0];
        vertices[i + 1].position = spriteVertices[1];
        vertices[i + 2].position = spriteVertices[2];
        vertices[i + 3].position = spriteVertices[3];
    }
}

void GraphicsVK::clear()
{
	currentCommandBuffer->begin();
	currentCommandBuffer->beginRenderPass(currentRenderPassBeginInfo);

	vkCmdClearAttachments(currentCommandBuffer->commandBuffer, 1, &clearAttachment, 1, &clearRect);

	currentCommandBuffer->endRenderPass();
	currentCommandBuffer->end();

	currentCommandBuffer->submit(queue, VK_NULL_HANDLE, firstTimePresentComplete, currentFence->fence);
	firstTimePresentComplete = VK_NULL_HANDLE;

	currentFence->wait();
	currentFence->reset();
}

void GraphicsVK::render()
{
	flush();

    presentInfo.pImageIndices = &currentSwapchainImage;

    vkQueuePresentKHR(queue, &presentInfo);

    if (view.updated())
    {
        orthoProj = view.getOrthoProj();
        uniformBuffer.subData(0, sizeof(Mat4x4), &orthoProj);
    }

	VkResult result = vkAcquireNextImageKHR(device, swapchain, INT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE,
		&currentSwapchainImage);
	assert(result == VK_SUCCESS);

	currentCommandBuffer = &commandBuffers[currentSwapchainImage];
	currentFence = &fence;
	currentPipeline = &graphicsPipeline;
	currentRenderPassBeginInfo = &renderPassBeginInfo;
	currentRenderPassBeginInfo->framebuffer = framebuffers[currentSwapchainImage];
	firstTimePresentComplete = presentCompleteSemaphore;
}

void GraphicsVK::flush()
{
	if (nSpritesBatched != 0)
	{
		vertexBuffer.subData(0, sizeof(Vertex) * nVerticesBatched(), vertices.get());

		currentCommandBuffer->begin();
		currentCommandBuffer->beginRenderPass(currentRenderPassBeginInfo);

		vkCmdBindPipeline(currentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
		vertexBuffer.bindVertexBuffer(currentCommandBuffer->commandBuffer);
		indexBuffer.bindIndexBuffer(currentCommandBuffer->commandBuffer);
		descriptorSetLayout.bind(currentCommandBuffer->commandBuffer);

		vkCmdDrawIndexed(currentCommandBuffer->commandBuffer, 6 * nSpritesBatched, 6, 0, 0, 0);

		currentCommandBuffer->endRenderPass();
		currentCommandBuffer->end();

		currentCommandBuffer->submit(queue, VK_NULL_HANDLE, VK_NULL_HANDLE, currentFence->fence);

		nSpritesBatched = 0;

		//NOTE: Need it here because changing state after this...
		currentFence->wait();
		currentFence->reset();
	}
}

void GraphicsVK::setupGfxGpu()
{
	view = View(renderWidth, renderHeight, screenWidth, screenHeight, viewportType);
    orthoProj = view.getOrthoProj();

    new (&shader) VulkanShader("shaders/RectSprite", device);

	shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(4, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.addAttribute(2, VulkanVertexLayout::Type::FLOAT);
	shader.vertexLayout.set(VK_VERTEX_INPUT_RATE_VERTEX);

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {};
	descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings[0].descriptorCount = 1;
	descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBindings[0].binding = 0;
	descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetLayoutBindings[1].descriptorCount = 1;
	descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;
	descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBindings[1].binding = 1;
	new (&descriptorSetLayout) VulkanDescriptorSetLayout(descriptorSetLayoutBindings, arrayCount(descriptorSetLayoutBindings), device);

    new (&vertexBuffer) VulkanBuffer(nullptr, sizeof(Vertex) * NUM_VERTICES_TO_BATCH, device, graphicsQueueFamilyIndex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, this);

    uint16_t indices[6 * NUM_SPRITES_TO_BATCH];
    for(uint16_t i = 0, counter = 0; i < NUM_SPRITES_TO_BATCH; ++i, counter += 6)
    {
        indices[counter + 0] = 0 + i * 4;
        indices[counter + 1] = 2 + i * 4;
        indices[counter + 2] = 3 + i * 4;
        indices[counter + 3] = 0 + i * 4;
        indices[counter + 4] = 1 + i * 4;
        indices[counter + 5] = 2 + i * 4;
    }
    new (&indexBuffer) VulkanBuffer(indices, arrayCount(indices), device, graphicsQueueFamilyIndex,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT, this);
    new (&uniformBuffer) VulkanBuffer(&orthoProj, sizeof(orthoProj), device, graphicsQueueFamilyIndex,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, this);

    graphicsPipeline = createPipeline(shader, descriptorSetLayout, renderPass);

    setClearAndPresentStructs();

	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = uniformBuffer.getSize();
	descriptorSetLayout.updateUniformBuffer(0, &descriptorBufferInfo);

	new (&fence) VulkanFence(device);

	for (uint32_t i = 0; i < NUM_BUFFERS; ++i)
	{
		new (&commandBuffers[i]) CommandBuffer(device, commandPool);
	}

	sampler = createSampler();

	VkResult result = vkAcquireNextImageKHR(device, swapchain, INT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE,
		&currentSwapchainImage);
	assert(result == VK_SUCCESS);
	currentCommandBuffer = &commandBuffers[currentSwapchainImage];
	currentFence = &fence;
	currentPipeline = &graphicsPipeline;
	currentRenderPassBeginInfo = &renderPassBeginInfo;
	currentRenderPassBeginInfo->framebuffer = framebuffers[currentSwapchainImage];
	clearRect.rect.extent = swapchainExtent;
	firstTimePresentComplete = presentCompleteSemaphore;

	VulkanTexture::setStaticParameters(this);

    uint8_t buffer[] = { 255 };
    new (&blackTexture) VulkanTexture(buffer, 1, 1);
}

void GraphicsVK::freeGfxGpu()
{
	vkDeviceWaitIdle(device);

	shader.~VulkanShader();
	vertexBuffer.~VulkanBuffer();
	indexBuffer.~VulkanBuffer();
	uniformBuffer.~VulkanBuffer();
	blackTexture.~VulkanTexture();
	descriptorSetLayout.~VulkanDescriptorSetLayout();
	fence.~VulkanFence();

    if(sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }

    if(graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        graphicsPipeline = VK_NULL_HANDLE;
    }
}

void GraphicsVK::draw(const CircleShape& circle)
{
    //TODO: Implement!
}

void GraphicsVK::bindOtherOrthoProj(const Mat4x4& otherOrthoProj)
{
    uniformBuffer.subData(0, sizeof(Mat4x4), &otherOrthoProj);
}

void GraphicsVK::unbindOtherOrthoProj()
{
    uniformBuffer.subData(0, sizeof(Mat4x4), &orthoProj);
}

void GraphicsVK::bindOtherCommandBuffer(CommandBuffer* commandBufferIn, VulkanFence* fenceIn, VkRenderPassBeginInfo* renderPassBeginInfoIn,
	VkPipeline* pipelineIn, const VkExtent2D& extentIn)
{
	flush();
	currentCommandBuffer = commandBufferIn;
	currentFence = fenceIn;
	currentRenderPassBeginInfo = renderPassBeginInfoIn;
	currentPipeline = pipelineIn;
	clearRect.rect.extent = extentIn;
}

void GraphicsVK::unbinOtherCommandBuffer()
{
	flush();
	currentCommandBuffer = &commandBuffers[currentSwapchainImage];
	currentFence = &fence;
	currentRenderPassBeginInfo = &renderPassBeginInfo;
	currentPipeline = &graphicsPipeline;
	clearRect.rect.extent = swapchainExtent;
}

void GraphicsVK::draw(const Sprite& sprite)
{
    assert(vertexBuffer);

    const VulkanTexture* texture = (const VulkanTexture*)sprite.getTexture();
    assert(*texture);

    if(currentBoundTexture != texture->getTextureView())
    {
        if(nSpritesBatched != 0)
            flush();

        descriptorSetLayout.updateSamplerImage(1, texture->getTextureView(), sampler);

        currentBoundTexture = texture->getTextureView();
    }
    else if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float texRectLeft = ((float)sprite.getTextureRect().left) / texture->getWidth();
    float texRectTop = ((float)sprite.getTextureRect().bottom) / texture->getHeight();
    float texRectRight = ((float)sprite.getTextureRect().getRight()) / texture->getWidth();
    float texRectBottom = ((float) sprite.getTextureRect().getTop()) / texture->getHeight();
    Vector2f texCoord[4] = { { texRectLeft, texRectTop },
                             { texRectRight, texRectTop },
                             { texRectRight, texRectBottom },
                             { texRectLeft, texRectBottom } };

    float red = sprite.color.r;
    float green = sprite.color.g;
    float blue = sprite.color.b;
    float alpha = sprite.color.a;

    Mat4x4 mv = sprite.getTransform();

    int32_t plusI = nVerticesBatched();

    for(int i = 0; i < 4; ++i)
    {
        vertices[i + plusI].tex = texCoord[i];
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

void GraphicsVK::draw(const RectangleShape& rect)
{
    assert(vertexBuffer);

    if(nSpritesBatched >= NUM_SPRITES_TO_BATCH)
    {
        flush();
    }

    float red = rect.fillColor.r;
    float green = rect.fillColor.g;
    float blue = rect.fillColor.b;
    float alpha = rect.fillColor.a;

    Mat4x4 mv = rect.getTransform();

    int32_t plusI = nVerticesBatched();

    for(int i = 0; i < 4; ++i)
    {
        vertices[i + plusI].tex = { -1.0f, -1.0f };
        vertices[i + plusI].colorR = red;
        vertices[i + plusI].colorG = green;
        vertices[i + plusI].colorB = blue;
        vertices[i + plusI].colorA = alpha;
        vertices[i + plusI].mvMatrix[0] = mv.matrix[0];
        vertices[i + plusI].mvMatrix[1] = mv.matrix[1];
        vertices[i + plusI].mvMatrix[2] = mv.matrix[4];
        vertices[i + plusI].mvMatrix[3] = mv.matrix[5];
        vertices[i + plusI].mvMatrix[4] = mv.matrix[12];
        vertices[i + plusI].mvMatrix[5] = mv.matrix[13];
    }

    ++nSpritesBatched;
}

int32_t GraphicsVK::nVerticesBatched() const
{
    return (nSpritesBatched * 4);
}

void GraphicsVK::setClearAndPresentStructs()
{
	for (uint32_t i = 0; i < 4; ++i)
	{
		clearValue.color.float32[i] = 0.0f;
	}

    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = nullptr;
    renderPassBeginInfo.renderArea.offset = VkOffset2D{ 0, 0 };
    renderPassBeginInfo.renderArea.extent = swapchainExtent;

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.pWaitSemaphores = nullptr;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pResults = nullptr;

	clearAttachment.colorAttachment = 0;
	clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clearAttachment.clearValue = clearValue;

	clearRect.baseArrayLayer = 0;
	clearRect.layerCount = 1;
	clearRect.rect.offset = VkOffset2D{ 0, 0 };
}
