﻿#pragma once
#include "VkRenderBackEnd.h"

#include <assert.h>
#include <vector>
#define GLFW_INCLUDE_NONE

#include "GLFW/glfw3.h"
#include "Utility/file/Tfile.h"
#include "Engine/Engine.h"
#include <EngineSrc\Math\Matrix44.h>
#include <EngineSrc\Mesh\VertexData.h>
#include <array>
#include <iostream>


#include "SOIL2/stb_image.h"
#include "vk/DeviceTextureVK.h"
#include "vk/DeviceShaderVK.h"
#include "vk/DeviceBufferVK.h"
#include "vk/DeviceRenderPassVK.h"
#include "Rendering/Renderer.h"
#include "Technique/Material.h"



#include "spirv_cross.hpp"
#include "spirv_glsl.hpp"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/Include/Types.h"
#include "glslang/public/ShaderLang.h"
#include "Mesh/Mesh.h"
#include "Mesh/InstancedMesh.h"
#include "2D/GUISystem.h"
#include "Texture/TextureMgr.h"
#include "Utility/log/Log.h"

#include "3D/Primitive/SpherePrimitive.h"
#include <EngineSrc\Scene\SceneMgr.h>
//#include "vk/DeviceShaderVK.h"
#define ENABLE_DEBUG_LAYERS 1
namespace tzw
{


#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
const bool enableValidationLayers = true;
const int MAX_FRAMES_IN_FLIGHT = 1;
static void initVk()
{
    
}


struct ItemUniform
{
    alignas(16) Matrix44 wvp;
    alignas(16) Matrix44 wv;
    alignas(16) Matrix44 world;
    alignas(16) Matrix44 view;
    alignas(16) Matrix44 projection;
};

static FrameBufferVK offScreenFrameBuf;
std::vector<const char*> getRequiredExtensions() {
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

PFN_vkCreateDebugReportCallbackEXT my_vkCreateDebugReportCallbackEXT = NULL; 

FILE * vulkanValidationFile = nullptr;
VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData)
{
    if(!vulkanValidationFile){
        vulkanValidationFile = fopen("./vulkanLog.txt", "w");
    }
    printf("validation Layer %s\n", pMessage);
    fprintf(vulkanValidationFile, "validation Layer %s\n", pMessage);
    return VK_FALSE;
}

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
};

VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName)
{
    int codeSize = 0;
    auto data = Tfile::shared()->getData(pFileName, false);
    char* pShaderCode = (char *)data.getBytes();//ReadBinaryFile(pFileName, codeSize);
    codeSize = data.getSize();
    assert(pShaderCode);


    


    
    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = codeSize;
    shaderCreateInfo.pCode = (const uint32_t*)pShaderCode;
    
    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, NULL, &shaderModule);
    CHECK_VULKAN_ERROR("vkCreateShaderModule error %d\n", res);
    printf("Created shader %s\n", pFileName);
    return shaderModule;    
}

bool VKRenderBackEnd::memory_type_from_properties(uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) 
{
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback2(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}
static VkSurfaceKHR createVKSurface(VkInstance* instance, GLFWwindow * window)
{
    VkSurfaceKHR surface;
    VkResult err = glfwCreateWindowSurface(*instance, window, NULL, &surface);
    if (err)
    {
        printf("aaaaa%d\n",err);
        abort();
        // Window surface creation failed
    }
    return surface;
}
	VKRenderBackEnd::VKRenderBackEnd()
	{
	m_imguiPipeline = nullptr;
	}
    void VKRenderBackEnd::updateItemDescriptor(VkDescriptorSet itemDescSet, Material* mat, size_t m_offset, size_t bufferRange)
    {
       auto & varList = mat->getVarList();
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        //update descriptor
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getBuffer();
        bufferInfo.offset = m_offset;
        bufferInfo.range = bufferRange;

        VkWriteDescriptorSet writeSet{};
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.pNext = nullptr;
        writeSet.dstSet = itemDescSet;
        writeSet.dstBinding = 0;
        writeSet.dstArrayElement = 0;
        writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeSet.descriptorCount = 1;
        writeSet.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), 1, &writeSet, 0, nullptr);
        //descriptorWrites.emplace_back(writeSet);
        auto shader = static_cast<DeviceShaderVK * >(mat->getProgram()->getDeviceShader());
        std::vector<VkDescriptorImageInfo> imageInfoList;
        for(auto &i : varList)
        {
            TechniqueVar* var = &i.second;
            if(!shader->hasLocationInfo(i.first)){continue;}
            switch(var->type)
            {
            
                case TechniqueVar::Type::Texture:
                {
                    auto tex = var->data.rawData.texInfo.tex;
					if(!tex)
					{
                        printf("abort%s %p\n", i.first.c_str(), mat);
						abort();
					}
                    auto locationInfo = shader->getLocationInfo(i.first);
                    if(locationInfo.set != 0) continue;
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = static_cast<DeviceTextureVK *>(tex->getTextureId())->getImageView();
                    imageInfo.sampler = static_cast<DeviceTextureVK *>(tex->getTextureId())->getSampler();
                    imageInfoList.emplace_back(imageInfo);
                    
                    VkWriteDescriptorSet texWriteSet{};
                    texWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    texWriteSet.dstSet = itemDescSet;
                    texWriteSet.dstBinding = locationInfo.binding;
                    texWriteSet.dstArrayElement = 0;
                    texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    texWriteSet.descriptorCount = 1;
                    texWriteSet.pImageInfo = &imageInfo;//(imageInfoList.data() + imageInfoList.size() - 1);
					vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), 1, &texWriteSet, 0, nullptr);
                    //descriptorWrites.emplace_back(texWriteSet);
                }
            }
        }
        //vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
    void VKRenderBackEnd::VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps)
    {
        unsigned NumExt = 0;
        VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, NULL);
        CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res);
    
        printf("Found %d extensions\n", NumExt);
    
        ExtProps.resize(NumExt);

        res = vkEnumerateInstanceExtensionProperties(NULL, &NumExt, &ExtProps[0]);
        CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res);        
    
        for (unsigned i = 0 ; i < NumExt ; i++) {
            printf("Instance extension %d - %s\n", i, ExtProps[i].extensionName);
        }
    }
    void VKRenderBackEnd::CreateInstance()
    {
VkApplicationInfo appInfo = {};       
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "TinyBuilder";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char* pInstExt[] = {
#ifdef ENABLE_DEBUG_LAYERS
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif        
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32    
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else    
        VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif            
    };
    
#ifdef ENABLE_DEBUG_LAYERS    
    const char* pInstLayers[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };
#endif    
    
    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
#ifdef ENABLE_DEBUG_LAYERS    
    instInfo.enabledLayerCount = ARRAY_SIZE_IN_ELEMENTS(pInstLayers);
    instInfo.ppEnabledLayerNames = pInstLayers;
#endif    
    instInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pInstExt);
    instInfo.ppEnabledExtensionNames = pInstExt;         

    VkResult res = vkCreateInstance(&instInfo, NULL, &m_inst);
    CHECK_VULKAN_ERROR("vkCreateInstance %d\n", res);
#ifdef ENABLE_DEBUG_LAYERS
    // Get the address to the vkCreateDebugReportCallbackEXT function
    my_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_inst, "vkCreateDebugReportCallbackEXT"));
    
    // Register the debug callback
    VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
    callbackCreateInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    callbackCreateInfo.pNext       = NULL;
    callbackCreateInfo.flags       = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | 
								     VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                     VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    callbackCreateInfo.pfnCallback = &MyDebugReportCallback;
    callbackCreateInfo.pUserData   = NULL;

    VkDebugReportCallbackEXT callback;
    res = my_vkCreateDebugReportCallbackEXT(m_inst, &callbackCreateInfo, NULL, &callback);
    CHECK_VULKAN_ERROR("my_vkCreateDebugReportCallbackEXT error %d\n", res);
