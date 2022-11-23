// Copyright Snaps 2022. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "vulkan.h"
#include <vector>

/*
	Render is responsible for rendering the game and talking to
	FApp and SDL in order to draw to the main game window.
*/
class FRenderer
{
public:

	void Initialize();
	void Shutdown();
	void SetupSwapchain();
	void Draw();

protected:

	// @TODO: Feels like these should be smart ptrs.
	VkInstance 					VulkanInstance;
	VkDebugUtilsMessengerEXT 	VulkanDebugMessenger;
	VkPhysicalDevice 			VulkanCurrentGPU;
	VkDevice 					VulkanCurrentDevice;
	VkSurfaceKHR 				VulkanWindowSurface;

	VkSwapchainKHR				VulkanSwapchain;
	VkFormat					VulkanSwapchainImageFormat;
	std::vector<VkImage>		VulkanSwapchainImages;
	std::vector<VkImageView>	VulkanSwapchainImageViews;

};