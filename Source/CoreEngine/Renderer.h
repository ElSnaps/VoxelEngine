// Copyright Snaps 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "vulkan.h"
#include <vector>

#define VK_TIME_SECOND		1000000000

/*
	Render is responsible for rendering the game and talking to
	FApp and SDL in order to draw to the main game window.
*/
class FRenderer
{
public:

	FRenderer();

	void Initialize();
	void Shutdown();
	void Draw();

protected:

	VkInstance 					VulkanInstance;
	VkDebugUtilsMessengerEXT 	VulkanDebugMessenger;
	VkPhysicalDevice 			VulkanCurrentGPU;
	VkDevice 					VulkanCurrentDevice;
	VkSurfaceKHR 				VulkanWindowSurface;

	VkSwapchainKHR				VulkanSwapchain;
	VkFormat					VulkanSwapchainImageFormat;
	std::vector<VkImage>		VulkanSwapchainImages;
	std::vector<VkImageView>	VulkanSwapchainImageViews;

	VkQueue						VulkanGraphicsQueue;
	uint32						VulkanGraphicsQueueFamily;
	VkCommandPool				VulkanCommandPool;
	VkCommandBuffer				VulkanMainCommandBuffer;

	VkRenderPass				VulkanRenderPass;
	std::vector<VkFramebuffer>	VulkanFrameBuffers;

	VkSemaphore					VulkanPresentSemaphore;
	VkSemaphore					VulkanRenderSemaphore;
	VkFence						VulkanRenderFence;
	int32						VulkanFrameNumber;

private:

	void SetupVulkan();
	void SetupSwapchain();
	void SetupCommands();
	void SetupRenderPass();
	void SetupFrameBuffers();
	void SetupSyncStructures();

	bool bHasInitialized = false;
};