#endif
    }
    void VKRenderBackEnd::VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& Surface, VulkanPhysicalDevices& PhysDevices)
    {
        unsigned NumDevices = 0;
    
        VkResult res = vkEnumeratePhysicalDevices(inst, &NumDevices, NULL);
        printf("vkEnumeratePhysicalDevices error %d\n", res);
        printf("Num physical devices %d\n", NumDevices);

        PhysDevices.m_devices.resize(NumDevices);
        PhysDevices.m_devProps.resize(NumDevices);
        PhysDevices.m_qFamilyProps.resize(NumDevices);
        PhysDevices.m_qSupportsPresent.resize(NumDevices);
        PhysDevices.m_surfaceFormats.resize(NumDevices);
        PhysDevices.m_surfaceCaps.resize(NumDevices);

        res = vkEnumeratePhysicalDevices(inst, &NumDevices, &PhysDevices.m_devices[0]);
        CHECK_VULKAN_ERROR("vkEnumeratePhysicalDevices error %d\n", res);

        for (unsigned i = 0 ; i < NumDevices ; i++) 
        {
                const VkPhysicalDevice& PhysDev = PhysDevices.m_devices[i];
                vkGetPhysicalDeviceProperties(PhysDev, &PhysDevices.m_devProps[i]);
                printf("Device name: %s\n", PhysDevices.m_devProps[i].deviceName);
                uint32_t apiVer = PhysDevices.m_devProps[i].apiVersion;
                printf("    API version: %d.%d.%d\n", VK_VERSION_MAJOR(apiVer),
                                                  VK_VERSION_MINOR(apiVer),
                                                  VK_VERSION_PATCH(apiVer));
                 unsigned NumQFamily = 0;         
        
                vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, NULL);
    
                printf("    Num of family queues: %d\n", NumQFamily);

                PhysDevices.m_qFamilyProps[i].resize(NumQFamily);
                PhysDevices.m_qSupportsPresent[i].resize(NumQFamily);

                vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, &(PhysDevices.m_qFamilyProps[i][0]));
            for (unsigned q = 0 ; q < NumQFamily ; q++) {
                res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, Surface, &(PhysDevices.m_qSupportsPresent[i][q]));
                CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceSupportKHR error %d\n", res);
            }
            unsigned NumFormats = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, NULL);
            assert(NumFormats > 0);
        
            PhysDevices.m_surfaceFormats[i].resize(NumFormats);
        
            res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, Surface, &NumFormats, &(PhysDevices.m_surfaceFormats[i][0]));
            CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR error %d\n", res);
    
            for (unsigned j = 0 ; j < NumFormats ; j++) {
                const VkSurfaceFormatKHR& SurfaceFormat = PhysDevices.m_surfaceFormats[i][j];
                printf("    Format %d color space %d\n", SurfaceFormat.format , SurfaceFormat.colorSpace);
            }
            res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, Surface, &(PhysDevices.m_surfaceCaps[i]));
            CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR error %d\n", res);
       
            //VulkanPrintImageUsageFlags(PhysDevices.m_surfaceCaps[i].supportedUsageFlags);
        }
    }


    VkInstance& VKRenderBackEnd::getVKInstance()
    {
        return m_inst;
        // TODO: 在此处插入 return 语句
    }

    void VKRenderBackEnd::setVKSurface(const VkSurfaceKHR& surface)
    {
        m_surface = surface;
    }

    VkSurfaceKHR& VKRenderBackEnd::getVKSurface()
    {
        // TODO: 在此处插入 return 语句
        return m_surface;
    }

    void VKRenderBackEnd::SelectPhysicalDevice()
    {
        for (unsigned i = 0 ; i < m_physicsDevices.m_devices.size() ; i++) {
                
            for (unsigned j = 0 ; j < m_physicsDevices.m_qFamilyProps[i].size() ; j++) {
                VkQueueFamilyProperties& QFamilyProp = m_physicsDevices.m_qFamilyProps[i][j];
            
                printf("Family %d Num queues: %d\n", j, QFamilyProp.queueCount);
                VkQueueFlags flags = QFamilyProp.queueFlags;
                printf("    GFX %s, Compute %s, Transfer %s, Sparse binding %s\n",
                        (flags & VK_QUEUE_GRAPHICS_BIT) ? "Yes" : "No",
                        (flags & VK_QUEUE_COMPUTE_BIT) ? "Yes" : "No",
                        (flags & VK_QUEUE_TRANSFER_BIT) ? "Yes" : "No",
                        (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? "Yes" : "No");
            
                if (flags & VK_QUEUE_GRAPHICS_BIT) {
                    if (!m_physicsDevices.m_qSupportsPresent[i][j]) {
                        printf("Present is not supported\n");
                        continue;
                    }

                    m_gfxDevIndex = i;
                    m_gfxQueueFamily = j;
                    printf("Using GFX device %d and queue family %d\n", m_gfxDevIndex, m_gfxQueueFamily);
                    break;
                }
            }
        }
    
        if (m_gfxDevIndex == -1) {
            printf("No GFX device found!\n");
            assert(0);
        }    
    }

    void VKRenderBackEnd::CreateLogicalDevice()
    {
        float qPriorities = 1.0f;
        VkDeviceQueueCreateInfo qInfo = {};
        qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qInfo.queueFamilyIndex = m_gfxQueueFamily;
        qInfo.queueCount = 1;
        qInfo.pQueuePriorities = &qPriorities;
    
        const char* pDevExt[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    
        VkDeviceCreateInfo devInfo = {};
        devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        devInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pDevExt);
        devInfo.ppEnabledExtensionNames = pDevExt;
        devInfo.queueCreateInfoCount = 1;
        devInfo.pQueueCreateInfos = &qInfo;
       
#if ENABLE_DEBUG_LAYERS
        devInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        devInfo.ppEnabledLayerNames = validationLayers.data();
#else
        devInfo.enabledLayerCount = 0;
#endif

        VkResult res = vkCreateDevice(GetPhysDevice(), &devInfo, NULL, &m_device);

        CHECK_VULKAN_ERROR("vkCreateDevice error %d\n", res);
   
        printf("Device created\n");
    }
    void VKRenderBackEnd::createSwapChain()
    {
        const VkSurfaceCapabilitiesKHR& SurfaceCaps = GetSurfaceCaps();
        assert(SurfaceCaps.currentExtent.width != -1);
                   
        unsigned NumImages = 2;

        assert(NumImages >= SurfaceCaps.minImageCount);
        assert(NumImages <= SurfaceCaps.maxImageCount);
      
        VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};
    
        SwapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        SwapChainCreateInfo.surface          = m_surface;
        SwapChainCreateInfo.minImageCount    = NumImages;
        SwapChainCreateInfo.imageFormat      = GetSurfaceFormat().format;
        SwapChainCreateInfo.imageColorSpace  = GetSurfaceFormat().colorSpace;
        SwapChainCreateInfo.imageExtent      = SurfaceCaps.currentExtent;
        SwapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        SwapChainCreateInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        SwapChainCreateInfo.imageArrayLayers = 1;
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        SwapChainCreateInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
        SwapChainCreateInfo.clipped          = true;
        SwapChainCreateInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    
        VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChainKHR);
        CHECK_VULKAN_ERROR("vkCreateSwapchainKHR error %d\n", res);    

        printf("Swap chain created\n");
    
        unsigned NumSwapChainImages = 0;
        res = vkGetSwapchainImagesKHR(m_device, m_swapChainKHR, &NumSwapChainImages, NULL);
        CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
        assert(NumImages == NumSwapChainImages);
    
        printf("Number of images %d\n", NumSwapChainImages);

        m_images.resize(NumSwapChainImages);
        m_views.resize(NumSwapChainImages);
        m_cmdBufs.resize(NumSwapChainImages);
    
        res = vkGetSwapchainImagesKHR(m_device, m_swapChainKHR, &NumSwapChainImages, &(m_images[0]));
        CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);



        vkGetPhysicalDeviceMemoryProperties(GetPhysDevice(), &memory_properties);
    }
    void VKRenderBackEnd::CreateCommandBuffer()
    {
            VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            cmdPoolCreateInfo.queueFamilyIndex = m_gfxQueueFamily;
    
            VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, NULL, &m_cmdBufPool);    
            CHECK_VULKAN_ERROR("vkCreateCommandPool error %d\n", res);
    
            printf("Command buffer pool created\n");
    
            VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
            cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocInfo.commandPool = m_cmdBufPool;
            cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocInfo.commandBufferCount = m_images.size();
    
            res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, &m_cmdBufs[0]);            
            CHECK_VULKAN_ERROR("vkAllocateCommandBuffers error %d\n", res);
    
            printf("Created command buffers\n");
    }

    void VKRenderBackEnd::RecordCommandBuffers()
        {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    
        VkClearColorValue clearColor = { 164.0f/256.0f, 30.0f/256.0f, 34.0f/256.0f, 0.0f };
        VkClearValue clearValue = {};
        clearValue.color = clearColor;
    
        VkImageSubresourceRange imageRange = {};
        imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRange.levelCount = 1;
        imageRange.layerCount = 1;


		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
	
        auto screenSize = Engine::shared()->winSize();
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;   
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent.width = screenSize.x;
        renderPassInfo.renderArea.extent.height = screenSize.y;
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        for (unsigned i = 0 ; i < m_cmdBufs.size() ; i++) {            
            VkResult res = vkBeginCommandBuffer(m_cmdBufs[i], &beginInfo);
            CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
            renderPassInfo.framebuffer = m_fbs[i];

            vkCmdBeginRenderPass(m_cmdBufs[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
            VkBuffer vertexBuffers[] = {m_vertexBuffer->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(m_cmdBufs[i], 0, 1, vertexBuffers, offsets);
            
            vkCmdBindIndexBuffer(m_cmdBufs[i], m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdBindDescriptorSets(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
            vkCmdDrawIndexed(m_cmdBufs[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            vkCmdEndRenderPass(m_cmdBufs[i]);
               
            res = vkEndCommandBuffer(m_cmdBufs[i]);
            CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
        }
    
        printf("Command buffers recorded\n");    
    }

    void VKRenderBackEnd::CreateRenderPass()
    {
        VkAttachmentReference attachRef = {};
        attachRef.attachment = 0;
        attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &attachRef;
		subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription attachDesc = {};    
        attachDesc.format = GetSurfaceFormat().format;
        attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentDescription> attachmentDescList = {attachDesc, depthAttachment};
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachmentDescList.size();
        renderPassCreateInfo.pAttachments = attachmentDescList.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;
        
        VkResult res = vkCreateRenderPass(m_device, &renderPassCreateInfo, NULL, &m_renderPass);
        CHECK_VULKAN_ERROR("vkCreateRenderPass error %d\n", res);

        printf("Created a render pass\n");
    }
    void VKRenderBackEnd::CreateTextureToScreenRenderPass()
    {
        VkAttachmentReference attachRef = {};
        attachRef.attachment = 0;
        attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &attachRef;
		subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription attachDesc = {};    
        attachDesc.format = GetSurfaceFormat().format;
        attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkAttachmentDescription> attachmentDescList = {attachDesc, depthAttachment};
        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachmentDescList.size();
        renderPassCreateInfo.pAttachments = attachmentDescList.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;
        
        VkResult res = vkCreateRenderPass(m_device, &renderPassCreateInfo, NULL, &m_textureToScreenRenderPass);
        CHECK_VULKAN_ERROR("vkCreateRenderPass error %d\n", res);

        printf("Created a render pass\n");
    }

	tzw::vec3 pointOnSurface(float u, float v)
	{
        float m_radius = 6360000.0f;
		return vec3(cos(u) * sin(v) * m_radius, cos(v) * m_radius, sin(u) * sin(v) * m_radius);
	}
    void VKRenderBackEnd::CreateDerrferredRenderPass()
    {



		m_sphere = new Mesh();
		float PI = 3.1416;
		float startU=0;
		float startV=0;
		float endU=PI*2;
		float endV=PI;
		float stepU=(endU-startU)/24; // step size between U-points on the grid
		float stepV=(endV-startV)/24; // step size between V-points on the grid
		for(int i=0;i<24;i++){ // U-points
			for(int j=0;j<24;j++){ // V-points
				float u=i*stepU+startU;
					float v=j*stepV+startV;
					float un=(i+1==24) ? endU : (i+1)*stepU+startU;
					float vn=(j+1==24) ? endV : (j+1)*stepV+startV;
					// Find the four points of the grid
					// square by evaluating the parametric
					// surface function
					vec3 p0=pointOnSurface(u, v);
					vec3 p1=pointOnSurface(u, vn);
					vec3 p2=pointOnSurface(un, v);
					vec3 p3=pointOnSurface(un, vn);
					// NOTE: For spheres, the normal is just the normalized
					// version of each vertex point; this generally won't be the case for
					// other parametric surfaces.
					// Output the first triangle of this grid square
						
					m_sphere->addVertex(VertexData(p0, p0.normalized(), vec2(u, 1.0 - v)));
					m_sphere->addVertex(VertexData(p2, p2.normalized(), vec2(un, 1.0 - v)));
					m_sphere->addVertex(VertexData(p1, p1.normalized(), vec2(u, 1.0 - vn)));
					m_sphere->addIndex(m_sphere->getIndicesSize());
					m_sphere->addIndex(m_sphere->getIndicesSize());
					m_sphere->addIndex(m_sphere->getIndicesSize());


					m_sphere->addVertex(VertexData(p3, p3.normalized(), vec2(un, 1.0 - vn)));
					m_sphere->addVertex(VertexData(p1, p1.normalized(), vec2(u, 1.0 - vn)));
					m_sphere->addVertex(VertexData(p2, p2.normalized(), vec2(un, 1.0 - v)));
					m_sphere->addIndex(m_sphere->getIndicesSize());
					m_sphere->addIndex(m_sphere->getIndicesSize());
					m_sphere->addIndex(m_sphere->getIndicesSize());
			}
		}
		m_sphere->finish();


        auto size = Engine::shared()->winSize();
        auto gBufferRenderPass = new DeviceRenderPassVK(4, DeviceRenderPassVK::OpType::CLEAR_AND_STORE, ImageFormat::R8G8B8A8_S);
        auto gBuffer = new DeviceFrameBufferVK(size.x, size.y, gBufferRenderPass);
        m_gPassStage = new DeviceRenderStageVK(gBufferRenderPass, gBuffer);


        auto deferredLightingPass = new DeviceRenderPassVK(1, DeviceRenderPassVK::OpType::CLEAR_AND_STORE, ImageFormat::R16G16B16A16_SFLOAT);
        auto deferredLightingBuffer= new DeviceFrameBufferVK(size.x, size.y, deferredLightingPass);
        m_DeferredLightingStage = new DeviceRenderStageVK(deferredLightingPass, deferredLightingBuffer);
       
        DeviceVertexInput imguiVertexInput;
        imguiVertexInput.stride = sizeof(VertexData);
        imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_pos)});
        imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_color)});
        imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32_SFLOAT, offsetof(VertexData, m_texCoord)});
        Material * mat = new Material();
        mat->loadFromTemplate("DirectLight");

        DeviceVertexInput emptyInstancingInput;
        m_dirLightingPassPiepeline = new DevicePipelineVK(mat, m_DeferredLightingStage->getRenderPass()->getRenderPass(), imguiVertexInput, false, emptyInstancingInput, 4);

        auto skyPass = new DeviceRenderPassVK(1, DeviceRenderPassVK::OpType::LOAD_AND_STORE, ImageFormat::R16G16B16A16_SFLOAT);
        m_skyStage = new DeviceRenderStageVK(skyPass, m_DeferredLightingStage->getFrameBuffer());


        Material * matSkyPass = new Material();
        matSkyPass->loadFromTemplate("Sky");
        m_skyPassPipeLine = new DevicePipelineVK(matSkyPass, m_skyStage->getRenderPass()->getRenderPass(), imguiVertexInput, false, emptyInstancingInput);

        Material * matTextureToScreen = new Material();
        matTextureToScreen->loadFromTemplate("TextureToScreen");
        m_textureToScreenPipeline = new DevicePipelineVK(matTextureToScreen, m_textureToScreenRenderPass, imguiVertexInput, false, emptyInstancingInput);

        VertexData vertices[] = {
            // Vertex data for face 0
            VertexData(vec3(-1.0f, -1.0f,  1.0f), vec2(0.0f, 0.0f)),  // v0
            VertexData(vec3( 1.0f, -1.0f,  1.0f), vec2(1.f, 0.0f)), // v1
            VertexData(vec3(-1.0f,  1.0f,  1.0f), vec2(0.0f, 1.f)),  // v2
            VertexData(vec3( 1.0f,  1.0f,  1.0f), vec2(1.f, 1.f)), // v3
        };
        auto vbuffer = createBuffer_imp();
        vbuffer->init(DeviceBufferType::Vertex);

        vbuffer->allocate(vertices, sizeof(vertices[0]) * ARRAY_SIZE_IN_ELEMENTS(vertices));
        m_quadVertexBuffer = static_cast<DeviceBufferVK *>(vbuffer);


        indices = {
         0,  1,  2,  1,  3,  2,

		};
        auto ibuffer = createBuffer_imp();
        ibuffer->init(DeviceBufferType::Index);

        ibuffer->allocate(indices.data(), sizeof(indices[0]) * indices.size());
        m_quadIndexBuffer = static_cast<DeviceBufferVK *>(ibuffer);
    }
    void VKRenderBackEnd::createDeferredFrameBuffer()
    {

    }
    void VKRenderBackEnd::createAttachment(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachmentVK* attachment)
    {
    }
    void VKRenderBackEnd::CreateFramebuffer()
    {
        printf("try create Frame Buffer\n");
        m_fbs.resize(m_images.size());
    
        VkResult res;
            
        for (unsigned i = 0 ; i < m_images.size() ; i++) {
            VkImageViewCreateInfo ViewCreateInfo = {};
            ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ViewCreateInfo.image = m_images[i];
            ViewCreateInfo.format = GetSurfaceFormat().format;
            ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ViewCreateInfo.subresourceRange.baseMipLevel = 0;
            ViewCreateInfo.subresourceRange.levelCount = 1;
            ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            ViewCreateInfo.subresourceRange.layerCount = 1;    

            res = vkCreateImageView(m_device, &ViewCreateInfo, NULL, &m_views[i]);
            CHECK_VULKAN_ERROR("vkCreateImageView error %d\n", res);
			std::array<VkImageView, 2> attachments = {
		    m_views[i], //color buffer of swap chain
		    depthImageView, //the depth buffer
			};
            auto screenSize = Engine::shared()->winSize();
            VkFramebufferCreateInfo fbCreateInfo = {};
            fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbCreateInfo.renderPass = m_renderPass;
            fbCreateInfo.attachmentCount = attachments.size();
            fbCreateInfo.pAttachments = attachments.data();
            fbCreateInfo.width = screenSize.x;
            fbCreateInfo.height = screenSize.y;
            fbCreateInfo.layers = 1;

            res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &m_fbs[i]);
            CHECK_VULKAN_ERROR("vkCreateFramebuffer error %d\n", res);
        }
   
        printf("Frame buffers created\n");
    }
    void VKRenderBackEnd::CreateShaders()
    {
        m_vsModule = VulkanCreateShaderModule(m_device, "VulkanShaders/vs.spv");
        assert(m_vsModule);

        m_fsModule = VulkanCreateShaderModule(m_device, "VulkanShaders/fs.spv");
        assert(m_fsModule);

        m_shader = new DeviceShaderVK();
        m_shader->create();
        tzw::Data data = tzw::Tfile::shared()->getData("VulkanShaders/vulkanTest_v.glsl",false);
        m_shader->addShader((const unsigned char *)data.getBytes(),data.getSize(),DeviceShaderType::VertexShader,(const unsigned char *)"vulkanTest_v.glsl");

        tzw::Data data2 = tzw::Tfile::shared()->getData("VulkanShaders/vulkanTest_f.glsl",false);
        m_shader->addShader((const unsigned char *)data2.getBytes(),data2.getSize(),DeviceShaderType::FragmentShader,(const unsigned char *)"vulkanTest_f.glsl");

        //shader 创建完毕，根据shader内容构建描述符集布局
        m_shader->finish();
    }
    void VKRenderBackEnd::CreatePipeline()
    {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
    
        shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageCreateInfo[0].module = m_shader->getVsModule();
        shaderStageCreateInfo[0].pName = "main";
        shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageCreateInfo[1].module = m_shader->getFsModule();
        shaderStageCreateInfo[1].pName = "main";   
	    
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        //create vertex input
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDecsriptionList;
        CreateVertexBufferDescription(attributeDecsriptionList);
		


        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDecsriptionList.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDecsriptionList.data();

        VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
        pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        auto screenSize = Engine::shared()->winSize();
        VkViewport vp = {};
        vp.x = 0.0f;
        vp.y = 0.0f;
        vp.width  = (float)screenSize.x;
        vp.height = (float)screenSize.y;
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;
        
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = VkExtent2D{(uint32_t)screenSize.x, (uint32_t)screenSize.y};

        VkPipelineViewportStateCreateInfo vpCreateInfo = {};
        vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vpCreateInfo.viewportCount = 1;
        vpCreateInfo.pViewports = &vp;
        vpCreateInfo.scissorCount = 1;
        vpCreateInfo.pScissors = &scissor;


        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
	
        VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
        rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rastCreateInfo.lineWidth = 1.0f;
    
        VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
        pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMSCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineMSCreateInfo.flags = 0;

        VkPipelineColorBlendAttachmentState blendAttachState = {};
        blendAttachState.colorWriteMask = 0xf;
    
        VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
        blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        blendCreateInfo.attachmentCount = 1;
        blendCreateInfo.pAttachments = &blendAttachState;
 

        //pipeline layout
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            abort();
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = ARRAY_SIZE_IN_ELEMENTS(shaderStageCreateInfo);
        pipelineInfo.pStages = &shaderStageCreateInfo[0];
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
        pipelineInfo.pViewportState = &vpCreateInfo;
        pipelineInfo.pRasterizationState = &rastCreateInfo;
        pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
        pipelineInfo.pColorBlendState = &blendCreateInfo;
		pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.basePipelineIndex = -1;
    
        VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &m_pipeline);
        //CHECK_VULKAN_ERROR("vkCreateGraphicsPipelines error %d\n", res);
        if(res){
            printf("fuck %d", res);
            abort();
        }
    
        printf("Graphics pipeline created\n");
    }
    void VKRenderBackEnd::CreateUiniform()
    {
        VkDeviceSize bufferSize = sizeof(Matrix44);

        uniformBuffers.resize(m_images.size());
        uniformBuffersMemory.resize(m_images.size());

        for (size_t i = 0; i < m_images.size(); i++) 
        {
            createVKBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        }
    }
    void VKRenderBackEnd::CreateVertexBufferDescription(std::vector<VkVertexInputAttributeDescription> & attributeDescriptions)
    {

        attributeDescriptions.resize(3);

		//local position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexData, m_pos);


		//color
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexData, m_color);
	
		//uv
	    attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexData, m_texCoord);

    }
    void VKRenderBackEnd::CreateVertexBuffer()
    {
	vec4 m_color = vec4(1, 1, 1, 1);
	vec4 m_color1 = vec4(0, 1, 0, 1);
	vec4 m_color2 = vec4(0, 0, 1, 1);
	vec4 m_color3 = vec4(1, 0, 0, 1);
	vec4 m_color4 = vec4(1, 0, 1, 1);
	vec4 m_color5 = vec4(1, 1, 0, 1);

	float halfWidth = 0.5;
	float halfDepth = 0.5;
	float halfHeight = 0.5;

	
    VertexData vertices[] = {
        // Vertex data for face 0
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2(0.0f, 0.0f), m_color),  // v0
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2(1.f, 0.0f), m_color), // v1
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(0.0f, 1.f), m_color),  // v2
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(1.f, 1.f), m_color), // v3

        // Vertex data for face 1
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2( 0.0f, 0.0f), m_color1), // v4
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(1.f, 0.0f), m_color1), // v5
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(0.0f, 1.0f), m_color1),  // v6
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(1.f, 1.0f), m_color1), // v7

        // Vertex data for face 2
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(0.0f, 0.0f), m_color2), // v8
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(1.0f, 0.0f), m_color2),  // v9
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(0.0f, 1.0f), m_color2), // v10
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(1.0f, 1.0f), m_color2),  // v11

        // Vertex data for face 3
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(0.0f, 0.0f), m_color3), // v12
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2(1.0f, 0.0f), m_color3),  // v13
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(0.0f, 1.f), m_color3), // v14
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(1.0f, 1.f), m_color3),  // v15

        // Vertex data for face 4
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(0.f, 0.0f), m_color4), // v16
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight, -1.0f * halfDepth), vec2(1.f, 0.0f), m_color4), // v17
        VertexData(vec3(-1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2(0.0f, 1.0f), m_color4), // v18
        VertexData(vec3( 1.0f *halfWidth, -1.0f * halfHeight,  1.0f * halfDepth), vec2(1.0f, 1.0f), m_color4), // v19

        // Vertex data for face 5
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(0.0f, 0.0f), m_color5), // v20
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight,  1.0f * halfDepth), vec2(1.0f, 0.0f), m_color5), // v21
        VertexData(vec3(-1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(0.0f, 1.0f), m_color5), // v22
        VertexData(vec3( 1.0f *halfWidth,  1.0f * halfHeight, -1.0f * halfDepth), vec2(1.0f, 1.0f), m_color5)  // v23
    };
	
        auto buffer = createBuffer_imp();
        buffer->init(DeviceBufferType::Vertex);

        buffer->allocate(vertices, sizeof(vertices[0]) * ARRAY_SIZE_IN_ELEMENTS(vertices));
        m_vertexBuffer = static_cast<DeviceBufferVK *>(buffer);
    }

    void VKRenderBackEnd::createDepthResources()
	{
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;//findDepthFormat();
		auto screenSize = Engine::shared()->winSize();
        createImage(screenSize.x, screenSize.y, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void VKRenderBackEnd::createCommandBufferForGeneral(int size)
    {
        m_generalCmdBuff.resize(m_images.size());
        for(int i = 0;i <m_images.size(); i++)
        {
            m_generalCmdBuff[i].resize(size);
            VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
            cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdBufAllocInfo.commandPool = m_generalCmdBufPool;
            cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmdBufAllocInfo.commandBufferCount = size;
    
            auto res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, m_generalCmdBuff[i].data());
            if(res){
                abort();
            }
        }

    }

    void VKRenderBackEnd::createCommandBufferPoolForGeneral()
    {
        VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolCreateInfo.queueFamilyIndex = m_gfxQueueFamily;
        VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, NULL, &m_generalCmdBufPool);    
        CHECK_VULKAN_ERROR("vkCreateCommandPool error %d\n", res);
    }

	
    void VKRenderBackEnd::updateUniformBuffer(uint32_t currentImage)
    {
        static float angle = 0.0;
        auto screenSize = Engine::shared()->winSize();
        float aspect = screenSize.x / screenSize.y;
        Matrix44 proj;
        proj.perspective(45, aspect, 0.1, 100.f);

        Quaternion q;
        q.fromAxisAngle(vec3(1, 1, 0).normalized(), angle);
        angle += 0.5;
        Matrix44 model;
        model.setRotation(q);
        model.setTranslate(vec3(0, 0, -5));
        void* data;
        vkMapMemory(m_device, uniformBuffersMemory[currentImage], 0, sizeof(Matrix44), 0, &data);
            memcpy(data, &(proj * model), sizeof(Matrix44));
        vkUnmapMemory(m_device, uniformBuffersMemory[currentImage]);
    }
    void VKRenderBackEnd::CreateIndexBuffer()
    {

        indices = {
         0,  1,  2,  1,  3,  2,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  5,  6,  5,  7,  6, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  9,  10, 9, 11, 10, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 13, 14, 13, 15, 14, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 17, 18, 17, 19, 18, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 21, 22, 21, 23, 22,      // Face 5 - triangle strip (v20, v21, v22, v23)
		};

	/*
		indices = {
		    0, 1, 2, 2, 3, 0,
		    4, 5, 6, 6, 7, 4
		};
	*/

        auto buffer = createBuffer_imp();
        buffer->init(DeviceBufferType::Index);

        buffer->allocate(indices.data(), sizeof(indices[0]) * indices.size());
        m_indexBuffer = static_cast<DeviceBufferVK *>(buffer);
    }

    void VKRenderBackEnd::CreateDescriptorSetLayout()
    {
        descriptorSetLayout = m_shader->getDescriptorSetLayOut();
        return;
    }
    void VKRenderBackEnd::createDescriptorPool()
    {
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 256;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 256;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 512;
        if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    void VKRenderBackEnd::createDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(m_images.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_images.size());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(m_images.size());
        if (vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            abort();
        }
        printf("createDescriptorSets 111 %p %p\n",descriptorSets[0], descriptorSets[1]);

        for (size_t i = 0; i < m_images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(Matrix44);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_myTexture->getImageView();
            imageInfo.sampler = textureSampler;

        	
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
    }
    void VKRenderBackEnd::createVKBuffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto res = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
        assert(!res);
        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = 0;
        alloc_info.allocationSize = memRequirements.size;
        bool pass = memory_type_from_properties(memRequirements.memoryTypeBits,
                                           properties,
                                           &alloc_info.memoryTypeIndex);
        res = vkAllocateMemory(m_device, &alloc_info, NULL,
                               &(bufferMemory));
        assert(!res);
        res = vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
        assert(!res);

    }

    DeviceMemoryPoolVK* VKRenderBackEnd::getMemoryPool()
    {
        return m_memoryPool;
    }

    VkFormat VKRenderBackEnd::getFormat(ImageFormat imageFormat)
    {
        switch(imageFormat){
        case ImageFormat::R8:
            return VK_FORMAT_R8_UNORM;
        case ImageFormat::R8G8:
            return VK_FORMAT_R8G8_UNORM;
        case ImageFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        case ImageFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case ImageFormat::R8G8B8A8_S:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case ImageFormat::R16G16B16A16:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case ImageFormat::D24_S8:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case ImageFormat::R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        }
        return VK_FORMAT_R8G8B8A8_UNORM;
    }

