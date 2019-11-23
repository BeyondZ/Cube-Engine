#include "PrintNode.h"
#include "CubeGame/ResNode.h"
#include "../BearPart.h"
#include "VarNode.h"
#include "CubeGame/UIHelper.h"

namespace tzw
{
	PrintNode::PrintNode()
	{
		name =TR(u8"��ӡ");
		m_strAttrVal = addInStr(TR(u8"�ַ���"), "let me do that shit");
	}

	NodeAttrValue PrintNode::execute()
	{
		UIHelper::shared()->showFloatTips(m_strAttrVal->eval().getStr());
		return NodeAttrValue();
	}


	int PrintNode::getNodeClass()
	{
		return Node_CLASS_PRINT;
	}
}
