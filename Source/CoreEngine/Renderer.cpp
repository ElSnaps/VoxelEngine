// Copyright Snaps 2022. All Rights Reserved.

#include "Renderer.h"
#include "Application.h"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "VkBootstrap.h" // Bootstrap simplifies vk init.

#define VK_CHECK(x)                                                 \
do                                                              	\
{                                                               	\
	VkResult err = x;                                           	\
	if (err)                                                    	\
	{                                                           	\
		SDL_Log("Detected Vulkan error: ", err); 					\
		abort();                                                	\
	}                                                           	\
} while (0)

FRenderer::FRenderer()
{
	VulkanFrameNumber = 0;
}

void FRenderer::Initialize()
{
	// Initialize Vulkan.
	SetupVulkan();
	SetupSwapchain();
	SetupCommands();
	SetupRenderPass();
	SetupFrameBuffers();
	SetupSyncStructures();

	// Flag that we are ready to render.
	bHasInitialized = true;
}

// @TODO: Validation layer doesn't work.
// if stuff shuts down in the wrong order aka vkDestroyInstance first
// then we won't see what went wrong, we just get a dirty callstack.
void FRenderer::Shutdown()
{
	// Make sure the GPU has stopped doing it's tasks.
	vkDeviceWaitIdle(VulkanCurrentDevice);

	vkDestroyCommandPool(VulkanCurrentDevice, VulkanCommandPool, nullptr);

	// Destroy sync objects
	vkDestroyFence(VulkanCurrentDevice, VulkanRenderFence, nullptr);
	vkDestroySemaphore(VulkanCurrentDevice, VulkanRenderSemaphore, nullptr);
	vkDestroySemaphore(VulkanCurrentDevice, VulkanPresentSemaphore, nullptr);

	vkDestroySwapchainKHR(VulkanCurrentDevice, VulkanSwapchain, nullptr);

	vkDestroyRenderPass(VulkanCurrentDevice, VulkanRenderPass, nullptr);

	// Destroy swapchain resources.
	for(int32 Index = 0; Index < VulkanFrameBuffers.size(); Index++)
	{
		vkDestroyFramebuffer(
			VulkanCurrentDevice,
			VulkanFrameBuffers[Index],
			nullptr
		);

		vkDestroyImageView(
			VulkanCurrentDevice,
			VulkanSwapchainImageViews[Index],
			nullptr
		);
	}

	vkDestroySurfaceKHR(VulkanInstance, VulkanWindowSurface, nullptr);

	vkDestroyDevice(VulkanCurrentDevice, nullptr);
	vkb::destroy_debug_utils_messenger(VulkanInstance, VulkanDebugMessenger);
	vkDestroyInstance(VulkanInstance, nullptr);
}