void VKRenderBackEnd::createTextureImage()
{
    m_myTexture = new DeviceTextureVK("logo.png");
}

VkCommandBuffer VKRenderBackEnd::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_cmdBufPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VKRenderBackEnd::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(m_images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
 }
void VKRenderBackEnd::initImguiStuff()
{
    m_imguiIndex = static_cast<DeviceBufferVK*>(createBuffer_imp());
    m_imguiIndex->init(DeviceBufferType::Index);
    m_imguiIndex->setAlignment(1024);
    m_imguiIndex->allocateEmpty(1024);

    m_imguiVertex = static_cast<DeviceBufferVK*>(createBuffer_imp());
    m_imguiVertex->init(DeviceBufferType::Vertex);
    m_imguiVertex->setAlignment(1024);
    m_imguiVertex->allocateEmpty(1024);

    m_imguiMat = new Material();
    m_imguiMat->loadFromTemplate("IMGUI");


    DeviceVertexInput imguiVertexInput;
    imguiVertexInput.stride = sizeof(ImDrawVert);
    imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32_SFLOAT, IM_OFFSETOF(ImDrawVert, pos)});
    imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32_SFLOAT, IM_OFFSETOF(ImDrawVert, uv)});
    imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R8G8B8A8_UNORM, IM_OFFSETOF(ImDrawVert, col)});

    DeviceVertexInput instancingInput;
    m_imguiPipeline =  new DevicePipelineVK(m_imguiMat, m_renderPass,imguiVertexInput, false, instancingInput);

    auto shader = static_cast<DeviceShaderVK *>(m_imguiMat->getProgram()->getDeviceShader());
    VkDescriptorSetLayout layout = m_imguiPipeline->getDescriptorSetLayOut();
    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = getDescriptorPool();
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &layout;
    auto res = vkAllocateDescriptorSets(m_device, &alloc_info, &m_imguiDescriptorSet);
    if(res)
    {
        abort();
    }
     printf("create descriptor sets 44444 %p\n", m_imguiDescriptorSet);




    m_imguiUniformBuffer = static_cast<DeviceBufferVK*>(createBuffer_imp());
    m_imguiUniformBuffer->init(DeviceBufferType::Uniform);
    m_imguiUniformBuffer->allocateEmpty(sizeof(Matrix44));

    //update descriptor
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_imguiUniformBuffer->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(Matrix44);

    VkWriteDescriptorSet writeSet;
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.pNext = nullptr;
    writeSet.dstSet = m_imguiDescriptorSet;
    writeSet.dstBinding = 0;
    writeSet.dstArrayElement = 0;
    writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeSet.descriptorCount = 1;
    writeSet.pBufferInfo = &bufferInfo;

    

    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    auto m_imguiTextureFont = new DeviceTextureVK();
    m_imguiTextureFont->initDataRaw(pixels, width, height, ImageFormat::R8G8B8A8);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_imguiTextureFont->getImageView();
    imageInfo.sampler = m_imguiTextureFont->getSampler();

    VkWriteDescriptorSet texWriteSet{};
    texWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    texWriteSet.dstSet = m_imguiDescriptorSet;
    texWriteSet.dstBinding = 1;
    texWriteSet.dstArrayElement = 0;
    texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texWriteSet.descriptorCount = 1;
    texWriteSet.pImageInfo = &imageInfo;

    VkWriteDescriptorSet writeSetList[] = {writeSet, texWriteSet};
    vkUpdateDescriptorSets(m_device, 2, writeSetList, 0, nullptr);
}
void VKRenderBackEnd::initVMAPool()
{
    m_memoryPool = new DeviceMemoryPoolVK(GetPhysDevice(), m_device, m_inst);
}
void VKRenderBackEnd::drawObjs(VkCommandBuffer command, std::vector<RenderCommand>& renderList)
{
    //UI的东西很特殊，一定需要根据队列的顺序来绘制,普通物体可以按材质分组绘制
    for(RenderCommand & a : renderList)
    {
        Material * mat = a.getMat();

        std::string & matStr = mat->getFullDescriptionStr();

        DevicePipelineVK * currPipeLine;
        auto iter = m_matPipelinePool.find(matStr);
        if(iter == m_matPipelinePool.end())
        {
            DeviceVertexInput imguiVertexInput;
            imguiVertexInput.stride = sizeof(VertexData);
            imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_pos)});
            imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_color)});
            imguiVertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32_SFLOAT, offsetof(VertexData, m_texCoord)});

            DeviceVertexInput instanceInput;
            currPipeLine = new DevicePipelineVK(mat, m_renderPass, imguiVertexInput, false, instanceInput);
            m_matPipelinePool[matStr]  =currPipeLine;
                
        }
        else{
            currPipeLine = iter->second;
        }
        if(m_fuckingObjList.find(currPipeLine) == m_fuckingObjList.end())
        {
            m_fuckingObjList.insert(currPipeLine);
            currPipeLine->collcetItemWiseDescritporSet();
            //update material-wise parameter.
            currPipeLine->updateUniform();
        }

        //update uniform.
        VkDescriptorSet itemDescriptorSet = currPipeLine->giveItemWiseDescriptorSet();

        size_t m_itemBufferOffset = m_itemBufferPool->giveMeBuffer(sizeof(Matrix44));
        void* data;
        vkMapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory(), m_itemBufferOffset, sizeof(Matrix44), 0, &data);
            Matrix44 wvp = a.m_transInfo.m_projectMatrix * (a.m_transInfo.m_viewMatrix  * a.m_transInfo.m_worldMatrix );
        memcpy(data, &wvp, sizeof(Matrix44));
        vkUnmapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory());
		auto mesh = a.getMesh();
		updateItemDescriptor(itemDescriptorSet, mat, m_itemBufferOffset, sizeof(Matrix44));
        //item->updateDescriptor();

        //recordDrawCommand
        auto vbo = static_cast<DeviceBufferVK *>(mesh->getArrayBuf()->bufferId());
        auto ibo = static_cast<DeviceBufferVK *>(mesh->getIndexBuf()->bufferId());

        if(vbo && ibo){
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, currPipeLine->getPipeline());
            VkBuffer vertexBuffers[] = {vbo->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
            
        
            vkCmdBindIndexBuffer(command, ibo->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            std::vector<VkDescriptorSet> descriptorSetList = {itemDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(mat->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(currPipeLine->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, currPipeLine->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);
            vkCmdDrawIndexed(command, static_cast<uint32_t>(mesh->getIndicesSize()), 1, 0, 0, 0);
        }
    }
}
void VKRenderBackEnd::drawObjs_Common(VkCommandBuffer command,VkRenderPass renderPass, std::vector<RenderCommand>& renderList)
{
    for(RenderCommand & a : renderList)
    {

        //if(a.batchType() != RenderCommand::RenderBatchType::Single) continue;
        Material * mat = a.getMat();

        std::string & matStr = mat->getFullDescriptionStr();
        DevicePipelineVK * currPipeLine;
        auto iter = m_matPipelinePool.find(matStr);
        if(iter == m_matPipelinePool.end())
        {
            DeviceVertexInput vertexInput;
            vertexInput.stride = sizeof(VertexData);
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_pos)});
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_color)});
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32_SFLOAT, offsetof(VertexData, m_texCoord)});
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_normal)});
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_tangent)});

            //use for terrain
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexData, m_matBlendFactor)});
            vertexInput.addVertexAttributeDesc({VK_FORMAT_R8G8B8_UINT, offsetof(VertexData, m_matIndex)});
            //instancing optional
            DeviceVertexInput instanceInput;
            //instancing
            if(a.batchType() != RenderCommand::RenderBatchType::Single)
            {
                instanceInput.stride = sizeof(InstanceData);
                //matrix
                instanceInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, transform)});
                instanceInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, transform) + sizeof(float) * 4});
                instanceInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, transform)+ sizeof(float) * 8});
                instanceInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, transform)+ sizeof(float) * 12});

                instanceInput.addVertexAttributeDesc({VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, extraInfo.x)});

                currPipeLine = new DevicePipelineVK(mat, renderPass, vertexInput, true, instanceInput, 4);
            }else //single draw call
            {
                currPipeLine = new DevicePipelineVK(mat, renderPass, vertexInput, false, instanceInput, 4);
            }
            
            m_matPipelinePool[matStr]  =currPipeLine;
                
        }
        else{
            currPipeLine = iter->second;
        }
        if(m_fuckingObjList.find(currPipeLine) == m_fuckingObjList.end())
        {
            m_fuckingObjList.insert(currPipeLine);
            currPipeLine->collcetItemWiseDescritporSet();
            //update material-wise parameter.
            currPipeLine->updateUniform();
        }
            
        //update uniform.
        VkDescriptorSet itemDescriptorSet = currPipeLine->giveItemWiseDescriptorSet();

        size_t m_itemBufferOffset = m_itemBufferPool->giveMeBuffer(sizeof(ItemUniform));
        void* data;
        vkMapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory(), m_itemBufferOffset, sizeof(Matrix44), 0, &data);
            ItemUniform uniformStruct;
            uniformStruct.wvp = a.m_transInfo.m_projectMatrix * (a.m_transInfo.m_viewMatrix  * a.m_transInfo.m_worldMatrix );
            uniformStruct.wv = a.m_transInfo.m_viewMatrix  * a.m_transInfo.m_worldMatrix;
            uniformStruct.world = a.m_transInfo.m_worldMatrix;
            uniformStruct.view = a.m_transInfo.m_viewMatrix;
            uniformStruct.projection = a.m_transInfo.m_projectMatrix;
        memcpy(data, &uniformStruct, sizeof(uniformStruct));
        vkUnmapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory());
		Mesh * mesh = nullptr;
        if(a.batchType() != RenderCommand::RenderBatchType::Single)
        {
            mesh = a.getInstancedMesh()->getMesh();
        }else
        {
            mesh = a.getMesh();
        }
        updateItemDescriptor(itemDescriptorSet, mat, m_itemBufferOffset, sizeof(uniformStruct));

        //recordDrawCommand
        auto vbo = static_cast<DeviceBufferVK *>(mesh->getArrayBuf()->bufferId());
        auto ibo = static_cast<DeviceBufferVK *>(mesh->getIndexBuf()->bufferId());
        if(vbo && ibo)
        {
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, currPipeLine->getPipeline());
            VkBuffer vertexBuffers[] = {vbo->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
            
            
            vkCmdBindIndexBuffer(command, ibo->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            std::vector<VkDescriptorSet> descriptorSetList = {itemDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(mat->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(currPipeLine->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, currPipeLine->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);
            if(a.batchType() != RenderCommand::RenderBatchType::Single)
            {
                auto instancingMesh = a.getInstancedMesh();
                auto instVBO = static_cast<DeviceBufferVK *>(instancingMesh->getInstanceBuf()->bufferId());
                VkBuffer instanceVertexBuffers[] = {instVBO->getBuffer()};
                VkDeviceSize instanceOffsets[] = {0};
                vkCmdBindVertexBuffers(command, 1, 1, instanceVertexBuffers, instanceOffsets);
                vkCmdDrawIndexed(command, static_cast<uint32_t>(mesh->getIndicesSize()), instancingMesh->getInstanceSize(), 0, 0, 0);
            }else
            {
                vkCmdDrawIndexed(command, static_cast<uint32_t>(mesh->getIndicesSize()), 1, 0, 0, 0);
            }
        }
    }
}
void VKRenderBackEnd::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);
    vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &commandBuffer);
}

