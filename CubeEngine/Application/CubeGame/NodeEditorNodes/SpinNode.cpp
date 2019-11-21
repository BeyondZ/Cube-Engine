#include "SpinNode.h"
#include "CubeGame/ResNode.h"
#include "../BearPart.h"

namespace tzw
{
	SpinNode::SpinNode()
	{
		name =TR(u8"��ת");
		m_bearingAttr = addIn(TR(u8"���"));
		m_signalAttr = addInSignal(TR(u8"�����ź�"), 1);
		m_rotateSpeedAttr = addInFloat(TR(u8"ת��"), 10.0f);
	}

	NodeAttrValue SpinNode::execute()
	{
		auto attrVal = m_bearingAttr->eval();
		for(auto val : attrVal.m_list)
		{
			auto node = static_cast<ResNode *>(val.usrPtr);
			auto constraint = dynamic_cast<BearPart *>(node->getProxy());

			int signal = m_signalAttr->eval().getInt();
			if(constraint)
			{
				if(constraint->getIsSteering())
				{
					if(signal!= 0)
					{
						constraint->enableAngularMotor(true, m_rotateSpeedAttr->eval().getInt() * signal, 50);
					}else
					{
						constraint->enableAngularMotor(true, 0, 10000000.0f);
					}
				}else
				{
					if(signal!= 0)
					{
						auto speed = m_rotateSpeedAttr->eval().getInt();
						constraint->enableAngularMotor(true, speed * signal, 50);
					}else
					{
						constraint->enableAngularMotor(false, m_rotateSpeedAttr->eval().getInt(), 50);
					}
				}
			}
		}
		return NodeAttrValue();
	}

	int SpinNode::getNodeClass()
	{
		return Node_CLASS_SPIN;
	}
}
