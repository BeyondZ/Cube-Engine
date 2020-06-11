#include "SwitchNode.h"
#include "CubeGame/SwitchPart.h"
#include "CubeGame/GameUISystem.h"
#include "CubeGame/BehaviorNode.h"
#include "CubeGame/BuildingSystem.h"


namespace tzw
{
	SwitchNode::SwitchNode(SwitchPart * part)
	{
		m_part = part;
		SwitchNode::syncName();
		m_onOn =addOutExe(TR(u8"��"));
		m_onOff = addOutExe(TR(u8"�ر�"));
		m_stateAttr = addOut(TR(u8"״̬�ź�"));
	}

	void SwitchNode::trigger()
	{

	}

	void SwitchNode::privateDraw()
	{
	}

	int SwitchNode::getNodeClass()
	{
		return Node_CLASS_SWITCH;
	}
	GamePart* SwitchNode::getProxy()
	{
		return m_part;
	}

	void SwitchNode::syncName()
	{
		char formatName[512];
		sprintf_s(formatName, 512, TRC(u8"���� %s"),m_part->getName().c_str());
		name = formatName;
	}

	void SwitchNode::triggerRelease()
	{
		NodeAttr * effectedAttr = nullptr;
		if(m_part->isCurrState())
		{
			effectedAttr = m_onOn;
		}else
		{
			effectedAttr = m_onOff;
		}

		auto nodeEditor = getNodeEditor();
		std::vector<GraphNode * > node_list;
		nodeEditor->findNodeLinksToAttr(effectedAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
			}
		}
	}
	bool SwitchNode::isPlayerOnSeat()
	{
		return BuildingSystem::shared()->getCurrentControlPart() && BuildingSystem::shared()->getCurrentControlPart()->getIsActivate();
	}
}
