#include "BehaviorNode.h"


namespace tzw
{
	BehaviorNode::BehaviorNode()
	{
		addIn(u8"ִ��");
		addOut(u8"˳��");
	}

	void BehaviorNode::execute(GameNodeEditorNode* from)
	{
		
	}

	vec3 BehaviorNode::getNodeColor()
	{
		return vec3(0.3, 0.3, 1.0);
	}
}
