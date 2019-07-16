#pragma once
#include "2D/GUISystem.h"

namespace tzw {



class GameNodeEditor : public IMGUIObject
{
public:
  // ͨ�� IMGUIObject �̳�
  void drawIMGUI() override;

protected:
  std::vector<std::pair<int, int>> m_links;
};
}