void FRenderer::SetupVulkan()
{
	vkb::InstanceBuilder VkBuilder;

	// Create Vulkan instance with bootstrap library.
	vkb::Result<vkb::Instance> InstanceBuilder = VkBuilder
		.set_app_name("Voxel Engine")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance NewInstance = InstanceBuilder.value();
	VulkanInstance = NewInstance.instance;
	VulkanDebugMessenger = NewInstance.debug_messenger;

	// Get surface of our main app window
	SDL_Vulkan_CreateSurface(
	    FApp::Get()->Window,
	    VulkanInstance, 
	    &VulkanWindowSurface
	);

	// Select GPU that can write to SDL surfaces and supports Vk 1.1
	vkb::PhysicalDeviceSelector Selector { NewInstance };
	vkb::PhysicalDevice NewPhysicalDevice = Selector
		.set_minimum_version(1, 1)
		.set_surface(VulkanWindowSurface)
		.select()
		.value();

	// Create the final Vulkan Device
	vkb::DeviceBuilder DeviceBuilder { NewPhysicalDevice };
	vkb::Device NewDevice = DeviceBuilder.build().value();
	VulkanCurrentDevice = NewDevice.device;
	VulkanCurrentGPU = NewPhysicalDevice.physical_device;

	// Get the graphics queue via Vulkan bootstrap.
	VulkanGraphicsQueue = NewDevice.get_queue(vkb::QueueType::graphics).value();
	VulkanGraphicsQueueFamily = NewDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void FRenderer::SetupSwapchain()
{
	vkb::SwapchainBuilder SwapchainBuilder { 
		VulkanCurrentGPU, 
		VulkanCurrentDevice, 
		VulkanWindowSurface 
	};

	// Create swapchain with Vulkan bootstrap library.
	vkb::Swapchain NewSwapchain = SwapchainBuilder
		.use_default_format_selection()
		.set_desired_present_mode(AppSettings::VSync ? 
		    VK_PRESENT_MODE_FIFO_KHR : 		// Buffered Mode
		    VK_PRESENT_MODE_IMMEDIATE_KHR	// Immediate Display (Screen tearing)
		)
		.set_desired_extent(AppSettings::WindowWidth, AppSettings::WindowHeight)
		.build()
		.value();

	VulkanSwapchain = NewSwapchain;
	VulkanSwapchainImages = NewSwapchain.get_images().value();
	VulkanSwapchainImageViews = NewSwapchain.get_image_views().value();
	VulkanSwapchainImageFormat = NewSwapchain.image_format;
}

void FRenderer::SetupCommands()
{
	// Create a command pool for commands submitted to the graphics queue.
	VkCommandPoolCreateInfo CommandPoolInfo {};
	CommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	CommandPoolInfo.pNext = nullptr;

	// This command pool can submit our graphics commands.
	CommandPoolInfo.queueFamilyIndex = VulkanGraphicsQueueFamily;

	// Allow command pool to allow resetting of individual command buffers.
	CommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK(vkCreateCommandPool(
		VulkanCurrentDevice, 
		&CommandPoolInfo,
		nullptr,
		&VulkanCommandPool
	));

	// Allocate the default command buffer for rendering.
	VkCommandBufferAllocateInfo CommandAllocInfo {};
	CommandAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandAllocInfo.pNext = nullptr;

	// Commands will be made from our VulkanCommandPool.
	CommandAllocInfo.commandPool = VulkanCommandPool;
	CommandAllocInfo.commandBufferCount = 1;
	CommandAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CHECK(vkAllocateCommandBuffers(
		VulkanCurrentDevice,
		&CommandAllocInfo,
		&VulkanMainCommandBuffer
	));
}

void FRenderer::SetupRenderPass()
{
	// Create color attachment (desc of image we will write into with cmds.)
	VkAttachmentDescription ColorAttachment {};
	ColorAttachment.format = VulkanSwapchainImageFormat;				// Match attachment to swapchain.
	ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// 1 Sample - No MSAA.
	ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Clear when attachment loads.
	ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 			// Keep stored attachment when renderpass ends.
	ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// We don't need stencil.
	ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// We don't know or care about attachment initial layout.
	ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// After renderpass ends, image must be on layout ready for display.

	VkAttachmentReference ColorAttachmentRef {};
	ColorAttachmentRef.attachment = 0;									// Attachment index for pAttachments array in parent renderpass.
	ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Create 1 subpass which is the minimum you can do.
	VkSubpassDescription SubPass {};
	SubPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	SubPass.colorAttachmentCount = 1;
	SubPass.pColorAttachments = &ColorAttachmentRef;

	// 1 Dependency, which is from the "outside" into the subpass. And we can read or write color
	VkSubpassDependency Dependency {};
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.srcAccessMask = 0;
	Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo RenderPassInfo {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RenderPassInfo.attachmentCount = 1;					// Connect the color attachment to the info.
	RenderPassInfo.pAttachments = &ColorAttachment;
	RenderPassInfo.subpassCount = 1;					// Connect the subpass to the info.
	RenderPassInfo.pSubpasses = &SubPass;
	RenderPassInfo.dependencyCount = 1;
	RenderPassInfo.pDependencies = &Dependency;

	VK_CHECK(vkCreateRenderPass(
		VulkanCurrentDevice, 
		&RenderPassInfo, 
		nullptr, 
		&VulkanRenderPass
	));
}

void FRenderer::SetupFrameBuffers()
{
	// Create framebuffers for swapchain images. This will connect renderpass to render images.
	VkFramebufferCreateInfo FramebufferInfo {};
	FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	FramebufferInfo.pNext = nullptr;
	FramebufferInfo.renderPass = VulkanRenderPass;
	FramebufferInfo.attachmentCount = 1;
	FramebufferInfo.width = AppSettings::WindowWidth;
	FramebufferInfo.height = AppSettings::WindowHeight;
	FramebufferInfo.layers = 1;

	// Grab how many images we have in the swapchain.
	const uint32 SwapchainImageCount = VulkanSwapchainImages.size();
	VulkanFrameBuffers = std::vector<VkFramebuffer>(SwapchainImageCount);

	// Create framebuffers for each of the swapchain image views
	for(int32 Index = 0; Index < SwapchainImageCount; Index++)
	{
		FramebufferInfo.pAttachments = &VulkanSwapchainImageViews[Index];
		VK_CHECK(vkCreateFramebuffer(
			VulkanCurrentDevice,
			&FramebufferInfo,
			nullptr,
			&VulkanFrameBuffers[Index]
		));
	}
}

void FRenderer::SetupSyncStructures()
{
	// Create main render fence for CPU to wait for GPU tasks.
	VkFenceCreateInfo FenceCreateInfo {};
	FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	FenceCreateInfo.pNext = nullptr;

	// Use create signalled flag with fence, so we can wait on it before using it on a GPU command (for the first frame)
	FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VK_CHECK(vkCreateFence(
		VulkanCurrentDevice,
		&FenceCreateInfo,
		nullptr,
		&VulkanRenderFence
	));

	VkSemaphoreCreateInfo SemaphoreCreateInfo {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.pNext = nullptr;
	SemaphoreCreateInfo.flags = 0; // No flags required.

	VK_CHECK(vkCreateSemaphore(
		VulkanCurrentDevice,
		&SemaphoreCreateInfo,
		nullptr,
		&VulkanPresentSemaphore
	));

	VK_CHECK(vkCreateSemaphore(
		VulkanCurrentDevice,
		&SemaphoreCreateInfo,
		nullptr,
		&VulkanRenderSemaphore
	));
}

void FRenderer::Draw()
{
	uint32 SwapchainImageIndex;

	if(!bHasInitialized)
	{
		return;
	}

	if(SDL_GetWindowFlags(FApp::Get()->Window) & SDL_WINDOW_MINIMIZED)
	{
		return;
	}

	// Wait until GPU has finished rendering last frame. Timeout of 1 sec.
	VK_CHECK(vkWaitForFences(
		VulkanCurrentDevice, 
		1, 
		&VulkanRenderFence, 
		true, 
		VK_TIME_SECOND
	));

	VK_CHECK(vkResetFences(
		VulkanCurrentDevice,
		1,
		&VulkanRenderFence
	));

	// Now we are sure commands finished exec, it's safe to reset command buffer and begin recording.
	VK_CHECK(vkResetCommandBuffer(VulkanMainCommandBuffer, 0));

	// Request image from the swapchain, Timeout of 1 sec.
	VK_CHECK(vkAcquireNextImageKHR(
		VulkanCurrentDevice,
		VulkanSwapchain,
		VK_TIME_SECOND,
		VulkanPresentSemaphore,
		nullptr,
		&SwapchainImageIndex
	));

	// Victor does this to shorten the name in code, but I don't likez it.
	// Not entirely sure if the copy is required here, don't think so.
	// but just incase it is, this is probably where bugs source could be.
	// delete can can confirm it's not an issue.
	// VkCommandBuffer Command = VulkanMainCommandBuffer;

	VkCommandBufferBeginInfo CommandBeginInfo {};
	CommandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBeginInfo.pNext = nullptr;
	CommandBeginInfo.pInheritanceInfo = nullptr;
	CommandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(VulkanMainCommandBuffer, &CommandBeginInfo));

//////////////////////////////////////////////////////////////////////////
// BEGIN TEMP RENDER CODE TEST.
//////////////////////////////////////////////////////////////////////////

	VkExtent2D RenderExtent;
	RenderExtent.width = AppSettings::WindowWidth;
	RenderExtent.height = AppSettings::WindowHeight;

	// Make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue ClearValue;
	float Flash = abs(sin(VulkanFrameNumber / 120.f));
	ClearValue.color = {{0.f, 0.f, Flash, 1.f}};

	// Start the main renderpass.
	// We will use the clear color from above, and the framebuffer of the index the swapchain gave us.
	VkRenderPassBeginInfo RenderPassInfo {};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	RenderPassInfo.pNext = nullptr;
	RenderPassInfo.renderPass = VulkanRenderPass;
	RenderPassInfo.renderArea.offset.x = 0;
	RenderPassInfo.renderArea.offset.y = 0;
	RenderPassInfo.renderArea.extent = RenderExtent;
	RenderPassInfo.framebuffer = VulkanFrameBuffers[SwapchainImageIndex];

	// Connect clear values
	RenderPassInfo.clearValueCount = 1;
	RenderPassInfo.pClearValues = &ClearValue;

	vkCmdBeginRenderPass(VulkanMainCommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// @TODO: Once we start adding render cmds, they will go here.

	// Finalize the render pass
	vkCmdEndRenderPass(VulkanMainCommandBuffer);

	// Finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(VulkanMainCommandBuffer));

	// Prepare the submission to the queue.
	// We want to wait on the PresentSemaphore, as that semaphore is signaled when the swapchain is ready.
	// we will signal the RenderSemaphore, to signal that rendering has finished.
	VkSubmitInfo SubmitInfo {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pNext = nullptr;

	VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubmitInfo.pWaitDstStageMask = &WaitStage;
	SubmitInfo.waitSemaphoreCount = 1;
	SubmitInfo.pWaitSemaphores = &VulkanPresentSemaphore;
	SubmitInfo.signalSemaphoreCount = 1;
	SubmitInfo.pSignalSemaphores = &VulkanRenderSemaphore;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &VulkanMainCommandBuffer;
	
	// Submit command buffer to the queue and execute it.
	// VulkanRenderFence will now block until the graphics commands finish execution
	VK_CHECK(vkQueueSubmit(
		VulkanGraphicsQueue,
		1, 
		&SubmitInfo, 
		VulkanRenderFence
	));

	// This will put the image we just rendered into the visible window.
	// we want to wait on the RenderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user.
	VkPresentInfoKHR PresentInfo {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = nullptr;
	PresentInfo.pSwapchains = &VulkanSwapchain;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pWaitSemaphores = &VulkanRenderSemaphore;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pImageIndices = &SwapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(VulkanGraphicsQueue, &PresentInfo));

	VulkanFrameNumber++;
}