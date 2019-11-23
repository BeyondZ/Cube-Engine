#include "ButtonPartNode.h"
#include "CubeGame/ButtonPart.h"
#include "Base/TranslationMgr.h"
#include "CubeGame/MainMenu.h"

namespace tzw
{
	ButtonPartNode::ButtonPartNode(ButtonPart * cannon)
	{
		m_part = cannon;
		ButtonPartNode::syncName();
		m_pressedAttr = addOutExe(TR(u8"����"));
		m_releasedAttr = addOutExe(TR(u8"�ɿ�"));
		m_stateAttr = addOut(TR(u8"״̬�ź�"));
	}

	void ButtonPartNode::privateDraw()
	{
		handleNameEdit();
	}

	GamePart* ButtonPartNode::getProxy()
	{
		return m_part;
	}

	void ButtonPartNode::load(rapidjson::Value& partData)
	{
		ResNode::load(partData);
	}

	void ButtonPartNode::syncName()
	{
		char formatName[512];
		sprintf_s(formatName, 512, TRC(u8"��ť %s"),m_part->getName().c_str());
		name = formatName;
	}

	void ButtonPartNode::triggerPress()
	{
		m_stateAttr->m_localAttrValue.setInt(1);
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_pressedAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
			}
		}
	}

	void ButtonPartNode::triggerRelease()
	{
		m_stateAttr->m_localAttrValue.setInt(0);
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_releasedAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
			}
		}
	}

	std::string ButtonPartNode::getResType()
	{
		return "BearPart";
	}

	int ButtonPartNode::getNodeClass()
	{
		return Node_CLASS_Button;
	}
}
