#pragma once
#include "2D/GUISystem.h"
#include <functional>

namespace tzw
{
	class OptionPanel : public IMGUIObject
	{
	public:
		// ͨ�� IMGUIObject �̳�
		void drawIMGUI(bool * isOpen) override;
		OptionPanel();
		void open();
    private:
		bool m_loadOpen;
		bool m_saveOpen;
		int m_screenSizeItemCurrent;
		int m_langItemCurrent;
		int m_screenSizeEngineCurrent;
		bool m_isFullScreen;
		bool m_isOpenSSAO;
		bool m_isOpenBloom;
		bool m_isOpenAA;
		bool m_isOpenFog;
		bool m_isOpenShadow;
	};

}
