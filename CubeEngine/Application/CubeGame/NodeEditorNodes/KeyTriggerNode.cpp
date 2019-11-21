#include "KeyTriggerNode.h"
#include "CubeGame/MainMenu.h"
#include "CubeGame/BehaviorNode.h"
#include "CubeGame/BuildingSystem.h"


namespace tzw
{
	KeyTriggerNode::KeyTriggerNode()
	{
		name = TR(u8"��������");
		m_forwardAttr =addOutExe(TR(u8"ǰ��"));
		m_sideAttr = addOutExe(TR(u8"����"));
		m_forwardSignalAttr = addOut(TR(u8"ǰ���ź�"));
		m_sideSignalAttr = addOut(TR(u8"�����ź�"));

		m_zKeyAttr =addOutExe(TR(u8"Z��"));
		m_xKeyAttr =addOutExe(TR(u8"X��"));
		m_cKeyAttr =addOutExe(TR(u8"C��"));
		m_forward = 0;
		m_side = 0;
	}

	void KeyTriggerNode::trigger()
	{

	}

	void KeyTriggerNode::handleKeyPress(int keyCode)
	{
		bool isForwardEffect = false;
		bool isSideEffect = false;
		switch(keyCode)
		{
		    case TZW_KEY_W: 
			{
				m_forward += 1;
		    	isForwardEffect = true;
		    }
			break;
		    case TZW_KEY_S: 
			{
				m_forward -= 1;
		    	isForwardEffect = true;
		    }
			break;
		    case TZW_KEY_A: 
			{
				m_side -= 1;
		    	isSideEffect = true;
		    }
			break;
		    case TZW_KEY_D: 
			{
				m_side += 1;
		    	isSideEffect = true;
		    }
			break;
		}
		m_forwardSignalAttr->m_localAttrValue.setInt(m_forward);
		m_sideSignalAttr->m_localAttrValue.setInt(m_side);
		if(isForwardEffect)
		{
			triggerForward();
		}

		if(isSideEffect)
		{
			triggerSide();
		}
	}

	void KeyTriggerNode::handleKeyRelease(int keyCode)
	{

		bool isForwardEffect = false;
		bool isSideEffect = false;
		switch(keyCode)
		{
		    case TZW_KEY_W: 
			{
				m_forward -= 1;
		    	isForwardEffect = true;
		    }
			break;
		    case TZW_KEY_S: 
			{
				m_forward += 1;
		    	isForwardEffect = true;
		    }
			break;
		    case TZW_KEY_A: 
			{
				m_side += 1;
		    	isSideEffect = true;
		    }
			break;
		    case TZW_KEY_D: 
			{
				m_side -= 1;
		    	isSideEffect = true;
		    }
			break;
		    case TZW_KEY_Z: 
			{
		    	triggerZ();
		    }
			break;
		    case TZW_KEY_X: 
			{
				triggerX();
		    }
			break;
		    case TZW_KEY_C: 
			{
				triggerC();
		    }
			break;
		}
		m_forwardSignalAttr->m_localAttrValue.setInt(m_forward);
		m_sideSignalAttr->m_localAttrValue.setInt(m_side);
		tlog("theSide %d", m_side);
		if(isForwardEffect)
		{
			triggerForward();
		}

		if(isSideEffect)
		{
			triggerSide();
		}
	}

	int KeyTriggerNode::getNodeClass()
	{
		return Node_CLASS_KEY_TRIGGER;
	}

	void KeyTriggerNode::triggerForward()
	{
		if(!isPlayerOnSeat()) return;
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_forwardAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
				//static_cast<BehaviorNode *>(node)->execute();
			}
		}
	}

	void KeyTriggerNode::triggerSide()
	{
		if(!isPlayerOnSeat()) return;
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_sideAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
				//static_cast<BehaviorNode *>(node)->execute();
			}
		}
	}

	void KeyTriggerNode::triggerZ()
	{
		auto nodeEditor = MainMenu::shared()->getNodeEditor();
		std::vector<GameNodeEditorNode * > node_list;
		nodeEditor->findNodeLinksToAttr(m_zKeyAttr, node_list);
		for(auto node : node_list)
		{
			if(node->getType() == Node_TYPE_BEHAVIOR)
			{
				nodeEditor->pushToStack(node);
				//static_cast<BehaviorNode *>(node)->execute();
			}
		}
	}

	void KeyTriggerNode::triggerX()
	{
	}

	void KeyTriggerNode::triggerC()
	{
	}

	bool KeyTriggerNode::isPlayerOnSeat()
	{
		return BuildingSystem::shared()->getCurrentControlPart() && BuildingSystem::shared()->getCurrentControlPart()->getIsActivate();
	}
}