void VKRenderBackEnd::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, int baseMipLevel, int levelCount)
{
   VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectFlags;
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_IMAGE_LAYOUT_GENERAL;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}


	void VKRenderBackEnd::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageAspectFlags aspectFlags, int mipLevel) 
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = aspectFlags;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }
void VKRenderBackEnd::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}
const VkSurfaceFormatKHR& VKRenderBackEnd::GetSurfaceFormat() const
    {
        assert(m_gfxDevIndex >= 0);
        return m_physicsDevices.m_surfaceFormats[m_gfxDevIndex][0];
    }
    const VkSurfaceCapabilitiesKHR VKRenderBackEnd::GetSurfaceCaps() const
    {
        assert(m_gfxDevIndex >= 0);
        return m_physicsDevices.m_surfaceCaps[m_gfxDevIndex];
    }

    const VkPhysicalDevice& VKRenderBackEnd::GetPhysDevice()
    {
        assert(m_gfxDevIndex >= 0);
        return m_physicsDevices.m_devices[m_gfxDevIndex];
    }

    const VulkanPhysicalDevices VKRenderBackEnd::GetPhysDeviceWrapper()
    {
        assert(m_gfxDevIndex >= 0);
        return m_physicsDevices;
    }

    VkPhysicalDeviceProperties VKRenderBackEnd::GetPhysDeviceProperties()
    {
        assert(m_gfxDevIndex >= 0);
        return m_physicsDevices.m_devProps[m_gfxDevIndex];
    }

