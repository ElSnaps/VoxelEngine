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

void FRenderer::Initialize()
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
	SDL_Vulkan_CreateSurface( // @TODO: This might exist inside Vk SDL ext.
		FApp::Get()->Window,
		NewInstance, 
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

	SetupSwapchain();
}

// @TODO: Validation layer doesn't work.
// if stuff shuts down in the wrong order aka vkDestroyInstance first
// then we won't see what went wrong, we just get a dirty callstack.
void FRenderer::Shutdown()
{
	vkDestroySwapchainKHR(VulkanCurrentDevice, VulkanSwapchain, nullptr);

	// Destroy swapchain resources.
	for(int32 Index = 0; Index < VulkanSwapchainImageViews.size(); Index++)
	{
		vkDestroyImageView(
			VulkanCurrentDevice, 
		    VulkanSwapchainImageViews[Index], 
		    nullptr
		);
	}

	vkDestroyDevice(VulkanCurrentDevice, nullptr);
	vkDestroySurfaceKHR(VulkanInstance, VulkanWindowSurface, nullptr);
	vkb::destroy_debug_utils_messenger(VulkanInstance, VulkanDebugMessenger);
	vkDestroyInstance(VulkanInstance, nullptr);
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

void FRenderer::Draw()
{
	//SDL_Log("Draw Frame");
}