#include "MainMenu.h"

#include "EngineSrc/CubeEngine.h"
#include "CubeGame/GameWorld.h"
#include "Game/ConsolePanel.h"
#include "Event/EventMgr.h"
#include "Technique/MaterialPool.h"
#include "3D/Sky.h"
#include "Collision/PhysicsMgr.h"
#include "Utility/math/TbaseMath.h"
#include "BuildingSystem.h"
#include "2D/GUISystem.h"
#include "../../EngineSrc/2D/imnodes.h"
#include "ScriptPy/ScriptPyMgr.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "UIHelper.h"

#include "dirent.h"
#include "Shader/ShaderMgr.h"
#include "2D/imgui_internal.h"
#include "algorithm"
#include "ThrusterPart.h"
#include "3D/Particle/ParticleEmitter.h"
#include "3D/Particle/ParticleUpdateColorModule.h"
#include "3D/Particle/ParticleInitAlphaModule.h"
#include "3D/Particle/ParticleInitLifeSpanModule.h"
#include "3D/Particle/ParticleUpdateSizeModule.h"
#include "3D/Particle/ParticleInitVelocityModule.h"
#include "3D/Particle/ParticleInitSizeModule.h"
#include "3D/Particle/ParticleInitPosModule.h"
#include "VehicleBroswer.h"
namespace tzw {
TZW_SINGLETON_IMPL(MainMenu);
static void exitNow(Button * btn)
{
    exit(0);
}

static void onOption(Button * btn)
{

}

MainMenu::MainMenu(): m_isShowProfiler(false), m_isShowConsole(false), m_isShowNodeEditor(false),
	m_isOpenTerrain(false), m_isOpenAssetEditor(false), m_isOpenRenderEditor(false),
	m_nodeEditor(nullptr), m_fileBrowser(nullptr),
	m_crossHair(nullptr),m_preIsNeedShow(false),m_isVisible(false),m_crossHairTipsInfo(nullptr)
	
{
}

Texture * testIcon = nullptr;
void MainMenu::init()
{
	EventMgr::shared()->addFixedPiorityListener(this);
	GUISystem::shared()->addObject(this);
	m_isShowProfiler = false;
	m_isShowConsole = false;
	m_nodeEditor = new GameNodeEditor();
	m_fileBrowser = new VehicleBroswer();
	m_fileBrowser->m_saveCallBack = [&](std::string fileName)
	{
		BuildingSystem::shared()->dump(fileName);
		m_fileBrowser->close();
	};
	m_fileBrowser->m_loadCallBack = [&](std::string fileName)
	{
		m_nodeEditor->clearAll();
		BuildingSystem::shared()->load(fileName);
		m_fileBrowser->close();
	};
	testIcon = TextureMgr::shared()->getByPath("./Texture/NodeEditor/ic_restore_white_24dp.png");
	//hide();
}

void MainMenu::show()
{
	setVisible(true);
}

void MainMenu::hide()
{
	if (GameWorld::shared()->getCurrentState() == GAME_STATE_RUNNING)
	{
		Engine::shared()->setUnlimitedCursor(true);
	}
	closeAllOpenedWindow();
}

void MainMenu::toggle()
{

	if (isVisible())
	{
		hide();
	}
	else
	{
		show();
	}
}

void MainMenu::drawIMGUI()
{
	auto currIsNeedShow = isVisible() || isNeedShowWindow();
	if(m_preIsNeedShow != currIsNeedShow)
	{
		if(currIsNeedShow)
		{
			Engine::shared()->setUnlimitedCursor(false);
			if(m_crossHair)
			{
				m_crossHair->setIsVisible(false);
			}
		}
		else
		{
			Engine::shared()->setUnlimitedCursor(true);
			if(m_crossHair)
			{
				m_crossHair->setIsVisible(true);
			}
		}
		m_preIsNeedShow = currIsNeedShow;
	}
	if (isVisible() || isNeedShowWindow())
	{
		bool isOpenAbout = false;
		bool isOpenHelp = false;
		if(isVisible())
		{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu(u8"��Ϸ"))
			{
				if (ImGui::MenuItem(u8"�������", nullptr))
				{
					GameWorld::shared()->getPlayer()->removeAllBlocks();
					m_nodeEditor->clearAll();
				}
				if (ImGui::MenuItem(u8"ѡ��", "CTRL+Z")) {}
				if (ImGui::MenuItem(u8"�˳�", "CTRL+Z")) { exit(0); }
				ImGui::EndMenu();
			}
			drawToolsMenu();
			static bool isOpenTerrain = false;
			if (ImGui::BeginMenu(u8"Debug"))
			{
				auto camera = g_GetCurrScene()->defaultCamera();
				ImGui::MenuItem(u8"��������", nullptr, &m_isShowProfiler);
				ImGui::MenuItem(u8"����̨", nullptr, &m_isShowConsole);
				ImGui::MenuItem(u8"���绷������", nullptr, &m_isOpenTerrain);
				if (ImGui::MenuItem(u8"���ؽű�", nullptr)) {ScriptPyMgr::shared()->reload();}
				if(ImGui::MenuItem("Particle test"))
				{
				auto node = Node::create();
				ParticleEmitter * emitter2 = new ParticleEmitter(40);
				emitter2->setSpawnRate(0.05);
				emitter2->addInitModule(new ParticleInitSizeModule(1.0, 1.0));
				emitter2->addInitModule(new ParticleInitVelocityModule(vec3(0, 1, 0), vec3(0, 1, 0)));
				emitter2->addInitModule(new ParticleInitLifeSpanModule(2.0, 2.0));
				emitter2->addInitModule(new ParticleInitAlphaModule(0.6, 0.6));
				emitter2->addUpdateModule(new ParticleUpdateSizeModule(1.0, 0.8));
				emitter2->addUpdateModule(new ParticleUpdateColorModule(vec4(0.36, 0.36, 0.5, 0.4), vec4(0.0, 0.0, 1.0, 0.01)));
				emitter2->setIsState(ParticleEmitter::State::Playing);
				node->addChild(emitter2);
				g_GetCurrScene()->addNode(node);
				node->setPos(camera->getPos() + camera->getForward() * 15.0f);
				node->setRotateE(vec3(0, 0, 45));
				}

				if(ImGui::MenuItem(u8"Reload Shader", nullptr, nullptr))
				{
					ShaderMgr::shared()->reloadAllShaders();
				}
				ImGui::MenuItem(u8"��Ⱦ����", nullptr, &m_isOpenRenderEditor);
				ImGui::EndMenu();
			}
			if (m_isShowProfiler)
			{
				m_debugInfoPanel.drawIMGUI(&m_isShowProfiler);
			}
			if(m_isOpenRenderEditor)
			{
				ImGui::Begin("RenderEditor", &m_isOpenRenderEditor);
				if(ImGui::RadioButton("skyEnable", Renderer::shared()->isSkyEnable()))
				{
					Renderer::shared()->setSkyEnable(!Renderer::shared()->isSkyEnable());
				}
				if(ImGui::RadioButton("FogEnable", Renderer::shared()->isFogEnable()))
				{
					Renderer::shared()->setFogEnable(!Renderer::shared()->isFogEnable());
				}
				if(ImGui::RadioButton("SSAOEnable", Renderer::shared()->isSsaoEnable()))
				{
					Renderer::shared()->setSsaoEnable(!Renderer::shared()->isSsaoEnable());
				}
				if(ImGui::RadioButton("BloomEnable", Renderer::shared()->isBloomEnable()))
				{
					Renderer::shared()->setBloomEnable(!Renderer::shared()->isBloomEnable());
				}
				if(ImGui::RadioButton("HDREnable", Renderer::shared()->isHdrEnable()))
				{
					Renderer::shared()->setHdrEnable(!Renderer::shared()->isHdrEnable());
				}
				if(ImGui::RadioButton("AAEnable", Renderer::shared()->isAaEnable()))
				{
					Renderer::shared()->setAaEnable(!Renderer::shared()->isAaEnable());
				}
				if(ImGui::RadioButton("ShadowEnable", Renderer::shared()->isShadowEnable()))
				{
					Renderer::shared()->setShadowEnable(!Renderer::shared()->isShadowEnable());
				}
				ImGui::End();
			}
			if (m_isShowConsole)
			{
				ShowExampleAppConsole(&m_isShowConsole);
			}

			if (ImGui::BeginMenu("?"))
			{
				if (ImGui::MenuItem(u8"����", nullptr)) {
					isOpenAbout = true;
				}
				if (ImGui::MenuItem(u8"����", nullptr)) {
					isOpenHelp = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();

			if (isOpenAbout) 
			{
                ImGui::OpenPopup(u8"����");
				isOpenAbout = false;
            }
			if (ImGui::BeginPopupModal(u8"����", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Cube-Engine By Tzw.\ntzwtangziwen@163.com\nhttps://github.com/tangziwen/Cube-Engine");
				ImGui::Separator();
				if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}


			if (isOpenHelp)
			{

				ScriptPyMgr::shared()->callFunV("showHelpPage");
			}
		}
		}
		if(m_isShowNodeEditor) 
		{
	        m_nodeEditor->drawIMGUI(&m_isShowNodeEditor);
		}
		if(m_isOpenAssetEditor)
		{
			m_isOpenAssetEditor = ScriptPyMgr::shared()->callFunB("draw_inventory");
		}
		m_fileBrowser->drawIMGUI();
	}

}

bool MainMenu::onKeyPress(int keyCode)
{
	if (keyCode == TZW_KEY_TAB)
		toggle();
	return true;
}

bool MainMenu::isVisible() const
{
	return m_isVisible;
}

void MainMenu::setVisible(bool val)
{
	m_isVisible = val;
}

GameNodeEditor* MainMenu::getNodeEditor()
{
	return m_nodeEditor;
}

void MainMenu::setIsShowNodeEditor(bool isShow)
{
	m_isShowNodeEditor = isShow;
}

void MainMenu::setIsShowAssetEditor(bool isShow)
{
	m_isOpenAssetEditor = true;
}

void MainMenu::popFloatTips(std::string floatString)
{
	UIHelper::shared()->showFloatTips(floatString);
}

void MainMenu::closeAllOpenedWindow()
{
	m_isShowNodeEditor = false;
	m_isOpenAssetEditor = false;
	m_isOpenTerrain = false;
}

bool MainMenu::isNeedShowWindow()
{
	return m_isOpenAssetEditor | m_isShowNodeEditor|m_fileBrowser->isOpen();
}

bool MainMenu::isAnyShow()
{
	return isVisible() || isNeedShowWindow();
}

void MainMenu::setIsFileBroswerOpen(bool isOpen)
{
	if(isOpen)
	{
		m_fileBrowser->open();
	}
	else
	{
		m_fileBrowser->close();
	}	
}

void MainMenu::startGame()
{
    GameWorld::shared()->startGame();
	hide();
}

void MainMenu::drawToolsMenu()
{
	if (ImGui::BeginMenu(u8"����"))
	{
		ImGui::MenuItem(u8"�ڵ�༭��", nullptr, &m_isShowNodeEditor);
		ImGui::MenuItem(u8"�ʲ������", nullptr, &m_isOpenAssetEditor);
		ImGui::EndMenu();
	}
	if (m_isOpenTerrain)
	{
		ImGui::Begin("Terrain Inspector", &m_isOpenTerrain);
		ImGui::Text("Terrain Inspector");
		auto bloomComposisit = MaterialPool::shared()->getMatFromTemplate("BloomCompositePass");
		bloomComposisit->inspect();
		
		auto terrainMat = MaterialPool::shared()->getMatFromTemplate("VoxelTerrain");
		terrainMat->inspect();

		auto ambient = g_GetCurrScene()->getAmbient();
		auto ambientIntensity = ambient->intensity();

		ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0, 50.0);
		ambient->setIntensity(ambientIntensity);
		auto fogMat = MaterialPool::shared()->getMaterialByName("GlobalFog");
		fogMat->inspect();

		auto PostMat = MaterialPool::shared()->getMaterialByName("SSAO");
		PostMat->inspect();

		//auto sunMat = Sky::shared()->getMaterial();
		//sunMat->inspectIMGUI("sun_intensity", 0.0f, 100.0f);
		auto dirLight = g_GetCurrScene()->getDirectionLight();
		float sunAngle2 = TbaseMath::Radius2Ang(dirLight->phi());
		ImGui::SliderFloat("Sun Angle", &sunAngle2, -180, 180);
		dirLight->setPhi(TbaseMath::Ang2Radius(sunAngle2));

		float sunIntensity = dirLight->intensity();
		ImGui::SliderFloat("SunIntensity", &sunIntensity, 0.0, 50.0);
		dirLight->setIntensity(sunIntensity);
		ImGui::End();
	}
}


struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;

