#include "ToggleNode.h"
#include "CubeGame/ResNode.h"
#include "../ThrusterPart.h"

namespace tzw
{
	ToggleNode::ToggleNode()
	{
		name =TR(u8"�л����ؽڵ�");
		m_bearingAttr = addIn(TR(u8"����"));
		m_signalAttr = addIn(TR(u8"�����ź�"));
	}

	NodeAttrValue ToggleNode::execute()
	{
		auto attrVal = m_bearingAttr->eval();
		for(auto val : attrVal.m_list)
		{
			auto node = static_cast<ResNode *>(val.usrPtr);
			auto constraint = dynamic_cast<ThrusterPart *>(node->getProxy());

			int signal = m_signalAttr->eval().getInt();
			constraint->toggle(signal);
		}
		return NodeAttrValue();
	}

	int ToggleNode::getNodeClass()
	{
		return Node_CLASS_TOGGLE;
	}
}
