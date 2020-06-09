#pragma once

#include "GraphNode.h"
#include "GamePart.h"

namespace tzw {

struct TriggerNode: public GraphNode 
{
public:
	TriggerNode();
	virtual void trigger();
	vec3 getNodeColor() override;
	int getType() override;
	virtual void handleKeyPress(int keyCode);
	virtual void handleKeyRelease(int keyCode);
protected:

};

}