void VKRenderBackEnd::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, int mipLevels)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    auto result = vkCreateImage(m_device, &imageInfo, nullptr, &image);
    if (result!= VK_SUCCESS) {
        char tmp[512];
        sprintf(tmp, "create Image error %d\n",result);
        throw std::runtime_error(tmp);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

	memory_type_from_properties(memRequirements.memoryTypeBits,
                                           properties,
                                           &allocInfo.memoryTypeIndex);
    //allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, image, imageMemory, 0);
}

void VKRenderBackEnd::createTextureImageView()
{
	//textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void VKRenderBackEnd::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkImageView VKRenderBackEnd::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,int baseMipLevel, int levelCount)
	{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
    viewInfo.subresourceRange.levelCount = levelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}
void VKRenderBackEnd::initDevice(GLFWwindow * window)
	{
		std::vector<VkExtensionProperties> ExtProps;
		VulkanEnumExtProps(ExtProps);
        CreateInstance();
        //create surface from GLFW
        setVKSurface(createVKSurface(&VKRenderBackEnd::shared()->getVKInstance(), window));
        initDevices();
	}

    void VKRenderBackEnd::initDevices()
    {
        VulkanGetPhysicalDevices(m_inst, m_surface, m_physicsDevices);
        SelectPhysicalDevice();
        CreateLogicalDevice();
        vkGetDeviceQueue(m_device, m_gfxQueueFamily, 0, &m_queue);
        createSwapChain();
        CreateCommandBuffer();
        createCommandBufferPoolForGeneral();
        createCommandBufferForGeneral(100);
        CreateRenderPass();
        CreateTextureToScreenRenderPass();
		createDepthResources();
        CreateFramebuffer();
        CreateShaders();
        CreateUiniform();
        CreateDescriptorSetLayout();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreatePipeline();
        createDescriptorPool();
        createDescriptorSets();
        RecordCommandBuffers();
        createSyncObjects();
        initVMAPool();
        CreateDerrferredRenderPass();
        m_itemBufferPool = new DeviceItemBufferPoolVK(1024 * 1024 * 20);
        
    }

    void VKRenderBackEnd::RenderScene()
    {

        auto screenSize = Engine::shared()->winSize();
        vkWaitForFences(m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        unsigned ImageIndex = 0;
    
        VkResult res = vkAcquireNextImageKHR(m_device, m_swapChainKHR, UINT64_MAX, imageAvailableSemaphores[currentFrame], NULL, &ImageIndex);
        if (imagesInFlight[ImageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_device, 1, &imagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
        }
        //CPU here
        m_itemBufferPool->reset();
        Renderer::shared()->collectPrimitives();
        auto & commonList = Renderer::shared()->getCommonList();
        m_fuckingObjList.clear();

        
        //------------deferred g - pass begin-------------
        VkCommandBuffer deferredCommand = m_generalCmdBuff[ImageIndex][0];

        m_gPassStage->prepare(deferredCommand);
        drawObjs_Common(deferredCommand, m_gPassStage->getRenderPass()->getRenderPass(), commonList);
        m_gPassStage->finish(deferredCommand);

        //------------deferred g - pass end-----------------

        //------------deferred Lighting Pass begin---------------
        VkCommandBuffer lightPassCmd = m_generalCmdBuff[ImageIndex][1];
        {


            m_DeferredLightingStage->prepare(lightPassCmd);
            
            vkCmdBindPipeline(lightPassCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_dirLightingPassPiepeline->getPipeline());
            m_dirLightingPassPiepeline->updateUniform();
            m_dirLightingPassPiepeline->collcetItemWiseDescritporSet();
            size_t m_itemBufferOffset = m_itemBufferPool->giveMeBuffer(sizeof(Matrix44));
            //update uniform.
            VkDescriptorSet itemDescriptorSet = m_dirLightingPassPiepeline->giveItemWiseDescriptorSet();

            //update descriptor
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            //update descriptor
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getBuffer();
            bufferInfo.offset = m_itemBufferOffset;
            bufferInfo.range = sizeof(Matrix44);

            VkWriteDescriptorSet writeSet{};
            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    writeSet.pNext = nullptr;
            writeSet.dstSet = itemDescriptorSet;
            writeSet.dstBinding = 0;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeSet.descriptorCount = 1;
            writeSet.pBufferInfo = &bufferInfo;
        	vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), 1, &writeSet, 0, nullptr);
            //descriptorWrites.emplace_back(writeSet);

            auto gbufferTex = m_gPassStage->getFrameBuffer()->getTextureList();//m_gbuffer->getTextureList();
            std::vector<VkDescriptorImageInfo> imageInfoList;
            for(int i =0; i < gbufferTex.size(); i++)
            {
                auto tex = gbufferTex[i];
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = tex->getImageLayOut();
                imageInfo.imageView = tex->getImageView();
                imageInfo.sampler = tex->getSampler();
                VkWriteDescriptorSet texWriteSet{};
                texWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                texWriteSet.dstSet = itemDescriptorSet;
                texWriteSet.dstBinding = i + 1;
                texWriteSet.dstArrayElement = 0;
                texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                texWriteSet.descriptorCount = 1;
                texWriteSet.pImageInfo = &imageInfo;
				vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), 1, &texWriteSet, 0, nullptr);
                descriptorWrites.emplace_back(texWriteSet);
            }


            VkBuffer vertexBuffers[] = {m_quadVertexBuffer->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(lightPassCmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(lightPassCmd, m_quadIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            std::vector<VkDescriptorSet> descriptorSetList = {itemDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(m_dirLightingPassPiepeline->getMat()->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(m_dirLightingPassPiepeline->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(lightPassCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_dirLightingPassPiepeline->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);
            vkCmdDrawIndexed(lightPassCmd, static_cast<uint32_t>(6), 1, 0, 0, 0);
            
            m_DeferredLightingStage->finish(lightPassCmd);

        }

        //------------deferred Lighting Pass end---------------
        //------------Sky Pass begin---------------
        VkCommandBuffer skyCmd = m_generalCmdBuff[ImageIndex][2];
        //------------Sky Pass end---------------
        {

            m_skyStage->prepare(skyCmd);

            vkCmdBindPipeline(skyCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyPassPipeLine->getPipeline());
            m_skyPassPipeLine->updateUniform();
            m_skyPassPipeLine->collcetItemWiseDescritporSet();
            size_t m_itemBufferOffset = m_itemBufferPool->giveMeBuffer(sizeof(Matrix44));
            //update uniform.
            VkDescriptorSet itemDescriptorSet = m_skyPassPipeLine->giveItemWiseDescriptorSet();
            void* data;
            vkMapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory(), m_itemBufferOffset, sizeof(Matrix44), 0, &data);
            Matrix44 m = g_GetCurrScene()->defaultCamera()->getViewProjectionMatrix();
            memcpy(data, &m, sizeof(Matrix44));
            vkUnmapMemory(VKRenderBackEnd::shared()->getDevice(), VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getMemory());

            //update descriptor
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            //update descriptor
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getBuffer();
            bufferInfo.offset = m_itemBufferOffset;
            bufferInfo.range = sizeof(Matrix44);

            VkWriteDescriptorSet writeSet{};
            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    writeSet.pNext = nullptr;
            writeSet.dstSet = itemDescriptorSet;
            writeSet.dstBinding = 0;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeSet.descriptorCount = 1;
            writeSet.pBufferInfo = &bufferInfo;
            descriptorWrites.emplace_back(writeSet);


            std::vector<VkDescriptorImageInfo> imageInfoList;
            auto tex = m_gPassStage->getFrameBuffer()->getDepthMap();
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = tex->getImageLayOut();
            imageInfo.imageView = tex->getImageView();
            imageInfo.sampler = tex->getSampler();
  
            VkWriteDescriptorSet texWriteSet{};
            texWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            texWriteSet.dstSet = itemDescriptorSet;
            texWriteSet.dstBinding = 1;
            texWriteSet.dstArrayElement = 0;
            texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            texWriteSet.descriptorCount = 1;
            texWriteSet.pImageInfo = &imageInfo;

            descriptorWrites.emplace_back(texWriteSet);

            auto sphereMesh = m_sphere;
            vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

            auto vbo = static_cast<DeviceBufferVK *>(sphereMesh->getArrayBuf()->bufferId());
            auto ibo = static_cast<DeviceBufferVK *>(sphereMesh->getIndexBuf()->bufferId());
            VkBuffer vertexBuffers[] = {vbo->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(skyCmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(skyCmd, ibo->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            std::vector<VkDescriptorSet> descriptorSetList = {itemDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(m_skyPassPipeLine->getMat()->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(m_skyPassPipeLine->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(skyCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_skyPassPipeLine->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);
            vkCmdDrawIndexed(skyCmd, static_cast<uint32_t>(sphereMesh->getIndicesSize()), 1, 0, 0, 0);

            m_skyStage->finish(skyCmd);
        }

        //------------Texture To Screen Pass begin---------------
        VkCommandBuffer textureToScreenCmd = m_generalCmdBuff[ImageIndex][3];
        {
            
            VkCommandBufferBeginInfo beginInfoDeffered = {};
            beginInfoDeffered.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfoDeffered.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            std::array<VkClearValue, 2> clearValuesDefferred{};
            clearValuesDefferred[0].color = {1.0f, 0.0f, 0.0f, 1.0f};
            clearValuesDefferred[1].depthStencil = {1.0f, 0};
            VkRenderPassBeginInfo renderPassInfoDeferred = {};
            renderPassInfoDeferred.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfoDeferred.renderPass = m_textureToScreenRenderPass;   
            renderPassInfoDeferred.renderArea.offset.x = 0;
            renderPassInfoDeferred.renderArea.offset.y = 0;
            renderPassInfoDeferred.renderArea.extent.width = screenSize.x;
            renderPassInfoDeferred.renderArea.extent.height = screenSize.y;
            renderPassInfoDeferred.clearValueCount = clearValuesDefferred.size();
            renderPassInfoDeferred.pClearValues = clearValuesDefferred.data();


            res = vkBeginCommandBuffer(textureToScreenCmd, &beginInfoDeffered);
            CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
            renderPassInfoDeferred.framebuffer = m_fbs[ImageIndex];
            vkCmdBeginRenderPass(textureToScreenCmd, &renderPassInfoDeferred, VK_SUBPASS_CONTENTS_INLINE);



            vkCmdBindPipeline(textureToScreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textureToScreenPipeline->getPipeline());

            m_textureToScreenPipeline->collcetItemWiseDescritporSet();
            size_t m_itemBufferOffset = m_itemBufferPool->giveMeBuffer(sizeof(Matrix44));
            //update uniform.
            VkDescriptorSet itemDescriptorSet = m_textureToScreenPipeline->giveItemWiseDescriptorSet();

            //update descriptor
            std::vector<VkWriteDescriptorSet> descriptorWrites{};
            //update descriptor
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = VKRenderBackEnd::shared()->getItemBufferPool()->getBuffer()->getBuffer();
            bufferInfo.offset = m_itemBufferOffset;
            bufferInfo.range = sizeof(Matrix44);

            VkWriteDescriptorSet writeSet{};
            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		    writeSet.pNext = nullptr;
            writeSet.dstSet = itemDescriptorSet;
            writeSet.dstBinding = 0;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeSet.descriptorCount = 1;
            writeSet.pBufferInfo = &bufferInfo;
            descriptorWrites.emplace_back(writeSet);

            auto lightingResultTex = m_skyStage->getFrameBuffer()->getTextureList();
            std::vector<VkDescriptorImageInfo> imageInfoList;

            auto tex = lightingResultTex[0];
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = tex->getImageLayOut();
            imageInfo.imageView = tex->getImageView();
            imageInfo.sampler = tex->getSampler();
  
            VkWriteDescriptorSet texWriteSet{};
            texWriteSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            texWriteSet.dstSet = itemDescriptorSet;
            texWriteSet.dstBinding = 1;
            texWriteSet.dstArrayElement = 0;
            texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            texWriteSet.descriptorCount = 1;
            texWriteSet.pImageInfo = &imageInfo;

            descriptorWrites.emplace_back(texWriteSet);

            vkUpdateDescriptorSets(VKRenderBackEnd::shared()->getDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

            VkBuffer vertexBuffers[] = {m_quadVertexBuffer->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(textureToScreenCmd, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(textureToScreenCmd, m_quadIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

            std::vector<VkDescriptorSet> descriptorSetList = {itemDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(m_textureToScreenPipeline->getMat()->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(m_textureToScreenPipeline->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(textureToScreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_textureToScreenPipeline->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);
            vkCmdDrawIndexed(textureToScreenCmd, static_cast<uint32_t>(6), 1, 0, 0, 0);


            vkCmdEndRenderPass(textureToScreenCmd);
            res = vkEndCommandBuffer(textureToScreenCmd);
            CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
        }
        //------------Texture To Screen Pass end---------------
        VkCommandBuffer command = m_generalCmdBuff[ImageIndex][4];
		

        
	
        auto drawSize = Renderer::shared()->getGUICommandList().size();






        

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;


		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
	
        
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;   
        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent.width = screenSize.x;
        renderPassInfo.renderArea.extent.height = screenSize.y;
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        res = vkBeginCommandBuffer(command, &beginInfo);
        CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
        renderPassInfo.framebuffer = m_fbs[ImageIndex];
        vkCmdBeginRenderPass(command, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        //drawObjs_Common(command, m_renderPass, commonList);
        drawObjs(command, Renderer::shared()->getGUICommandList());

        //IMGUI
        GUISystem::shared()->renderIMGUI();

        auto draw_data = GUISystem::shared()->getDrawData();
        int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
        // Will project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
        ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
        if (draw_data->TotalVtxCount > 0)
        {
            if(!m_imguiPipeline)
            {
                initImguiStuff();
            }
            void* data;
            vkMapMemory(m_device, m_imguiUniformBuffer->getMemory(), 0, sizeof(Matrix44), 0, &data);
		        Matrix44 projection;
                auto screenSize = Engine::shared()->winSize();
		        projection.ortho(0.0f, screenSize.x, screenSize.y, 0.0f, 0.1f, 10.0f);
                memcpy(data, &projection, sizeof(Matrix44));
            vkUnmapMemory(m_device, m_imguiUniformBuffer->getMemory());
            // Create or resize the vertex/index buffers
            size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
            size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
            if (m_imguiVertex->getBuffer() == VK_NULL_HANDLE || m_imguiVertex->getSize() < vertex_size)
                m_imguiVertex->allocateEmpty(vertex_size);
                //CreateOrResizeBuffer(rb->VertexBuffer, rb->VertexBufferMemory, rb->VertexBufferSize, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            if (m_imguiIndex->getBuffer() == VK_NULL_HANDLE || m_imguiIndex->getSize() < index_size)
                m_imguiIndex->allocateEmpty(index_size);
                //CreateOrResizeBuffer(rb->IndexBuffer, rb->IndexBufferMemory, rb->IndexBufferSize, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

            // Upload vertex/index data into a single contiguous GPU buffer
            ImDrawVert* vtx_dst = NULL;
            ImDrawIdx* idx_dst = NULL;
            VkResult err = vkMapMemory(m_device, m_imguiVertex->getMemory(), 0, vertex_size, 0, (void**)(&vtx_dst));
            err = vkMapMemory(m_device, m_imguiIndex->getMemory(), 0, index_size, 0, (void**)(&idx_dst));
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtx_dst += cmd_list->VtxBuffer.Size;
                idx_dst += cmd_list->IdxBuffer.Size;
            }
            VkMappedMemoryRange range[2] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = m_imguiVertex->getMemory();
            range[0].size = VK_WHOLE_SIZE;
            range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[1].memory = m_imguiIndex->getMemory();
            range[1].size = VK_WHOLE_SIZE;
            err = vkFlushMappedMemoryRanges(m_device, 2, range);
            vkUnmapMemory(m_device, m_imguiVertex->getMemory());
            vkUnmapMemory(m_device, m_imguiIndex->getMemory());

            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_imguiPipeline->getPipeline());


            std::vector<VkDescriptorSet> descriptorSetList = {m_imguiDescriptorSet,};
            if(static_cast<DeviceShaderVK *>(m_imguiMat->getProgram()->getDeviceShader())->isHaveMaterialDescriptorSetLayOut()){
                descriptorSetList.push_back(m_imguiPipeline->getMaterialDescriptorSet());
            }
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_imguiPipeline->getPipelineLayOut(), 0, descriptorSetList.size(), descriptorSetList.data(), 0, nullptr);


            VkBuffer vertex_buffers[1] = { m_imguiVertex->getBuffer() };
            VkDeviceSize vertex_offset[1] = { 0 };
            vkCmdBindVertexBuffers(command, 0, 1, vertex_buffers, vertex_offset);
            vkCmdBindIndexBuffer(command, m_imguiIndex->getBuffer(), 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);


            // Render command lists
            // (Because we merged all buffers into a single one, we maintain our own offset into them)
            int global_vtx_offset = 0;
            int global_idx_offset = 0;
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

                    
                    // Project scissor/clipping rectangles into framebuffer space
                    ImVec4 clip_rect;
                    clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                    clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                    clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                    clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                    if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                    {
                        // Negative offsets are illegal for vkCmdSetScissor
                        if (clip_rect.x < 0.0f)
                            clip_rect.x = 0.0f;
                        if (clip_rect.y < 0.0f)
                            clip_rect.y = 0.0f;

                        // Apply scissor/clipping rectangle
                        VkRect2D scissor;
                        scissor.offset.x = (int32_t)(clip_rect.x);
                        scissor.offset.y = (int32_t)(clip_rect.y);
                        scissor.extent.width = (uint32_t)(clip_rect.z - clip_rect.x);
                        scissor.extent.height = (uint32_t)(clip_rect.w - clip_rect.y);
                        vkCmdSetScissor(command, 0, 1, &scissor);

                        
                    }
                    // Draw
                    vkCmdDrawIndexed(command, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
                }
                global_idx_offset += cmd_list->IdxBuffer.Size;
                global_vtx_offset += cmd_list->VtxBuffer.Size;
            }
        }

        vkCmdEndRenderPass(command);
        res = vkEndCommandBuffer(command);
        CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
        Renderer::shared()->clearCommands();


        //updateUniformBuffer(ImageIndex);
        if (imagesInFlight[ImageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_device, 1, &imagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[ImageIndex] = inFlightFences[currentFrame];
        std::vector<VkCommandBuffer> commandList = {deferredCommand, lightPassCmd, skyCmd, textureToScreenCmd, command};
        VkSubmitInfo submitInfo = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = commandList.size();
        submitInfo.pCommandBuffers      = commandList.data();

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;


        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(m_device, 1, &inFlightFences[currentFrame]);
    
        res = vkQueueSubmit(m_queue, 1, &submitInfo, inFlightFences[currentFrame]);    
        CHECK_VULKAN_ERROR("vkQueueSubmit error %d\n", res);
    
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &m_swapChainKHR;
        presentInfo.pImageIndices      = &ImageIndex;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
    
        res = vkQueuePresentKHR(m_queue, &presentInfo);    
        CHECK_VULKAN_ERROR("vkQueuePresentKHR error %d\n" , res);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    DeviceTexture* VKRenderBackEnd::loadTexture_imp(const unsigned char* buf, size_t buffSize, unsigned int loadingFlag)
    {
        DeviceTextureVK * texture = new DeviceTextureVK(buf, buffSize);
        return texture;
    }

    DeviceTexture* VKRenderBackEnd::loadTextureRaw_imp(const unsigned char* buf, int width, int height, ImageFormat format, unsigned int loadingFlag)
    {
        DeviceTextureVK * texture = new DeviceTextureVK();
        texture->initDataRaw(buf, width, height,format);
        return texture;
    }

    DeviceShader* VKRenderBackEnd::createShader_imp()
    {
        return new DeviceShaderVK();
    }

    DeviceBuffer* VKRenderBackEnd::createBuffer_imp()
    {
        return new DeviceBufferVK();
    }

    VkDevice VKRenderBackEnd::getDevice()
    {
        return m_device;
    }

    VkDescriptorPool& VKRenderBackEnd::getDescriptorPool()
    {
        return descriptorPool;
    }

    DeviceItemBufferPoolVK* VKRenderBackEnd::getItemBufferPool()
    {
        return m_itemBufferPool;
    }

}
