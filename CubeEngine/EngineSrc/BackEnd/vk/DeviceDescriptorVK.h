#pragma once
#include "vulkan/vulkan.h"
namespace tzw
{
class DeviceTextureVK;
class DeviceBufferVK;
class DeviceDescriptorVK
{
public:
	DeviceDescriptorVK(VkDescriptorSet descSet);
	void updateDescriptorByBinding(int binding, DeviceTextureVK * texture);
	void updateDescriptorByBinding(int binding, DeviceBufferVK * buffer, size_t offset, size_t range);
	VkDescriptorSet getDescSet();
private:
	VkDescriptorSet m_descriptorSet;
};
};

