#include "KeyAnyTriggerNode.h"
#include "CubeGame/MainMenu.h"
#include "CubeGame/BehaviorNode.h"
#include "CubeGame/BuildingSystem.h"


namespace tzw
{
	KeyAnyTriggerNode::KeyAnyTriggerNode()
	{
		name =u8"��������";
		m_pressedAttr =addOutExe(u8"����");
		m_ReleasedAttr = addOutExe(u8"�ɿ�");
		m_keyCode = TZW_KEY_N;
	}

	void KeyAnyTriggerNode::trigger()
	{

	}

	void KeyAnyTriggerNode::privateDraw()
	{
		int intValue = m_keyCode;
		ImGui::PushItemWidth(80);
		bool isInput = ImGui::InputInt("",&intValue);
		ImGui::PopItemWidth();
		if(isInput)
		{
			m_keyCode = intValue;
			//m_attr->m_localAttrValue.setInt(intValue);
		}
	}

	void KeyAnyTriggerNode::handleKeyPress(int keyCode)
	{
		if(!m_keyCode) return;
		if(m_keyCode == keyCode)
		{
			triggerPress();
		}
	}

	void KeyAnyTriggerNode::handleKeyRelease(int keyCode)
	{
		if(!m_keyCode) return;
		if(m_keyCode == keyCode)
		{
			triggerRelease();
		}
	}

	int KeyAnyTriggerNode::getNodeClass()
	{
		return Node_CLASS_KEY_TRIGGER;
	}

	void KeyAnyTriggerNode::triggerPress()
	{
		//if(!isPlayerOnSeat()) return;
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_pressedAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
				//static_cast<BehaviorNode *>(node)->execute();
			}
		}
	}

	void KeyAnyTriggerNode::triggerRelease()
	{
		//if(!isPlayerOnSeat()) return;
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_ReleasedAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
				//static_cast<BehaviorNode *>(node)->execute();
			}
		}
	}
	bool KeyAnyTriggerNode::isPlayerOnSeat()
	{
		return BuildingSystem::shared()->getCurrentControlPart() && BuildingSystem::shared()->getCurrentControlPart()->getIsActivate();
	}
}
