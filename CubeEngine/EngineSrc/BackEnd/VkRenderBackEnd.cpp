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
#define ENABLE_DEBUG_LAYERS 1
namespace tzw
{

#define CHECK_VULKAN_ERROR(a, b) {if(b){printf(a, b);abort();}}
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

const int MAX_FRAMES_IN_FLIGHT = 1;
static void initVk()
{
    
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const bool enableValidationLayers = true;
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
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
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
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    populateDebugMessengerCreateInfo(debugCreateInfo);
    instInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#endif    
    instInfo.enabledExtensionCount = ARRAY_SIZE_IN_ELEMENTS(pInstExt);
    instInfo.ppEnabledExtensionNames = pInstExt;         

    VkResult res = vkCreateInstance(&instInfo, NULL, &m_inst);
    CHECK_VULKAN_ERROR("vkCreateInstance %d\n", res);
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
            VkBuffer vertexBuffers[] = {vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(m_cmdBufs[i], 0, 1, vertexBuffers, offsets);
            
            vkCmdBindIndexBuffer(m_cmdBufs[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            //vkCmdDraw(m_cmdBufs[i], 3, 1, 0, 0);

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
        attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
    }
    void VKRenderBackEnd::CreatePipeline()
    {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
    
        shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageCreateInfo[0].module = m_vsModule;
        shaderStageCreateInfo[0].pName = "main";
        shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageCreateInfo[1].module = m_fsModule;
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
		
        CreateVertexBuffer();
        CreateIndexBuffer();

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
        CHECK_VULKAN_ERROR("vkCreateGraphicsPipelines error %d\n", res);
    
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
	/*
        VertexData vertices[] = {
            VertexData(vec3(-0.5 + 0.2, -0.5,  0), vec2(1.0f, 0.0f), m_color),
            VertexData(vec3(0.5 + 0.2, -0.5,  0), vec2(0.0f, 0.0f), m_color),
            VertexData(vec3(0.5 + 0.2, 0.5,  0), vec2(0.0f, 1.0f), m_color),
            VertexData(vec3(-0.5 + 0.2, 0.5,  0), vec2(1.0f, 1.0f), m_color),


            VertexData(vec3(-0.5, -0.5,  -0.1), vec2(1.0f, 0.0f), m_color1),
            VertexData(vec3(0.5, -0.5,  -0.1), vec2(0.0f, 0.0f), m_color1),
            VertexData(vec3(0.5, 0.5,  -0.1), vec2(0.0f, 1.0f), m_color1),
            VertexData(vec3(-0.5, 0.5,  -0.1), vec2(1.0f, 1.0f), m_color1),
        };
        */
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
	

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(vertices[0]) * ARRAY_SIZE_IN_ELEMENTS(vertices);
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        
        vkCreateBuffer(m_device, &bufferInfo, nullptr, &vertexBuffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = 0;

        alloc_info.allocationSize = memRequirements.size;
        bool pass = memory_type_from_properties(memRequirements.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &alloc_info.memoryTypeIndex);
        VkDeviceMemory mem;
        vkAllocateMemory(m_device, &alloc_info, NULL,
                               &(mem));

        void* data;
        vkMapMemory(m_device, mem, 0, bufferInfo.size, 0, &data);
            memcpy(data, vertices, (size_t) bufferInfo.size);
        vkUnmapMemory(m_device, mem);
        vkBindBufferMemory(m_device, vertexBuffer, mem, 0);
    }

    void VKRenderBackEnd::createDepthResources()
	{
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;//findDepthFormat();
		auto screenSize = Engine::shared()->winSize();
        createImage(screenSize.x, screenSize.y, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
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
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(uint16_t) * indices.size();
        bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


        vkCreateBuffer(m_device, &bufferInfo, nullptr, &indexBuffer);



        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, indexBuffer, &memRequirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = 0;

        alloc_info.allocationSize = memRequirements.size;
        bool pass = memory_type_from_properties(memRequirements.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &alloc_info.memoryTypeIndex);
        VkDeviceMemory mem;
        vkAllocateMemory(m_device, &alloc_info, NULL,
                               &(mem));

        void* data;
        vkMapMemory(m_device, mem, 0, bufferInfo.size, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferInfo.size);
        vkUnmapMemory(m_device, mem);
        vkBindBufferMemory(m_device, indexBuffer, mem, 0);

    }
    void VKRenderBackEnd::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = bindings.size();
        layoutInfo.pBindings = bindings.data();

        auto res = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout);
        assert(!res);
    }
    void VKRenderBackEnd::createDescriptorPool()
    {
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(m_images.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(m_images.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(m_images.size());

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

        for (size_t i = 0; i < m_images.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(Matrix44);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
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
        VkMemoryRequirements memRequirements;
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

void VKRenderBackEnd::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("./logo.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createVKBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
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

void VKRenderBackEnd::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
   VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
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
    } else {
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


	void VKRenderBackEnd::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
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

void VKRenderBackEnd::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
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
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
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
    samplerInfo.anisotropyEnable = VK_TRUE;
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

VkImageView VKRenderBackEnd::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
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
        CreateRenderPass();
		createDepthResources();
        CreateFramebuffer();
        CreateShaders();
        CreateUiniform();
        CreateDescriptorSetLayout();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
        CreatePipeline();
        createDescriptorPool();
        createDescriptorSets();
        RecordCommandBuffers();
        createSyncObjects();
    }

    void VKRenderBackEnd::RenderScene()
    {

        vkWaitForFences(m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        unsigned ImageIndex = 0;
    
        VkResult res = vkAcquireNextImageKHR(m_device, m_swapChainKHR, UINT64_MAX, imageAvailableSemaphores[currentFrame], NULL, &ImageIndex);
        if (imagesInFlight[ImageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(m_device, 1, &imagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
        }


        //CHECK_VULKAN_ERROR("vkAcquireNextImageKHR error %d\n" , res);
        updateUniformBuffer(ImageIndex);
 
        imagesInFlight[ImageIndex] = inFlightFences[currentFrame];
        VkSubmitInfo submitInfo = {};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &m_cmdBufs[ImageIndex];

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
    
        res = vkQueuePresentKHR(m_queue, &presentInfo);    
        CHECK_VULKAN_ERROR("vkQueuePresentKHR error %d\n" , res);
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}
