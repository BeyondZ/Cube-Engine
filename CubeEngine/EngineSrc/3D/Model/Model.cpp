#include "Model.h"

#include "ModelLoader.h"
#include "../../Rendering/Renderer.h"
#include "../../Scene/SceneMgr.h"
#include "Utility/misc/Tmisc.h"
namespace tzw {

Model::Model()
{

}

void Model::initWithFile(std::string modelFilePath)
{
    ModelLoader::shared()->loadModel(this,modelFilePath);
    setCamera(g_GetCurrScene()->defaultCamera());
    for(auto mesh : m_meshList)
    {
		mesh->calculateAABB();
		m_localAABB.merge(mesh->getAabb());
	}
    reCache();
    reCacheAABB();
    setIsAccpectOcTtree(true);
}

Model *Model::create(std::string modelFilePath)
{
    auto theModel = new Model();
    theModel->initWithFile(modelFilePath);
    return theModel;
}

void Model::submitDrawCmd(RenderCommand::RenderType passType)
{
	if(getIsVisible())
	{
		auto type = passType;
	    for(auto mesh : m_meshList)
	    {
	        auto tech = m_effectList[0];
	        RenderCommand command(mesh,tech,type);
    		setUpCommand(command);
	        setUpTransFormation(command.m_transInfo);
	        Renderer::shared()->addRenderCommand(command);
	    }
    }
}

Mesh * Model::getMesh(int id)
{
	return m_meshList[id];
}

Mesh* Model::getMesh()
{
	return m_meshList[0];
}

Material* Model::getMat(int index)
{
	return m_effectList[index];
}

void Model::setMaterial(Material* mat)
{
	m_effectList[0] = mat;
}

void Model::setColor(vec4 color)
{
	Drawable::setColor(color);
	m_effectList[0]->setVar("TU_color", color);
}
} // namespace tzw
