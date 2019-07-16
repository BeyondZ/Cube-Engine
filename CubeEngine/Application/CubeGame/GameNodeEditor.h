#pragma once
#include "2D/GUISystem.h"
#include "GameNodeEditorNode.h"

namespace tzw {



class GameNodeEditor : public IMGUIObject
{
public:
	// ͨ�� IMGUIObject �̳�
	void drawIMGUI(bool * isOpen);

	void addNode(GameNodeEditorNode * newNode);
 
protected:
	std::vector<GameNodeEditorNode * > m_gameNodes;
	std::vector<std::pair<int, int>> m_links;
};
}
