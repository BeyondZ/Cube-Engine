#include "DeviceBufferVK.h"
#include "../VkRenderBackEnd.h"
namespace tzw
{
    DeviceBufferVK::DeviceBufferVK()
    {
        m_buffer = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
        m_alignment = 0;
    }
    void DeviceBufferVK::allocate(void* data, size_t ammount)
	{
        auto device = VKRenderBackEnd::shared()->getDevice();
        allocateEmpty(ammount);
        void* mapData;
        vkMapMemory(device, m_memory, 0, ammount, 0, &mapData);
            memcpy(mapData, data, (size_t) ammount);
        vkUnmapMemory(device, m_memory);
	}

    void DeviceBufferVK::allocateEmpty(size_t ammount)
    {
        VkResult res = VK_SUCCESS;
        auto device = VKRenderBackEnd::shared()->getDevice();
        if (m_buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, m_buffer, nullptr);
        if (m_memory != VK_NULL_HANDLE)
            vkFreeMemory(device, m_memory, nullptr);
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        if(m_alignment > 0)
        {
            bufferInfo.size = ((ammount - 1) / m_alignment + 1) * m_alignment;
        }
        else
        {
            bufferInfo.size = ammount;
        }
        
        if(m_type == DeviceBufferType::Vertex){
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        
        }
        else if(m_type == DeviceBufferType::Index){
            bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        else if(m_type == DeviceBufferType::Uniform){
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        res = vkCreateBuffer(device, &bufferInfo, nullptr, &m_buffer);
        if(res){
        
        abort();
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.memoryTypeIndex = 0;

        alloc_info.allocationSize = memRequirements.size;
        m_bufferSize = memRequirements.size;
        bool pass = VKRenderBackEnd::shared()->memory_type_from_properties(memRequirements.memoryTypeBits,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           &alloc_info.memoryTypeIndex);
 
        res = vkAllocateMemory(device, &alloc_info, NULL,
                               &(m_memory));
        if(res){
        
        abort();
        }
        res = vkBindBufferMemory(device, m_buffer, m_memory, 0);
        if(res){
        
        abort();
        }
    }

	bool DeviceBufferVK::init(DeviceBufferType type)
	{
        m_type = type;
		return false;
	}
	void DeviceBufferVK::bind()
	{
	}
    VkBuffer DeviceBufferVK::getBuffer()
    {
        return m_buffer;
    }
    size_t DeviceBufferVK::getAlignment()
    {
        return m_alignment;
    }
    size_t DeviceBufferVK::getSize()
    {
        return m_bufferSize;
    }
    void DeviceBufferVK::setAlignment(size_t newAlignment)
    {
        m_alignment = newAlignment;
    }
    VkDeviceMemory DeviceBufferVK::getMemory()
    {
        return m_memory;
    }
}