	void    Clear() { Buf.clear(); LineOffsets.clear(); }

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = nullptr)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		ImGui::Begin(title, p_open);
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy) ImGui::LogToClipboard();

		if (Filter.IsActive())
		{
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != nullptr; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : nullptr;
				if (Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : nullptr;
			}
		}
		else
		{
			ImGui::TextUnformatted(Buf.begin());
		}

		if (ScrollToBottom)
			ImGui::SetScrollHere(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};

void MainMenu::ShowExampleAppLog(bool* p_open)
{
	static ExampleAppLog log;
	log.Draw("LogMenu", p_open);
}

void MainMenu::ShowExampleAppConsole(bool* p_open)
{
	ConsolePanel::shared()->Draw("Console", p_open);
}

LabelNew* MainMenu::getCrossHairTipsInfo() const
{
	return m_crossHairTipsInfo;
}

void MainMenu::initInGame()
{
    m_crossHair = Sprite::create("Texture/cross_hair.png");
	 
    auto size = m_crossHair->getContentSize();
	 
    m_crossHair->setPos2D(Engine::shared()->windowWidth()/2 - size.x/2,Engine::shared()->windowHeight()/2 - size.y/2);
	 
    GameWorld::shared()->getMainRoot()->addChild(m_crossHair);
	m_crossHairTipsInfo = LabelNew::create(u8"������ʾ");
	GameWorld::shared()->getMainRoot()->addChild(m_crossHairTipsInfo);
	m_crossHairTipsInfo->setIsVisible(false);
	m_crossHairTipsInfo->setPos2D(Engine::shared()->windowWidth()/2 - size.x/2,Engine::shared()->windowHeight()/2 - size.y/2 -35);
	// m_crossHair->addChild(m_crossHairTipsInfo);
}
} // namespace tzw
