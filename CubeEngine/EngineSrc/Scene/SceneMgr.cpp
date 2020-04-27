#include "SceneMgr.h"
namespace tzw {

SceneMgr::SceneMgr()
    :m_candidate(nullptr)
{
    m_currentScene = nullptr;
}

Scene *SceneMgr::getCurrScene() const
{
    return m_currentScene;
}

void SceneMgr::setCurrentScene(Scene *currentScene)
{
    if(!m_currentScene)
    {
        m_currentScene = currentScene;
    }else
    {
        m_candidate = currentScene;
    }
}

void SceneMgr::autoSwitch()
{
    if(m_candidate)
    {
        m_currentScene = m_candidate;
        m_candidate = nullptr;
    }
}

void SceneMgr::doVisit()
{
    autoSwitch();
    if(m_currentScene)
    {
        m_currentScene->visit();
    }
}

void SceneMgr::init()
{
    m_currentScene = new Scene();
    m_currentScene->init();
}

Scene *g_GetCurrScene()
{
	return SceneMgr::shared()->getCurrScene();
}

} // namespace tzw

