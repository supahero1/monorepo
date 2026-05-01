/*
 *   Copyright 2024-2026 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <shared/str.h>
#include <shared/file.h>
#include <shared/sync.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/atomic.h>
#include <shared/extent.h>
#include <shared/threads.h>
#include <client/tex/base.h>
#include <shared/settings.h>
#include <client/window/dds.h>
#include <shared/alloc/base.h>
#include <client/window/base.h>
#include <client/window/volk.h>
#include <client/window/vulkan.h>

#include <volk.h>
#include <vulkan/vk_platform.h>

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdatomic.h>

#define VK_MAX_IMAGES 8
#define VK_MAX_EXTENSIONS 64
#define VK_COMMANDS 4
#define VK_STAGING_BUFFER_SIZE 1 * 1024 * 1024
#define VK_INSTANCE_BUFFER_SIZE 1 * 1024 * 1024
#define VK_DRAW_DATA_BUFFER_SIZE (VK_INSTANCE_BUFFER_SIZE / sizeof(vulkan_draw_data_t))
#define VK_FPS_DECAY_FACTOR 0.9f


typedef struct vk_barrier
{
	VkSemaphore semaphore;
	VkFence fence;
	VkCommandBuffer command_buffers[VK_MAX_IMAGES];
}
vk_barrier_t;

typedef struct vk_buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
}
vk_buffer_t;

typedef struct vk_frame
{
	VkImage image;
	VkImageView view;
	VkFramebuffer framebuffer;
	VkSemaphore semaphore;
	vk_barrier_t* barrier;

	vk_buffer_t instance_input_buffer;
	vk_buffer_t draw_count_buffer;
}
vk_frame_t;

typedef enum vk_image_type
{
	VK_IMAGE_TYPE_DEPTH_STENCIL,
	VK_IMAGE_TYPE_MULTISAMPLED,
	VK_IMAGE_TYPE_TEXTURE,
	MACRO_ENUM_BITS(VK_IMAGE_TYPE)
}
vk_image_type_t;

typedef struct vk_image
{
	const char* path;

	uint32_t width;
	uint32_t height;
	uint32_t layers;

	VkFormat format;
	vk_image_type_t type;

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;

	VkImageAspectFlags aspect;
	VkImageUsageFlags usage;
	VkSampleCountFlagBits samples;
}
vk_image_t;

typedef struct vk_vertex_input
{
	pair_t pos;
	pair_t tex_coord;
}
vk_vertex_input_t;

typedef struct vk_vertex_consts
{
	pair_t window_size;
}
vk_vertex_consts_t;

typedef struct vk_command
{
	VkCommandBuffer buffer;
	VkFence fence;
	bool waited;
	bool using_temp;

	vk_buffer_t staging_buffer;
	vk_buffer_t temp_staging_buffer;
}
vk_command_t;

struct vulkan
{
	time_timers_t timers;

	window_t window;
	event_listener_t* resize_listener;

	vulkan_event_table_t event_table;

	settings_t settings;

	setting_t* buffering_setting;
	setting_t* max_msaa_samples_setting;
	setting_t* sample_shading_setting;
	setting_t* min_sample_shading_setting;

	uint32_t buffering;
	uint32_t max_msaa_samples;

	struct VolkDeviceTable table;

	VkDebugUtilsMessengerEXT messenger;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkSurfaceCapabilitiesKHR surface_capabilities;

	uint32_t queue_id;
	VkFormat format;
	VkSampleCountFlagBits samples;
	VkPhysicalDeviceLimits limits;

	VkPhysicalDevice physical_device;
	VkDevice device;
	VkQueue queue;
	VkPhysicalDeviceMemoryProperties memory_properties;

	VkCommandPool command_pool;
	vk_command_t commands[VK_COMMANDS];
	vk_command_t* command;

	VkExtent2D extent;
	uint32_t image_count;
	VkSurfaceTransformFlagBitsKHR transform;
	VkPresentModeKHR present_mode;

	vk_frame_t frames[VK_MAX_IMAGES];
	vk_barrier_t* barriers;
	vk_barrier_t* barrier;

	VkSwapchainKHR swapchain;

	VkViewport viewport;
	VkRect2D scissor;

	VkSampler sampler;

	vk_image_t depth_image;
	vk_image_t multisampling_image;

	vk_buffer_t vertex_input_buffer;
	vulkan_draw_data_t* draw_data;
	uint32_t draw_data_count;

	vk_vertex_consts_t consts;

	VkRenderPass render_pass;
	VkDescriptorSetLayout set_layout;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipeline;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;
	vk_image_t textures[TEX__COUNT];

	thread_t thread;
	bool _Atomic should_run;

	sync_mtx_t draw_mtx;
	sync_mtx_t resize_mtx;
	sync_cond_t resize_cond;
	bool _Atomic resized;

	float delta;
	float fps;
};


const char* vk_instance_extensions[] =
{
#ifndef NDEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

const char* vk_instance_layers[] =
{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};

const char* vk_device_extensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const char* vk_device_layers[] =
{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation",
#endif
};


const vk_vertex_input_t vk_vertex_input[] =
{
	{ {{ -0.5f, -0.5f }}, {{ 0.0f, 0.0f }} },
	{ {{  0.5f, -0.5f }}, {{ 1.0f, 0.0f }} },
	{ {{ -0.5f,  0.5f }}, {{ 0.0f, 1.0f }} },
	{ {{  0.5f,  0.5f }}, {{ 1.0f, 1.0f }} },
};


#ifndef NDEBUG

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* data,
	void* user_data
	)
{
	fputs(data->pMessage, stderr);
	return VK_FALSE;
}

#endif


void
vk_init_settings(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->settings = settings_init("settings/vulkan.bin", vk->timers);

	vk->buffering_setting = settings_add_i64(vk->settings, "buffering", 2, 2, 3, NULL);
	vk->max_msaa_samples_setting = settings_add_i64(vk->settings, "max_msaa_samples", 8, 1, 64, NULL);

	settings_seal(vk->settings);
	settings_load(vk->settings);

	vk->buffering = setting_get_i64(vk->buffering_setting);
	vk->max_msaa_samples = setting_get_i64(vk->max_msaa_samples_setting);
}


void
vk_free_settings(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	settings_free(vk->settings);
}


const char**
vk_get_instance_extensions(
	vulkan_t vk,
	const char** extension
	)
{
	assert_not_null(vk);
	assert_not_null(extension);

	uint32_t available_instance_extension_count = 0;
	VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, NULL);
	hard_assert_eq(result, VK_SUCCESS);

	VkExtensionProperties available_instance_extensions[available_instance_extension_count];

	VkExtensionProperties* available_instance_extension = available_instance_extensions;
	VkExtensionProperties* available_instance_extension_end =
		available_instance_extension + available_instance_extension_count;

	while(available_instance_extension < available_instance_extension_end)
	{
		*(available_instance_extension++) = (VkExtensionProperties){0};
	}

	result = vkEnumerateInstanceExtensionProperties(NULL,
		&available_instance_extension_count, available_instance_extensions);
	hard_assert_eq(result, VK_SUCCESS);

	puts("\nVK available instance extensions:");

	for(
		available_instance_extension = available_instance_extensions;
		available_instance_extension < available_instance_extension_end;
		available_instance_extension++
		)
	{
		printf("- %s\n", available_instance_extension->extensionName);
	}

	puts("");

	const char* const* instance_extension = vk_instance_extensions;
	const char* const* instance_extension_end = instance_extension + MACRO_ARRAY_LEN(vk_instance_extensions);

	while(instance_extension < instance_extension_end)
	{
		bool found = false;
		const char* extension_name = *(instance_extension++);

		available_instance_extension = available_instance_extensions;
		while(available_instance_extension < available_instance_extension_end)
		{
			if(strcmp(extension_name, available_instance_extension->extensionName) == 0)
			{
				found = true;
				break;
			}

			available_instance_extension++;
		}

		hard_assert_true(found, fprintf(stderr, "VK instance extension %s not found\n", extension_name));
		printf("+ %s\n", extension_name);
		*(extension++) = cstr_init(extension_name);
	}

	uint32_t sdl_instance_extension_count = 0;
	const char* const* sdl_instance_extensions = window_get_vulkan_extensions(&sdl_instance_extension_count);
	hard_assert_ptr(sdl_instance_extensions, sdl_instance_extension_count);

	const char* const* sdl_instance_extension = sdl_instance_extensions;
	const char* const* sdl_instance_extension_end = sdl_instance_extension + sdl_instance_extension_count;

	while(sdl_instance_extension < sdl_instance_extension_end)
	{
		bool found = false;
		const char* extension_name = *(sdl_instance_extension++);

		available_instance_extension = available_instance_extensions;
		while(available_instance_extension < available_instance_extension_end)
		{
			if(strcmp(extension_name, available_instance_extension->extensionName) == 0)
			{
				found = true;
				break;
			}

			available_instance_extension++;
		}

		hard_assert_true(found, fprintf(stderr, "SDL VK instance extension %s not found\n", extension_name));
		printf("+ %s\n", extension_name);
		*(extension++) = cstr_init(extension_name);
	}

	return extension;
}


const char**
vk_get_instance_layers(
	vulkan_t vk,
	const char** layer
	)
{
	assert_not_null(vk);
	assert_not_null(layer);

	uint32_t available_instance_layer_count = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, NULL);
	hard_assert_eq(result, VK_SUCCESS);

	VkLayerProperties available_instance_layers[available_instance_layer_count];

	VkLayerProperties* available_instance_layer = available_instance_layers;
	VkLayerProperties* available_instance_layer_end = available_instance_layer + available_instance_layer_count;

	while(available_instance_layer < available_instance_layer_end)
	{
		*(available_instance_layer++) = (VkLayerProperties){0};
	}

	result = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, available_instance_layers);
	hard_assert_eq(result, VK_SUCCESS);

	puts("\nVK available instance layers:");

	for(
		available_instance_layer = available_instance_layers;
		available_instance_layer < available_instance_layer_end;
		available_instance_layer++
		)
	{
		printf("- %s\n", available_instance_layer->layerName);
	}

	puts("");

	const char* const* instance_layer = vk_instance_layers;
	const char* const* instance_layer_end = instance_layer + MACRO_ARRAY_LEN(vk_instance_layers);

	while(instance_layer < instance_layer_end)
	{
		bool found = false;
		const char* layer_name = *(instance_layer++);

		available_instance_layer = available_instance_layers;
		while(available_instance_layer < available_instance_layer_end)
		{
			if(strcmp(layer_name, available_instance_layer->layerName) == 0)
			{
				found = true;
				break;
			}

			available_instance_layer++;
		}

		hard_assert_true(found, fprintf(stderr, "VK instance layer %s not found\n", layer_name));
		printf("+ %s\n", layer_name);
		*(layer++) = cstr_init(layer_name);
	}

	return layer;
}


void
vk_free_str_array(
	const char** arr,
	const char** arr_end
	)
{
	while(arr < arr_end)
	{
		cstr_free(*arr);
		arr++;
	}
}


void
vk_init_instance(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	volkInitializeCustom(window_get_vulkan_proc_addr_fn());

	const char* instance_extensions[VK_MAX_EXTENSIONS];
	const char** instance_extension = vk_get_instance_extensions(vk, instance_extensions);
	hard_assert_lt(instance_extension, instance_extensions + MACRO_ARRAY_LEN(instance_extensions));

	const char* instance_layers[VK_MAX_EXTENSIONS];
	const char** instance_layer = vk_get_instance_layers(vk, instance_layers);
	hard_assert_lt(instance_layer, instance_layers + MACRO_ARRAY_LEN(instance_layers));

	VkApplicationInfo application_info =
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = NULL,
		.pApplicationName = "Game",
		.apiVersion = VK_API_VERSION_1_0
	};

	VkInstanceCreateInfo instance_info =
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.pApplicationInfo = &application_info,
		.enabledLayerCount = instance_layer - instance_layers,
		.ppEnabledLayerNames = instance_layers,
		.enabledExtensionCount = instance_extension - instance_extensions,
		.ppEnabledExtensionNames = instance_extensions
	};

#ifndef NDEBUG
	VkDebugUtilsMessengerCreateInfoEXT debug_info =
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = vk_debug_callback,
		.pUserData = NULL
	};

	instance_info.pNext = &debug_info;
#endif

	VkResult result = vkCreateInstance(&instance_info, NULL, &vk->instance);
	hard_assert_eq(result, VK_SUCCESS);

	vk_free_str_array(instance_extensions, instance_extension);
	vk_free_str_array(instance_layers, instance_layer);

	volkLoadInstanceOnly(vk->instance);

#ifndef NDEBUG
	result = vkCreateDebugUtilsMessengerEXT(vk->instance, &debug_info, NULL, &vk->messenger);
	hard_assert_eq(result, VK_SUCCESS);
#endif
}


void
vk_free_instance(
	vulkan_t vk
	)
{
	assert_not_null(vk);

#ifndef NDEBUG
	vkDestroyDebugUtilsMessengerEXT(vk->instance, vk->messenger, NULL);
#endif

	vkDestroyInstance(vk->instance, NULL);

	volkFinalize();
}


void
vk_init_surface(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	window_init_vulkan_surface(vk->window, vk->instance, &vk->surface);
}


void
vk_free_surface(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	window_free_vulkan_surface(vk->instance, vk->surface);
}


typedef struct vk_device_score
{
	const char* name;
	uint32_t score;
	uint32_t queue_id;
	VkFormat format;
	VkSampleCountFlagBits samples;
	VkPhysicalDeviceLimits limits;
}
vk_device_score_t;


bool
vk_get_device_features(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	if(!features.sampleRateShading)
	{
		hard_assert_log();
		return false;
	}

	if(!features.textureCompressionBC)
	{
		hard_assert_log();
		return false;
	}

	return true;
}


bool
vk_get_device_queues(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	uint32_t queue_count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, NULL);
	if(queue_count == 0)
	{
		hard_assert_log();
		return false;
	}

	VkQueueFamilyProperties queues[queue_count];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues);

	VkQueueFamilyProperties* queue = queues;
	for(uint32_t i = 0; i < queue_count; ++i, ++queue)
	{
		VkBool32 present;
		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vk->surface, &present);
		if(result != VK_SUCCESS)
		{
			hard_assert_log();
			continue;
		}

		if(present && (queue->queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			device_score->queue_id = i;
			return true;
		}
	}

	hard_assert_log();
	return false;
}


bool
vk_get_device_extensions(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	if(MACRO_ARRAY_LEN(vk_device_extensions) == 0)
	{
		return true;
	}

	uint32_t available_device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(device, NULL, &available_device_extension_count, NULL);
	if(available_device_extension_count == 0)
	{
		hard_assert_log();
		return false;
	}

	VkExtensionProperties available_device_extensions[available_device_extension_count];
	VkExtensionProperties* available_device_extension = available_device_extensions;
	VkExtensionProperties* available_device_extension_end =
		available_device_extension + available_device_extension_count;

	while(available_device_extension < available_device_extension_end)
	{
		*(available_device_extension++) = (VkExtensionProperties){0};
	}

	vkEnumerateDeviceExtensionProperties(device, NULL,
		&available_device_extension_count, available_device_extensions);

	puts("\nVK available device extensions:");

	for(
		available_device_extension = available_device_extensions;
		available_device_extension < available_device_extension_end;
		available_device_extension++
		)
	{
		printf("- %s\n", available_device_extension->extensionName);
	}

	puts("");

	const char* const* device_extension = vk_device_extensions;
	const char* const* device_extension_end = device_extension + MACRO_ARRAY_LEN(vk_device_extensions);

	while(device_extension < device_extension_end)
	{
		bool found = false;
		const char* extension_name = *(device_extension++);

		available_device_extension = available_device_extensions;
		while(available_device_extension < available_device_extension_end)
		{
			if(strcmp(extension_name, available_device_extension->extensionName) == 0)
			{
				found = true;
				break;
			}

			available_device_extension++;
		}

		if(!found)
		{
			printf("VK device extension %s not found\n", extension_name);
			return false;
		}

		printf("+ %s\n", extension_name);
	}

	return true;
}


bool
vk_get_device_layers(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	if(MACRO_ARRAY_LEN(vk_device_layers) == 0)
	{
		return true;
	}

	uint32_t available_device_layer_count = 0;
	VkResult result = vkEnumerateDeviceLayerProperties(device, &available_device_layer_count, NULL);
	if(result != VK_SUCCESS || available_device_layer_count == 0)
	{
		hard_assert_log();
		return false;
	}

	VkLayerProperties available_device_layers[available_device_layer_count];
	result = vkEnumerateDeviceLayerProperties(device, &available_device_layer_count, available_device_layers);
	if(result != VK_SUCCESS)
	{
		hard_assert_log();
		return false;
	}

	puts("\nVK available device layers:");

	for(uint32_t i = 0; i < available_device_layer_count; ++i)
	{
		printf("- %s\n", available_device_layers[i].layerName);
	}

	puts("");

	const char* const* device_layer = vk_device_layers;
	const char* const* device_layer_end = device_layer + MACRO_ARRAY_LEN(vk_device_layers);

	while(device_layer < device_layer_end)
	{
		bool found = false;
		const char* layer_name = *(device_layer++);

		VkLayerProperties* layer = available_device_layers;
		VkLayerProperties* layer_end = layer + available_device_layer_count;

		while(layer < layer_end)
		{
			if(strcmp(layer_name, layer->layerName) == 0)
			{
				found = true;
				break;
			}

			layer++;
		}

		if(!found)
		{
			printf("VK device layer %s not found\n", layer_name);
			return false;
		}

		printf("+ %s\n", layer_name);
	}

	return true;
}


bool
vk_get_device_swapchain(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	uint32_t format_count;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk->surface, &format_count, NULL);
	if(result != VK_SUCCESS || format_count == 0)
	{
		hard_assert_log();
		return false;
	}

	VkSurfaceFormatKHR formats[format_count];
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, vk->surface, &format_count, formats);
	if(result != VK_SUCCESS)
	{
		hard_assert_log();
		return false;
	}

	VkSurfaceFormatKHR* format = formats;
	VkSurfaceFormatKHR* format_end = format + format_count;

	while(1)
	{
		if(
			(
				format->format == VK_FORMAT_R8G8B8A8_SRGB ||
				format->format == VK_FORMAT_B8G8R8A8_SRGB
				) &&
			format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			)
		{
			device_score->format = format->format;
			return true;
		}

		if(++format == format_end)
		{
			hard_assert_log();
			return false;
		}
	}
}


bool
vk_get_device_properties(
	vulkan_t vk,
	VkPhysicalDevice device,
	vk_device_score_t* device_score
	)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	device_score->name = cstr_init(properties.deviceName);

	if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		device_score->score += 1000;
	}

	VkSampleCountFlags sample_count =
		properties.limits.framebufferColorSampleCounts &
		properties.limits.framebufferDepthSampleCounts;

	if(sample_count >= VK_SAMPLE_COUNT_16_BIT)
	{
		device_score->samples = VK_SAMPLE_COUNT_16_BIT;
	}
	else if(sample_count >= VK_SAMPLE_COUNT_8_BIT)
	{
		device_score->samples = VK_SAMPLE_COUNT_8_BIT;
	}
	else if(sample_count >= VK_SAMPLE_COUNT_4_BIT)
	{
		device_score->samples = VK_SAMPLE_COUNT_4_BIT;
	}
	else if(sample_count >= VK_SAMPLE_COUNT_2_BIT)
	{
		device_score->samples = VK_SAMPLE_COUNT_2_BIT;
	}
	else
	{
		hard_assert_log();
		return false;
	}

	device_score->score += device_score->samples * 16;

	if(properties.limits.maxImageDimension2D < 1024)
	{
		hard_assert_log("%u\n", properties.limits.maxImageDimension2D);
		return false;
	}

	if(properties.limits.maxPushConstantsSize < sizeof(vk_vertex_consts_t))
	{
		hard_assert_log("%u\n", properties.limits.maxPushConstantsSize);
		return false;
	}

	if(properties.limits.maxBoundDescriptorSets < 1)
	{
		hard_assert_log("%u\n", properties.limits.maxBoundDescriptorSets);
		return false;
	}

	device_score->score += properties.limits.maxImageDimension2D;
	device_score->limits = properties.limits;

	return true;
}


vk_device_score_t
vk_get_device_score(
	vulkan_t vk,
	VkPhysicalDevice device
	)
{
	assert_not_null(vk);
	assert_not_null(device);

	vk_device_score_t device_score = {0};

	if(!vk_get_device_extensions(vk, device, &device_score))
	{
		goto goto_err;
	}

	if(!vk_get_device_layers(vk, device, &device_score))
	{
		goto goto_err;
	}

	if(!vk_get_device_features(vk, device, &device_score))
	{

		goto goto_err;
	}
	if(!vk_get_device_queues(vk, device, &device_score))
	{
		goto goto_err;
	}

	if(!vk_get_device_swapchain(vk, device, &device_score))
	{
		goto goto_err;
	}

	if(!vk_get_device_properties(vk, device, &device_score))
	{
		goto goto_err;
	}

	return device_score;


	goto_err:
	device_score.score = 0;
	return device_score;
}


void
vk_update_constants(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->viewport =
	(VkViewport)
	{
		.x = 0.0f,
		.y = 0.0f,
		.width = vk->extent.width,
		.height = vk->extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	vk->scissor =
	(VkRect2D)
	{
		.offset = { .x = 0, .y = 0 },
		.extent = vk->extent
	};

	vk->consts.window_size =
	(pair_t)
	{
		.w = vk->extent.width,
		.h = vk->extent.height
	};
}


void
vk_get_extent(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	int width = 0;
	int height = 0;

	while(1)
	{
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			vk->physical_device, vk->surface, &vk->surface_capabilities);
		hard_assert_eq(result, VK_SUCCESS);

		width = vk->surface_capabilities.currentExtent.width;
		height = vk->surface_capabilities.currentExtent.height;

		if(width != 0 && height != 0)
		{
			break;
		}

		sync_mtx_lock(&vk->resize_mtx);
			sync_cond_wait(&vk->resize_cond, &vk->resize_mtx);
		sync_mtx_unlock(&vk->resize_mtx);
	}

	if(width == UINT32_MAX || height == UINT32_MAX)
	{
		window_info_t window_info;
		window_get_info(vk->window, &window_info);

		width = window_info.extent.w;
		height = window_info.extent.h;
	}

	width = MACRO_CLAMP(width,
		vk->surface_capabilities.minImageExtent.width,
		vk->surface_capabilities.maxImageExtent.width);

	height = MACRO_CLAMP(height,
		vk->surface_capabilities.minImageExtent.height,
		vk->surface_capabilities.maxImageExtent.height);

	vk->extent =
	(VkExtent2D)
	{
		.width = width,
		.height = height
	};

	vk_update_constants(vk);
}


void
vk_init_device(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	uint32_t physical_device_count = 0;
	VkResult result = vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, NULL);
	hard_assert_eq(result, VK_SUCCESS);
	hard_assert_neq(physical_device_count, 0);

	VkPhysicalDevice physical_devices[physical_device_count];
	VkPhysicalDevice* physical_device = physical_devices;
	VkPhysicalDevice* physical_device_end = physical_device + physical_device_count;

	result = vkEnumeratePhysicalDevices(vk->instance, &physical_device_count, physical_devices);
	hard_assert_eq(result, VK_SUCCESS);

	VkPhysicalDevice best_device = NULL;
	vk_device_score_t best_device_score = {0};

	printf("\nVK available physical devices (%u):\n", physical_device_count);

	do
	{
		vk_device_score_t this_device_score = vk_get_device_score(vk, *physical_device);

		printf(
			"\n= %s:\n"
			"\tscore: %u\n"
			"\tqueue_id: %u\n"
			"\tformat: %u\n"
			"\tsamples: %u\n",
			this_device_score.name,
			this_device_score.score,
			this_device_score.queue_id,
			this_device_score.format,
			this_device_score.samples
			);
		cstr_free(this_device_score.name);

		if(this_device_score.score > best_device_score.score)
		{
			best_device = *physical_device;
			best_device_score = this_device_score;
		}
	}
	while(++physical_device != physical_device_end);

	hard_assert_not_null(best_device);

	vk->queue_id = best_device_score.queue_id;
	vk->format = best_device_score.format;
	vk->samples = MACRO_MIN(vk->max_msaa_samples, best_device_score.samples);
	vk->limits = best_device_score.limits;

	vk->physical_device = best_device;


	float priority = 1.0f;
	VkDeviceQueueCreateInfo device_queue_info =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = vk->queue_id,
		.queueCount = 1,
		.pQueuePriorities = &priority
	};

	VkPhysicalDeviceFeatures device_features =
	{
		.sampleRateShading = VK_TRUE,
		.textureCompressionBC = VK_TRUE
	};

	VkDeviceCreateInfo device_info =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &device_queue_info,
		.enabledLayerCount = MACRO_ARRAY_LEN(vk_device_layers),
		.ppEnabledLayerNames = vk_device_layers,
		.enabledExtensionCount = MACRO_ARRAY_LEN(vk_device_extensions),
		.ppEnabledExtensionNames = vk_device_extensions,
		.pEnabledFeatures = &device_features
	};

	result = vkCreateDevice(best_device, &device_info, NULL, &vk->device);
	hard_assert_eq(result, VK_SUCCESS);


	volkLoadDeviceTable(&vk->table, vk->device);

	vk->table.vkGetDeviceQueue(vk->device, vk->queue_id, 0, &vk->queue);
	vkGetPhysicalDeviceMemoryProperties(vk->physical_device, &vk->memory_properties);


	vk_get_extent(vk);

	if(!vk->surface_capabilities.maxImageCount)
	{
		vk->surface_capabilities.maxImageCount = UINT32_MAX;
	}

	vk->image_count = MACRO_CLAMP(
		vk->buffering,
		vk->surface_capabilities.minImageCount,
		vk->surface_capabilities.maxImageCount
		);
	vk->transform = vk->surface_capabilities.currentTransform;


	uint32_t present_mode_count;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vk->physical_device, vk->surface, &present_mode_count, NULL);
	hard_assert_eq(result, VK_SUCCESS);
	hard_assert_neq(present_mode_count, 0);

	VkPresentModeKHR present_modes[present_mode_count];
	VkPresentModeKHR* present_mode = present_modes;
	VkPresentModeKHR* present_mode_end =
		present_mode + present_mode_count;

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		vk->physical_device, vk->surface, &present_mode_count, present_modes);
	hard_assert_eq(result, VK_SUCCESS);

	while(1)
	{
		if(*present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			vk->present_mode = *present_mode;
			break;
		}

		if(++present_mode == present_mode_end)
		{
			vk->present_mode = VK_PRESENT_MODE_FIFO_KHR;
			break;
		}
	}
}


void
vk_free_device(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->table.vkDestroyDevice(vk->device, NULL);
}


uint32_t
vk_get_memory(
	vulkan_t vk,
	uint32_t bits,
	VkMemoryPropertyFlags flags
	)
{
	for(uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; ++i)
	{
		if(
			(bits & (1 << i)) &&
			(vk->memory_properties.memoryTypes[i].propertyFlags & flags) == flags
			)
		{
			return i;
		}
	}

	hard_assert_unreachable();
}


void
vk_init_buffer(
	vulkan_t vk,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags flags,
	vk_buffer_t* buffer
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);

	VkBufferCreateInfo buffer_info =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL
	};

	VkResult result = vk->table.vkCreateBuffer(vk->device, &buffer_info, NULL, &buffer->buffer);
	hard_assert_eq(result, VK_SUCCESS);

	VkMemoryRequirements memory_requirements;
	vk->table.vkGetBufferMemoryRequirements(vk->device, buffer->buffer, &memory_requirements);

	uint32_t memory_type_index = vk_get_memory(vk, memory_requirements.memoryTypeBits, flags);

	VkMemoryAllocateInfo memory_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	result = vk->table.vkAllocateMemory(vk->device, &memory_info, NULL, &buffer->memory);
	hard_assert_eq(result, VK_SUCCESS);

	result = vk->table.vkBindBufferMemory(vk->device, buffer->buffer, buffer->memory, 0);
	hard_assert_eq(result, VK_SUCCESS);
}


void
vk_free_buffer(
	vulkan_t vk,
	vk_buffer_t* buffer
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);

	vk->table.vkFreeMemory(vk->device, buffer->memory, NULL);
	buffer->memory = NULL;

	vk->table.vkDestroyBuffer(vk->device, buffer->buffer, NULL);
	buffer->buffer = NULL;
}


void
vk_init_staging_buffer(
	vulkan_t vk,
	VkDeviceSize size,
	vk_buffer_t* buffer
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vk_init_buffer(vk, size, usage, flags, buffer);
}


void
vk_init_vertex_buffer(
	vulkan_t vk,
	VkDeviceSize size,
	vk_buffer_t* buffer
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vk_init_buffer(vk, size, usage, flags, buffer);
}


void
vk_init_indirect_buffer(
	vulkan_t vk,
	VkDeviceSize size,
	vk_buffer_t* buffer
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);

	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	vk_init_buffer(vk, sizeof(VkDrawIndirectCommand) * size, usage, flags, buffer);
}


void
vk_init_commands(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkCommandPoolCreateInfo command_pool_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = vk->queue_id
	};

	VkResult result = vk->table.vkCreateCommandPool(vk->device, &command_pool_info, NULL, &vk->command_pool);
	hard_assert_eq(result, VK_SUCCESS);


	VkCommandBuffer command_buffers[MACRO_ARRAY_LEN(vk->commands)];
	VkCommandBuffer* command_buffer = command_buffers;

	VkCommandBufferAllocateInfo command_buffer_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = vk->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = MACRO_ARRAY_LEN(command_buffers)
	};

	result = vk->table.vkAllocateCommandBuffers(vk->device, &command_buffer_info, command_buffers);
	hard_assert_eq(result, VK_SUCCESS);

	VkFenceCreateInfo fence_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	vk_command_t* command = vk->commands;
	vk_command_t* command_end = command + MACRO_ARRAY_LEN(vk->commands);

	while(command != command_end)
	{
		command->buffer = *command_buffer;

		result = vk->table.vkCreateFence(vk->device, &fence_info, NULL, &command->fence);
		hard_assert_eq(result, VK_SUCCESS);

		command->waited = false;
		command->using_temp = false;

		vk_init_staging_buffer(vk, VK_STAGING_BUFFER_SIZE, &command->staging_buffer);

		++command_buffer;
		++command;
	}

	vk->command = vk->commands;
}


void
vk_free_commands(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkCommandBuffer command_buffers[MACRO_ARRAY_LEN(vk->commands)];
	VkCommandBuffer* command_buffer = command_buffers;

	vk_command_t* command = vk->commands;
	vk_command_t* command_end = command + MACRO_ARRAY_LEN(vk->commands);

	while(command != command_end)
	{
		vk_free_buffer(vk, &command->staging_buffer);

		if(command->using_temp)
		{
			vk_free_buffer(vk, &command->temp_staging_buffer);
		}

		vk->table.vkDestroyFence(vk->device, command->fence, NULL);

		*command_buffer = command->buffer;

		++command_buffer;
		++command;
	}

	vk->table.vkFreeCommandBuffers(vk->device, vk->command_pool, MACRO_ARRAY_LEN(vk->commands), command_buffers);

	vk->table.vkDestroyCommandPool(vk->device, vk->command_pool, NULL);
}


void
vk_wait_command(
	vulkan_t vk,
	vk_command_t* command
	)
{
	assert_not_null(vk);
	assert_not_null(command);

	if(command->waited)
	{
		return;
	}

	VkResult result = vk->table.vkWaitForFences(vk->device, 1, &command->fence, VK_TRUE, UINT64_MAX);
	hard_assert_eq(result, VK_SUCCESS);

	result = vk->table.vkResetFences(vk->device, 1, &command->fence);
	hard_assert_eq(result, VK_SUCCESS);

	command->waited = true;

	if(command->using_temp)
	{
		vk_free_buffer(vk, &command->temp_staging_buffer);
		command->using_temp = false;
	}
}


vk_command_t*
vk_get_command(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	if(vk->command >= vk->commands + MACRO_ARRAY_LEN(vk->commands))
	{
		vk->command = vk->commands;
	}

	vk_command_t* command = vk->command;
	++vk->command;

	if(!command->waited)
	{
		vk_wait_command(vk, command);
	}

	VkResult result = vk->table.vkResetCommandBuffer(command->buffer, 0);
	hard_assert_eq(result, VK_SUCCESS);

	VkCommandBufferBeginInfo command_buffer_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = NULL
	};

	result = vk->table.vkBeginCommandBuffer(command->buffer, &command_buffer_info);
	hard_assert_eq(result, VK_SUCCESS);

	return command;
}


void
vk_run_command(
	vulkan_t vk,
	vk_command_t* command
	)
{
	assert_not_null(vk);
	assert_not_null(command);

	VkResult result = vk->table.vkEndCommandBuffer(command->buffer);
	hard_assert_eq(result, VK_SUCCESS);

	VkSubmitInfo submit_info =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 0,
		.pWaitSemaphores = NULL,
		.pWaitDstStageMask = NULL,
		.commandBufferCount = 1,
		.pCommandBuffers = &command->buffer,
		.signalSemaphoreCount = 0,
		.pSignalSemaphores = NULL
	};

	result = vk->table.vkQueueSubmit(vk->queue, 1, &submit_info, command->fence);
	hard_assert_eq(result, VK_SUCCESS);

	command->waited = false;
}


void
vk_copy_to_buffer_explicit(
	vulkan_t vk,
	vk_buffer_t* buffer,
	const void* data,
	VkDeviceSize size
	)
{
	assert_not_null(vk);
	assert_not_null(buffer);
	assert_ptr(data, size);

	if(!size)
	{
		return;
	}

	vk_command_t* command = vk_get_command(vk);

	vk_buffer_t* staging_buffer = &command->staging_buffer;

	if(size > VK_STAGING_BUFFER_SIZE)
	{
		vk_init_staging_buffer(vk, size, &command->temp_staging_buffer);
		staging_buffer = &command->temp_staging_buffer;
		command->using_temp = true;
	}

	void* mapped_data;
	VkResult result = vk->table.vkMapMemory(vk->device, staging_buffer->memory, 0, size, 0, &mapped_data);
	hard_assert_eq(result, VK_SUCCESS);

	memcpy(mapped_data, data, size);

	vk->table.vkUnmapMemory(vk->device, staging_buffer->memory);

	VkBufferCopy buffer_copy =
	{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size
	};

	vk->table.vkCmdCopyBuffer(command->buffer, staging_buffer->buffer, buffer->buffer, 1, &buffer_copy);

	vk_run_command(vk, command);
}


#define vk_copy_to_buffer(vk, buffer, data, size)	\
vk_copy_to_buffer_explicit((vk), (buffer), (data), sizeof(*(data)) * (size))


void
vk_copy_texture_to_image(
	vulkan_t vk,
	vk_image_t* image,
	dds_tex_t* tex
	)
{
	assert_not_null(vk);
	assert_not_null(image);
	assert_not_null(tex);

	VkDeviceSize size = dds_data_size(tex);
	uint8_t* data = &tex->data[0];

	vk_command_t* command = vk_get_command(vk);

	vk_buffer_t* staging_buffer = &command->staging_buffer;

	if(size > VK_STAGING_BUFFER_SIZE)
	{
		vk_init_staging_buffer(vk, size, &command->temp_staging_buffer);
		staging_buffer = &command->temp_staging_buffer;
		command->using_temp = true;
	}

	void* mapped_data;
	VkResult result = vk->table.vkMapMemory(vk->device, staging_buffer->memory, 0, size, 0, &mapped_data);
	hard_assert_eq(result, VK_SUCCESS);

	memcpy(mapped_data, data, size);

	vk->table.vkUnmapMemory(vk->device, staging_buffer->memory);

	VkBufferImageCopy buffer_image_copies[image->layers];
	VkBufferImageCopy* buffer_image_copy = buffer_image_copies;

	for(uint32_t layer = 0; layer < image->layers; ++layer)
	{
		*(buffer_image_copy++) =
		(VkBufferImageCopy)
		{
			.bufferOffset = dds_offset(tex, layer),
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = layer,
				.layerCount = 1
			},
			.imageOffset =
			{
				.x = 0,
				.y = 0,
				.z = 0
			},
			.imageExtent = { image->width, image->height, 1 }
		};
	}

	vk->table.vkCmdCopyBufferToImage(command->buffer, staging_buffer->buffer, image->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MACRO_ARRAY_LEN(buffer_image_copies), buffer_image_copies);

	vk_run_command(vk, command);
}


void
vk_transition_image_layout(
	vulkan_t vk,
	vk_image_t* image,
	VkImageLayout from,
	VkImageLayout to
	)
{
	assert_not_null(vk);
	assert_not_null(image);

	vk_command_t* command = vk_get_command(vk);

	VkPipelineStageFlags src_stage;
	VkPipelineStageFlags dst_stage;

	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = NULL,
		.oldLayout = from,
		.newLayout = to,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image->image,
		.subresourceRange =
		{
			.aspectMask = image->aspect,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = image->layers
		}
	};

	if(
		from == VK_IMAGE_LAYOUT_UNDEFINED &&
		(
			to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
			to == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			)
		)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(
		(
			from == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
			from == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			) &&
		to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		hard_assert_unreachable();
	}

	vk->table.vkCmdPipelineBarrier(command->buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

	vk_run_command(vk, command);
}


void
vk_copy_data_to_image(
	vulkan_t vk,
	vk_image_t* image,
	dds_tex_t* tex
	)
{
	assert_not_null(vk);
	assert_not_null(image);
	assert_not_null(tex);

	vk_transition_image_layout(vk, image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vk_copy_texture_to_image(vk, image, tex);

	vk_transition_image_layout(vk, image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}


void
vk_create_image(
	vulkan_t vk,
	vk_image_t* image
	)
{
	assert_not_null(vk);
	assert_not_null(image);

	dds_tex_t* tex = NULL;


	switch(image->type)
	{

	case VK_IMAGE_TYPE_DEPTH_STENCIL:
	{
		image->aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		image->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image->samples = vk->samples;
		image->width = vk->extent.width;
		image->height = vk->extent.height;
		image->layers = 1;
		break;
	}

	case VK_IMAGE_TYPE_MULTISAMPLED:
	{
		image->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		image->usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		image->samples = vk->samples;
		image->width = vk->extent.width;
		image->height = vk->extent.height;
		image->layers = 1;
		break;
	}

	case VK_IMAGE_TYPE_TEXTURE:
	{
		tex = dds_load(image->path);
		image->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		image->usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image->samples = VK_SAMPLE_COUNT_1_BIT;
		image->width = tex->width;
		image->height = tex->height;
		image->layers = tex->array_size;
		break;
	}

	default: assert_unreachable();

	}


	VkImageCreateInfo image_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = image->format,
		.extent = { image->width, image->height, 1 },
		.mipLevels = 1,
		.arrayLayers = image->layers,
		.samples = image->samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = image->usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkResult result = vk->table.vkCreateImage(vk->device, &image_info, NULL, &image->image);
	hard_assert_eq(result, VK_SUCCESS);

	VkMemoryRequirements memory_requirements;
	vk->table.vkGetImageMemoryRequirements(vk->device, image->image, &memory_requirements);

	uint32_t memory_type_index = vk_get_memory(vk,
		memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo memory_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memory_requirements.size,
		.memoryTypeIndex = memory_type_index
	};

	result = vk->table.vkAllocateMemory(vk->device, &memory_info, NULL, &image->memory);
	hard_assert_eq(result, VK_SUCCESS);

	result = vk->table.vkBindImageMemory(vk->device, image->image, image->memory, 0);
	hard_assert_eq(result, VK_SUCCESS);

	VkImageViewCreateInfo view_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = image->image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
		.format = image->format,
		.components =
		{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange =
		{
			.aspectMask = image->aspect,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = image->layers
		}
	};

	result = vk->table.vkCreateImageView(vk->device, &view_info, NULL, &image->view);
	hard_assert_eq(result, VK_SUCCESS);

	if(image->type == VK_IMAGE_TYPE_TEXTURE)
	{
		vk_copy_data_to_image(vk, image, tex);
		dds_free(tex);
	}
}


void
vk_free_image(
	vulkan_t vk,
	vk_image_t* image
	)
{
	assert_not_null(vk);
	assert_not_null(image);

	vk->table.vkDestroyImageView(vk->device, image->view, NULL);
	vk->table.vkDestroyImage(vk->device, image->image, NULL);
	vk->table.vkFreeMemory(vk->device, image->memory, NULL);
}


void
vk_init_images(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->depth_image =
	(vk_image_t)
	{
		.format = VK_FORMAT_D32_SFLOAT,
		.type = VK_IMAGE_TYPE_DEPTH_STENCIL
	};
	vk_create_image(vk, &vk->depth_image);

	vk->multisampling_image =
	(vk_image_t)
	{
		.format = vk->format,
		.type = VK_IMAGE_TYPE_MULTISAMPLED
	};
	vk_create_image(vk, &vk->multisampling_image);
}


void
vk_free_images(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk_free_image(vk, &vk->multisampling_image);
	vk_free_image(vk, &vk->depth_image);
}


VkShaderModule
vk_init_shader(
	vulkan_t vk,
	const char* path
	)
{
	assert_not_null(vk);
	assert_not_null(path);

	file_t file;
	bool status = file_read(path, &file);
	hard_assert_true(status);

	VkShaderModuleCreateInfo shader_info =
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = file.len,
		.pCode = (void*) file.data
	};

	VkShaderModule shader_module;
	VkResult result = vk->table.vkCreateShaderModule(vk->device, &shader_info, NULL, &shader_module);
	hard_assert_eq(result, VK_SUCCESS);

	file_free(file);

	return shader_module;
}


void
vk_free_shader(
	vulkan_t vk,
	VkShaderModule shader
	)
{
	assert_not_null(vk);
	assert_not_null(shader);

	vk->table.vkDestroyShaderModule(vk->device, shader, NULL);
}


VkPipelineCache
vk_init_pipeline_cache(
	vulkan_t vk,
	const char* path
	)
{
	assert_not_null(vk);
	assert_not_null(path);

	VkPipelineCacheCreateInfo pipeline_cache_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = NULL
	};

	file_t file = {0};
	VkPipelineCache pipeline_cache;

	if(file_exists(path))
	{
		bool status = file_read(path, &file);
		if(status)
		{
			pipeline_cache_info.initialDataSize = file.len;
			pipeline_cache_info.pInitialData = file.data;
		}
		else
		{
			hard_assert_log("file_read(\"%s\")\n", path);
		}
	}

	VkResult result = vk->table.vkCreatePipelineCache(vk->device, &pipeline_cache_info, NULL, &pipeline_cache);
	hard_assert_eq(result, VK_SUCCESS);

	file_free(file);

	return pipeline_cache;
}


void
vk_free_pipeline_cache(
	vulkan_t vk,
	const char* path,
	VkPipelineCache pipeline_cache
	)
{
	assert_not_null(vk);
	assert_not_null(path);
	assert_not_null(pipeline_cache);

	file_t file;
	VkResult result = vk->table.vkGetPipelineCacheData(vk->device, pipeline_cache, &file.len, NULL);
	hard_assert_eq(result, VK_SUCCESS);

	file.data = alloc_malloc(file.data, file.len);
	assert_ptr(file.data, file.len);

	result = vk->table.vkGetPipelineCacheData(vk->device, pipeline_cache, &file.len, file.data);
	hard_assert_eq(result, VK_SUCCESS);

	bool status = file_write(path, file);
	if(!status)
	{
		hard_assert_log("file_write(\"%s\")\n", path);
	}

	file_free(file);

	vk->table.vkDestroyPipelineCache(vk->device, pipeline_cache, NULL);
}


void
vk_init_render_pass(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkAttachmentDescription attachments[] =
	{
		{
			.flags = 0,
			.format = vk->multisampling_image.format,
			.samples = vk->samples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		},
		{
			.flags = 0,
			.format = vk->depth_image.format,
			.samples = vk->samples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		},
		{
			.flags = 0,
			.format = vk->format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		}
	};

	VkAttachmentReference color_attachment_ref =
	{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	VkAttachmentReference depth_attachment_ref =
	{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	VkAttachmentReference multisampling_ref =
	{
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};

	VkSubpassDescription subpass =
	{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_ref,
		.pResolveAttachments = &multisampling_ref,
		.pDepthStencilAttachment = &depth_attachment_ref
	};

	VkSubpassDependency subpass_dependency =
	{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};

	VkRenderPassCreateInfo render_pass_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.attachmentCount = MACRO_ARRAY_LEN(attachments),
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &subpass_dependency
	};

	VkResult result = vk->table.vkCreateRenderPass(vk->device, &render_pass_info, NULL, &vk->render_pass);
	hard_assert_eq(result, VK_SUCCESS);
}


void
vk_free_render_pass(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->table.vkDestroyRenderPass(vk->device, vk->render_pass, NULL);
}


void
vk_init_pipeline(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkPipelineShaderStageCreateInfo shader_stages[] =
	{
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vk_init_shader(vk, "shaders/vert.spv"),
			.pName = "main"
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = vk_init_shader(vk, "shaders/frag.spv"),
			.pName = "main"
		}
	};

	VkDynamicState dynamic_states[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = MACRO_ARRAY_LEN(dynamic_states),
		.pDynamicStates = dynamic_states
	};

	VkVertexInputBindingDescription vertex_bindings[] =
	{
		{
			.binding = 0,
			.stride = sizeof(vk_vertex_input_t),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		},
		{
			.binding = 1,
			.stride = sizeof(vulkan_draw_data_t),
			.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
		}
	};

	VkVertexInputAttributeDescription vertex_attributes[] =
	{
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vk_vertex_input_t, pos)
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vk_vertex_input_t, tex_coord)
		},
		{
			.location = 2,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, pos)
		},
		{
			.location = 3,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, size)
		},
		{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(vulkan_draw_data_t, white_color)
		},
		{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, white_depth)
		},
		{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_B8G8R8A8_UNORM,
			.offset = offsetof(vulkan_draw_data_t, black_color)
		},
		{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, black_depth)
		},
		{
			.location = 8,
			.binding = 1,
			.format = VK_FORMAT_R16G16_UINT,
			.offset = offsetof(vulkan_draw_data_t, tex)
		},
		{
			.location = 9,
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, angle)
		},
		{
			.location = 10,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, tex_scale)
		},
		{
			.location = 11,
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(vulkan_draw_data_t, tex_offset)
		}
	};

	VkPipelineVertexInputStateCreateInfo vertex_input_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.vertexBindingDescriptionCount = MACRO_ARRAY_LEN(vertex_bindings),
		.pVertexBindingDescriptions = vertex_bindings,
		.vertexAttributeDescriptionCount = MACRO_ARRAY_LEN(vertex_attributes),
		.pVertexAttributeDescriptions = vertex_attributes
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		.primitiveRestartEnable = VK_FALSE
	};

	VkPipelineViewportStateCreateInfo viewport_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = NULL,
		.scissorCount = 1,
		.pScissors = NULL
	};

	VkPipelineRasterizationStateCreateInfo rasterization_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f
	};

	VkPipelineMultisampleStateCreateInfo multisample_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = vk->samples,
		.sampleShadingEnable = VK_TRUE,
		.minSampleShading = 0.2f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE
	};

	VkPipelineDepthStencilStateCreateInfo depth_stencil_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_GREATER,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {0},
		.back = {0},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f
	};

	VkPipelineColorBlendAttachmentState blending_attachment =
	{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo color_blend_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_CLEAR,
		.attachmentCount = 1,
		.pAttachments = &blending_attachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
	};


	VkDescriptorSetLayoutBinding bindings[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MACRO_ARRAY_LEN(vk->textures),
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		}
	};

	VkDescriptorSetLayoutCreateInfo set_layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = MACRO_ARRAY_LEN(bindings),
		.pBindings = bindings
	};

	VkResult result = vk->table.vkCreateDescriptorSetLayout(vk->device, &set_layout_info, NULL, &vk->set_layout);
	hard_assert_eq(result, VK_SUCCESS);


	VkPushConstantRange push_constants[] =
	{
		{
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.offset = 0,
			.size = sizeof(vk_vertex_consts_t)
		}
	};

	VkPipelineLayoutCreateInfo pipeline_layout_info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &vk->set_layout,
		.pushConstantRangeCount = MACRO_ARRAY_LEN(push_constants),
		.pPushConstantRanges = push_constants
	};

	result = vk->table.vkCreatePipelineLayout(vk->device, &pipeline_layout_info, NULL, &vk->pipeline_layout);
	hard_assert_eq(result, VK_SUCCESS);

	VkGraphicsPipelineCreateInfo pipeline_info =
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = MACRO_ARRAY_LEN(shader_stages),
		.pStages = shader_stages,
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly_info,
		.pTessellationState = NULL,
		.pViewportState = &viewport_info,
		.pRasterizationState = &rasterization_info,
		.pMultisampleState = &multisample_info,
		.pDepthStencilState = &depth_stencil_info,
		.pColorBlendState = &color_blend_info,
		.pDynamicState = &dynamic_state_info,
		.layout = vk->pipeline_layout,
		.renderPass = vk->render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	static const char* pipeline_cache_path = "cache/pipeline.bin";
	VkPipelineCache pipeline_cache = vk_init_pipeline_cache(vk, pipeline_cache_path);

	result = vk->table.vkCreateGraphicsPipelines(vk->device, pipeline_cache, 1, &pipeline_info, NULL, &vk->pipeline);
	hard_assert_eq(result, VK_SUCCESS);

	vk_free_pipeline_cache(vk, pipeline_cache_path, pipeline_cache);

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(shader_stages); ++i)
	{
		vk_free_shader(vk, shader_stages[i].module);
	}
}


void
vk_free_pipeline(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->table.vkDestroyPipeline(vk->device, vk->pipeline, NULL);
	vk->table.vkDestroyPipelineLayout(vk->device, vk->pipeline_layout, NULL);
	vk->table.vkDestroyDescriptorSetLayout(vk->device, vk->set_layout, NULL);
}


void
vk_free_swapchain(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->table.vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
}


void
vk_init_swapchain(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkSwapchainCreateInfoKHR swapchain_info =
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = vk->surface,
		.minImageCount = vk->image_count,
		.imageFormat = vk->format,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = vk->extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.preTransform = vk->transform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = vk->present_mode,
		.clipped = VK_TRUE,
		.oldSwapchain = vk->swapchain
	};

	VkSwapchainKHR swapchain;
	VkResult result = vk->table.vkCreateSwapchainKHR(vk->device, &swapchain_info, NULL, &swapchain);
	hard_assert_eq(result, VK_SUCCESS);

	vk_free_swapchain(vk);
	vk->swapchain = swapchain;
}


void
vk_init_barriers(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->barriers = alloc_calloc(vk->barriers, vk->buffering);
	hard_assert_not_null(vk->barriers);

	uint32_t command_buffer_count = vk->buffering * vk->image_count;
	VkCommandBuffer command_buffers[command_buffer_count];
	VkCommandBuffer* command_buffer = command_buffers;

	VkCommandBufferAllocateInfo command_buffer_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = vk->command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = command_buffer_count
	};

	VkResult result = vk->table.vkAllocateCommandBuffers(vk->device, &command_buffer_info, command_buffers);
	hard_assert_eq(result, VK_SUCCESS);


	VkSemaphoreCreateInfo semaphore_info =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};

	VkFenceCreateInfo fence_info =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = NULL,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	vk_barrier_t* barrier = vk->barriers;
	vk_barrier_t* barrier_end = barrier + vk->buffering;

	while(barrier < barrier_end)
	{
		result = vk->table.vkCreateSemaphore(vk->device, &semaphore_info, NULL, &barrier->semaphore);
		hard_assert_eq(result, VK_SUCCESS);

		result = vk->table.vkCreateFence(vk->device, &fence_info, NULL, &barrier->fence);
		hard_assert_eq(result, VK_SUCCESS);

		for(uint32_t i = 0; i < vk->image_count; ++i)
		{
			barrier->command_buffers[i] = *(command_buffer++);
		}

		++barrier;
	}

	vk->barrier = vk->barriers;
}


void
vk_free_barriers(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	uint32_t command_buffer_count = vk->buffering * vk->image_count;
	VkCommandBuffer command_buffers[command_buffer_count];
	VkCommandBuffer* command_buffer = command_buffers;

	vk_barrier_t* barrier = vk->barriers;
	vk_barrier_t* barrier_end = barrier + vk->buffering;

	while(barrier < barrier_end)
	{
		for(uint32_t i = 0; i < vk->image_count; ++i)
		{
			*(command_buffer++) = barrier->command_buffers[i];
		}

		vk->table.vkDestroyFence(vk->device, barrier->fence, NULL);
		vk->table.vkDestroySemaphore(vk->device, barrier->semaphore, NULL);

		++barrier;
	}

	vk->table.vkFreeCommandBuffers(vk->device, vk->command_pool, command_buffer_count, command_buffers);

	alloc_free(vk->barriers, vk->buffering);
}


void
vk_init_framebuffer(
	vulkan_t vk,
	vk_frame_t* frame,
	VkImage* swapchain_image
	)
{
	assert_not_null(vk);

	frame->image = *swapchain_image;

	VkImageViewCreateInfo view_info =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = frame->image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = vk->format,
		.components =
		{
			.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.a = VK_COMPONENT_SWIZZLE_IDENTITY
		},
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	VkResult result = vk->table.vkCreateImageView(vk->device, &view_info, NULL, &frame->view);
	hard_assert_eq(result, VK_SUCCESS);

	VkImageView attachments[] =
	{
		vk->multisampling_image.view,
		vk->depth_image.view,
		frame->view
	};

	VkFramebufferCreateInfo framebuffer_info =
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.renderPass = vk->render_pass,
		.attachmentCount = MACRO_ARRAY_LEN(attachments),
		.pAttachments = attachments,
		.width = vk->extent.width,
		.height = vk->extent.height,
		.layers = 1
	};

	result = vk->table.vkCreateFramebuffer(vk->device, &framebuffer_info, NULL, &frame->framebuffer);
	hard_assert_eq(result, VK_SUCCESS);
}


void
vk_free_framebuffer(
	vulkan_t vk,
	vk_frame_t* frame
	)
{
	assert_not_null(vk);

	vk->table.vkDestroyFramebuffer(vk->device, frame->framebuffer, NULL);
	vk->table.vkDestroyImageView(vk->device, frame->view, NULL);
}


void
vk_init_framebuffers(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	uint32_t image_count;
	VkResult result = vk->table.vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &image_count, NULL);
	hard_assert_eq(result, VK_SUCCESS);

	assert_lt(image_count, VK_MAX_IMAGES);
	assert_ge(image_count, vk->image_count);

	vk->image_count = image_count;

	VkImage images[image_count];
	result = vk->table.vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &image_count, images);
	hard_assert_eq(result, VK_SUCCESS);


	VkSemaphoreCreateInfo semaphore_info =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0
	};

	vk_frame_t* frame = vk->frames;
	vk_frame_t* frame_end = frame + vk->image_count;

	VkImage* image = images;

	while(frame < frame_end)
	{
		vk_init_framebuffer(vk, frame, image);


		result = vk->table.vkCreateSemaphore(vk->device, &semaphore_info, NULL, &frame->semaphore);
		hard_assert_eq(result, VK_SUCCESS);

		frame->barrier = NULL;

		vk_init_vertex_buffer(vk, VK_INSTANCE_BUFFER_SIZE, &frame->instance_input_buffer);
		vk_init_indirect_buffer(vk, 1, &frame->draw_count_buffer);


		++frame;
		++image;
	}
}


void
vk_free_framebuffers(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk_frame_t* frame = vk->frames;
	vk_frame_t* frame_end = frame + vk->image_count;

	while(frame < frame_end)
	{
		vk_free_buffer(vk, &frame->draw_count_buffer);
		vk_free_buffer(vk, &frame->instance_input_buffer);

		vk->table.vkDestroySemaphore(vk->device, frame->semaphore, NULL);

		vk_free_framebuffer(vk, frame);

		++frame;
	}
}


void
vk_init_textures(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkSamplerCreateInfo sampler_info =
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		.mipLodBias = 0.0f,
		.anisotropyEnable = VK_FALSE,
		.maxAnisotropy = 0.0f,
		.compareEnable = VK_FALSE,
		.compareOp = VK_COMPARE_OP_NEVER,
		.minLod = 0.0f,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkResult result = vk->table.vkCreateSampler(vk->device, &sampler_info, NULL, &vk->sampler);
	hard_assert_eq(result, VK_SUCCESS);


	vk_image_t* texture = vk->textures;
	vk_image_t* texture_end = texture + MACRO_ARRAY_LEN(vk->textures);

	const tex_file_t* tex_file = tex_files;

	while(texture < texture_end)
	{
		*texture =
		(vk_image_t)
		{
			.path = tex_file->path,
			.format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
			.type = VK_IMAGE_TYPE_TEXTURE
		};

		vk_create_image(vk, texture);

		++tex_file;
		++texture;
	}


	VkDescriptorPoolSize pool_sizes[] =
	{
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = MACRO_ARRAY_LEN(vk->textures)
		}
	};

	VkDescriptorPoolCreateInfo descriptor_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets = 1,
		.poolSizeCount = MACRO_ARRAY_LEN(pool_sizes),
		.pPoolSizes = pool_sizes
	};

	result = vk->table.vkCreateDescriptorPool(vk->device, &descriptor_info, NULL, &vk->descriptor_pool);
	hard_assert_eq(result, VK_SUCCESS);


	VkDescriptorSetAllocateInfo alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorPool = vk->descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &vk->set_layout
	};

	result = vk->table.vkAllocateDescriptorSets(vk->device, &alloc_info, &vk->descriptor_set);
	hard_assert_eq(result, VK_SUCCESS);


	VkDescriptorImageInfo image_infos[MACRO_ARRAY_LEN(vk->textures)];
	VkWriteDescriptorSet descriptor_writes[MACRO_ARRAY_LEN(vk->textures)];

	texture = vk->textures;

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(vk->textures); ++i)
	{
		image_infos[i] =
		(VkDescriptorImageInfo)
		{
			.sampler = vk->sampler,
			.imageView = texture->view,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		descriptor_writes[i] =
		(VkWriteDescriptorSet)
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk->descriptor_set,
			.dstBinding = 0,
			.dstArrayElement = i,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = image_infos + i,
			.pBufferInfo = NULL,
			.pTexelBufferView = NULL
		};

		++texture;
	}

	vk->table.vkUpdateDescriptorSets(vk->device, MACRO_ARRAY_LEN(vk->textures), descriptor_writes, 0, NULL);
}


void
vk_free_textures(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk->table.vkDestroyDescriptorPool(vk->device, vk->descriptor_pool, NULL);

	vk_image_t* texture = vk->textures;
	vk_image_t* texture_end = texture + MACRO_ARRAY_LEN(vk->textures);

	while(texture < texture_end)
	{
		vk_free_image(vk, texture);
		++texture;
	}

	vk->table.vkDestroySampler(vk->device, vk->sampler, NULL);
}


void
vk_init_vertex(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk_init_vertex_buffer(vk, sizeof(vk_vertex_input), &vk->vertex_input_buffer);
	vk_copy_to_buffer(vk, &vk->vertex_input_buffer, vk_vertex_input, MACRO_ARRAY_LEN(vk_vertex_input));

	vk->draw_data = alloc_malloc(vk->draw_data, VK_DRAW_DATA_BUFFER_SIZE);
	hard_assert_not_null(vk->draw_data);
}


void
vk_free_vertex(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	alloc_free(vk->draw_data, VK_DRAW_DATA_BUFFER_SIZE);
	vk_free_buffer(vk, &vk->vertex_input_buffer);
}


void
vk_record_commands(
	vulkan_t vk,
	VkCommandBuffer command_buffer,
	vk_frame_t* frame
	)
{
	VkDeviceSize offset = 0;

	VkClearValue clear_values[] =
	{
		{ .color = {{ 0.0f, 0.0f, 0.0f, 0.0f }} },
		{ .depthStencil = { 0.0f, 0 } }
	};

	VkResult result = vk->table.vkResetCommandBuffer(command_buffer, 0);
	hard_assert_eq(result, VK_SUCCESS);

	VkCommandBufferBeginInfo begin_info =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0,
		.pInheritanceInfo = NULL
	};

	result = vk->table.vkBeginCommandBuffer(command_buffer, &begin_info);
	hard_assert_eq(result, VK_SUCCESS);

	VkRenderPassBeginInfo render_pass_info =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = vk->render_pass,
		.framebuffer = frame->framebuffer,
		.renderArea =
		{
			.offset = { 0, 0 },
			.extent = vk->extent
		},
		.clearValueCount = MACRO_ARRAY_LEN(clear_values),
		.pClearValues = clear_values
	};

	vk->table.vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vk->table.vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk->pipeline);

	vk->table.vkCmdSetViewport(command_buffer, 0, 1, &vk->viewport);
	vk->table.vkCmdSetScissor(command_buffer, 0, 1, &vk->scissor);

	vk->table.vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk->vertex_input_buffer.buffer, &offset);
	vk->table.vkCmdBindVertexBuffers(command_buffer, 1, 1, &frame->instance_input_buffer.buffer, &offset);

	vk->table.vkCmdBindDescriptorSets(command_buffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS, vk->pipeline_layout, 0, 1, &vk->descriptor_set, 0, NULL);

	vk->table.vkCmdPushConstants(command_buffer, vk->pipeline_layout,
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vk->consts), &vk->consts);

	vk->table.vkCmdDrawIndirect(command_buffer, frame->draw_count_buffer.buffer, 0, 1, 0);

	vk->table.vkCmdEndRenderPass(command_buffer);

	result = vk->table.vkEndCommandBuffer(command_buffer);
	hard_assert_eq(result, VK_SUCCESS);
}


void
vk_record_all_commands(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	for(uint32_t i = 0; i < vk->image_count; ++i)
	{
		vk->frames[i].barrier = NULL;
	}

	vk_barrier_t* barrier = vk->barriers;
	vk_barrier_t* barrier_end = barrier + vk->buffering;

	while(barrier < barrier_end)
	{
		for(uint32_t i = 0; i < vk->image_count; ++i)
		{
			vk_record_commands(vk, barrier->command_buffers[i], &vk->frames[i]);
		}

		++barrier;
	}
}


void
vk_device_wait_idle(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	VkResult result = vk->table.vkDeviceWaitIdle(vk->device);
	hard_assert_eq(result, VK_SUCCESS);
}


void
vk_recreate_swapchain(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk_device_wait_idle(vk);

	vk_free_images(vk);
	vk_get_extent(vk);
	vk_init_images(vk);

	vk_free_barriers(vk);
	vk_free_framebuffers(vk);
	vk_init_swapchain(vk);
	vk_init_framebuffers(vk);
	vk_init_barriers(vk);

	vk_record_all_commands(vk);
}


typedef struct vk_present_data
{
	vulkan_t vk;
	VkPresentInfoKHR* present_info;
	VkResult* result;
}
vk_present_data_t;


void
vk_present(
	vk_present_data_t* data
	)
{
	*(data->result) = data->vk->table.vkQueuePresentKHR(data->vk->queue, data->present_info);
}


void
vk_draw(
	vulkan_t vk
	)
{
	VkResult result = vk->table.vkWaitForFences(vk->device, 1, &vk->barrier->fence, VK_TRUE, UINT64_MAX);
	hard_assert_eq(result, VK_SUCCESS);

	uint32_t image_idx;
	result = vk->table.vkAcquireNextImageKHR(vk->device, vk->swapchain,
		UINT64_MAX, vk->barrier->semaphore, VK_NULL_HANDLE, &image_idx);
	if(result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vk_recreate_swapchain(vk);
		return;
	}

	if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		hard_assert_log("%d\n", result);
		hard_assert_unreachable();
	}


	vk_frame_t* frame = vk->frames + image_idx;

	if(frame->barrier)
	{
		result = vk->table.vkWaitForFences(vk->device, 1, &frame->barrier->fence, VK_TRUE, UINT64_MAX);
		hard_assert_eq(result, VK_SUCCESS);
	}
	frame->barrier = vk->barrier;

	result = vk->table.vkResetFences(vk->device, 1, &vk->barrier->fence);
	hard_assert_eq(result, VK_SUCCESS);


	static uint64_t last_time = 0;
	uint64_t time = time_get();

	if(last_time)
	{
		vk->delta = (float)(time - last_time) / time_ms_to_ns(1);
		vk->fps = VK_FPS_DECAY_FACTOR * vk->fps + (1.0f - VK_FPS_DECAY_FACTOR) * (1000.0f / vk->delta);
	}
	last_time = time;


	vk->draw_data_count = 0;

	vulkan_draw_event_data_t event_data =
	{
		.vulkan = vk,
		.delta = vk->delta,
		.fps = vk->fps
	};
	event_target_fire(&vk->event_table.draw_target, &event_data);

	vk_copy_to_buffer(vk, &frame->instance_input_buffer, vk->draw_data, vk->draw_data_count);


	VkDrawIndirectCommand command =
	{
		.vertexCount = MACRO_ARRAY_LEN(vk_vertex_input),
		.instanceCount = vk->draw_data_count,
		.firstVertex = 0,
		.firstInstance = 0
	};

	vk_copy_to_buffer(vk, &frame->draw_count_buffer, &command, 1);


	VkPipelineStageFlags wait_stages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo submit_info =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &vk->barrier->semaphore,
		.pWaitDstStageMask = wait_stages,
		.commandBufferCount = 1,
		.pCommandBuffers = vk->barrier->command_buffers + image_idx,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &frame->semaphore
	};

	result = vk->table.vkQueueSubmit(vk->queue, 1, &submit_info, vk->barrier->fence);
	hard_assert_eq(result, VK_SUCCESS);

	VkPresentInfoKHR present_info =
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = NULL,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame->semaphore,
		.swapchainCount = 1,
		.pSwapchains = &vk->swapchain,
		.pImageIndices = &image_idx,
		.pResults = NULL
	};

	vk_present_data_t present_data =
	{
		.vk = vk,
		.present_info = &present_info,
		.result = &result
	};

	thread_data_t data =
	{
		.fn = (void*) vk_present,
		.data = &present_data
	};

	window_run_on_main_thread(vk->window, data, true);

	bool resized = atomic_load_acq(&vk->resized);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized)
	{
		if(resized)
		{
			atomic_store_rel(&vk->resized, false);
		}

		vk_recreate_swapchain(vk);
	}
	else
	{
		hard_assert_eq(result, VK_SUCCESS);
	}

	if(++vk->barrier >= vk->barriers + vk->buffering)
	{
		vk->barrier = vk->barriers;
	}
}


void
vk_thread_fn(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	while(atomic_load_acq(&vk->should_run))
	{
		sync_mtx_lock(&vk->draw_mtx);
		vk_draw(vk);
		sync_mtx_unlock(&vk->draw_mtx);
	}

	window_unref(vk->window);
}


void
vk_init_thread(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	atomic_init(&vk->should_run, true);

	thread_data_t thread_data =
	{
		.fn = (void*) vk_thread_fn,
		.data = vk
	};
	thread_init(&vk->thread, thread_data);
}


void
vk_free_thread(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	atomic_store_rel(&vk->should_run, false);

	thread_join(vk->thread);
	thread_free(&vk->thread);
}


void
vk_resize_fn(
	vulkan_t vk,
	window_resize_event_data_t* event_data
	)
{
	assert_not_null(vk);
	assert_not_null(event_data);

	atomic_store_rel(&vk->resized, true);

	sync_mtx_lock(&vk->resize_mtx);
		sync_cond_wake(&vk->resize_cond);
	sync_mtx_unlock(&vk->resize_mtx);
}


void
vk_init_fn(
	vulkan_t vk,
	window_init_event_data_t* event_data
	)
{
	assert_not_null(vk);
	assert_not_null(event_data);

	vk_init_settings(vk);
	vk_init_instance(vk);
	vk_init_surface(vk);
	vk_init_device(vk);
	vk_init_commands(vk);
	vk_init_images(vk);
	vk_init_render_pass(vk);
	vk_init_pipeline(vk);
	vk_init_swapchain(vk);
	vk_init_barriers(vk);
	vk_init_framebuffers(vk);
	vk_init_textures(vk);
	vk_init_vertex(vk);

	vk_record_all_commands(vk);
	vk_init_thread(vk);

	vulkan_init_event_data_t init_event_data =
	{
		.vulkan = vk
	};
	event_target_fire(&vk->event_table.init_target, &init_event_data);
}


void
vk_free(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	vk_free_thread(vk);

	vk_device_wait_idle(vk);

	vk_free_vertex(vk);
	vk_free_textures(vk);
	vk_free_framebuffers(vk);
	vk_free_barriers(vk);
	vk_free_swapchain(vk);
	vk_free_pipeline(vk);
	vk_free_render_pass(vk);
	vk_free_images(vk);
	vk_free_commands(vk);
	vk_free_device(vk);
	vk_free_surface(vk);
	vk_free_instance(vk);
	vk_free_settings(vk);
}


void
vk_free_fn(
	vulkan_t vk,
	window_free_event_data_t* event_data
	)
{
	assert_not_null(vk);

	vulkan_free_event_data_t free_event_data =
	{
		.vulkan = vk
	};
	event_target_fire(&vk->event_table.free_target, &free_event_data);

	vk_free(vk);

	event_target_free(&vk->event_table.draw_target);
	event_target_free(&vk->event_table.free_target);
	event_target_free(&vk->event_table.init_target);

	sync_cond_free(&vk->resize_cond);
	sync_mtx_free(&vk->resize_mtx);
	sync_mtx_free(&vk->draw_mtx);

	window_event_table_t* table = window_get_event_table(vk->window);
	event_target_del(&table->resize_target, vk->resize_listener);

	alloc_free(vk, 1);
}


void
vk_closing_fn(
	vulkan_t vk,
	window_closing_event_data_t* event_data
	)
{
	assert_not_null(vk);
	assert_not_null(event_data);

	atomic_store_rel(&vk->should_run, false);
}


vulkan_t
vulkan_init(
	window_t window,
	time_timers_t timers
	)
{
	assert_not_null(window);

	vulkan_t vk = alloc_calloc(vk, 1);
	assert_ptr(vk, 1);

	event_target_init(&vk->event_table.init_target);
	event_target_init(&vk->event_table.free_target);
	event_target_init(&vk->event_table.draw_target);

	vk->timers = timers;

	window_ref(window);

	vk->window = window;
	window_event_table_t* table = window_get_event_table(window);

	event_listener_data_t init_data =
	{
		.fn = (void*) vk_init_fn,
		.data = vk
	};
	event_target_once(&table->init_target, init_data);

	event_listener_data_t free_data =
	{
		.fn = (void*) vk_free_fn,
		.data = vk
	};
	event_target_once(&table->free_target, free_data);

	event_listener_data_t resize_data =
	{
		.fn = (void*) vk_resize_fn,
		.data = vk
	};
	vk->resize_listener = event_target_add(&table->resize_target, resize_data);

	event_listener_data_t closing_data =
	{
		.fn = (void*) vk_closing_fn,
		.data = vk
	};
	event_target_once(&table->closing_target, closing_data);

	sync_mtx_init(&vk->draw_mtx);
	sync_mtx_init(&vk->resize_mtx);
	sync_cond_init(&vk->resize_cond);
	atomic_store_rel(&vk->resized, false);

	return vk;
}


vulkan_event_table_t*
vulkan_get_event_table(
	vulkan_t vk
	)
{
	assert_not_null(vk);

	return &vk->event_table;
}


void
vulkan_add_draw_data(
	vulkan_t vk,
	const vulkan_draw_data_t* data
	)
{
	assert_not_null(vk);
	assert_not_null(data);

	assert_lt(vk->draw_data_count, VK_DRAW_DATA_BUFFER_SIZE);
	vk->draw_data[vk->draw_data_count++] = *data;
}


void
vulkan_set_buffering(
	vulkan_t vk,
	uint32_t buffering
	)
{
	assert_not_null(vk);
	assert_neq(buffering, 0);

	sync_mtx_lock(&vk->draw_mtx);

	buffering = MACRO_CLAMP(
		buffering,
		vk->surface_capabilities.minImageCount,
		vk->surface_capabilities.maxImageCount
		);

	if(buffering == vk->buffering)
	{
		sync_mtx_unlock(&vk->draw_mtx);
		return;
	}

	for(uint32_t i = 0; i < vk->image_count; ++i)
	{
		vk->frames[i].barrier = NULL;
	}

	vk_device_wait_idle(vk);

	vk_free_barriers(vk);
	vk->buffering = buffering;
	vk_init_barriers(vk);

	vk_record_all_commands(vk);

	sync_mtx_unlock(&vk->draw_mtx);
}
