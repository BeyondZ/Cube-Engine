#include "GameNodeEditorNode.h"
#include "2D/imnodes.h"

namespace tzw
{
	static int g_attr_uid = 0;

	static void increaseAttrGID()
	{
		g_attr_uid += 1;
	}
	NodeAttr* GameNodeEditorNode::addIn(std::string attrName)
	{
		increaseAttrGID();
		auto attr = new NodeAttr();
		attr->m_name = attrName;
		attr->gID = g_attr_uid;
		attr->type = NodeAttr::Type::INPUT_ATTR;
		m_inAttr.push_back(attr);
		return attr;
	}

	NodeAttr* GameNodeEditorNode::addOut(std::string attrName)
	{
		increaseAttrGID();
		auto attr = new NodeAttr();
		attr->m_name = attrName;
		attr->gID = g_attr_uid;
		attr->type = NodeAttr::Type::OUTPUT_ATTR;
		m_outAttr.push_back(attr);
		return attr;
	}

	GameNodeEditorNode::GameNodeEditorNode()
	{
		increaseAttrGID();
		m_nodeID = g_attr_uid;
		isShowed = false;
	}

	std::vector<NodeAttr*>& GameNodeEditorNode::getInAttrs()
	{
		return m_inAttr;
	}

	std::vector<NodeAttr*>& GameNodeEditorNode::getOuAttrs()
	{
		return m_outAttr;
	}

	void GameNodeEditorNode::privateDraw()
	{

	}

	NodeAttr* GameNodeEditorNode::checkInNodeAttr(int gid)
	{
		for(auto attr: m_inAttr) 
		{
			if (attr->gID == gid) return attr;  
		}
		return nullptr;
	}

	NodeAttr* GameNodeEditorNode::checkOutNodeAttr(int gid)
	{
		for(auto attr: m_outAttr) 
		{
			if (attr->gID == gid) return attr;  
		}
		return nullptr;
	}

	void GameNodeEditorNode::onLinkOut(int startID, int endID, GameNodeEditorNode* other)
	{
	}

	void GameNodeEditorNode::onLinkIn(int startID, int endID, GameNodeEditorNode* other)
	{
	}

	NodeAttr* GameNodeEditorNode::getAttrByGid(int GID)
	{
		for(auto attr : m_inAttr) {
			if(attr->gID == GID)
				return attr;
		}

		for(auto attr : m_outAttr) {
			if(attr->gID == GID)
				return attr;
		}
		return nullptr;
	}

	GameNodeEditorNode::~GameNodeEditorNode()
	{

	}

	int GameNodeEditorNode::getInputAttrLocalIndexByGid(int GID)
	{
		for(int i = 0; i < m_inAttr.size(); i++) {
			if(m_inAttr[i]->gID == GID)
				return i;
		}
		return -1;
	}

	int GameNodeEditorNode::getOutputAttrLocalIndexByGid(int GID)
	{
		for(int i = 0; i < m_outAttr.size(); i++) {
			if(m_outAttr[i]->gID == GID)
				return i;
		}
		return -1;
	}

	NodeAttr* GameNodeEditorNode::getInByIndex(int localIndex)
	{
		return m_inAttr[localIndex];
	}

	NodeAttr* GameNodeEditorNode::getOutByIndex(int localIndex)
	{
		return m_outAttr[localIndex];
	}

	NodeAttr* GameNodeEditorNode::getInByGid(int globalIndex)
	{
		auto index = getInputAttrLocalIndexByGid(globalIndex);
		if( index>= 0)
		{
			return m_inAttr[index];
		}
		return nullptr;
	}

	NodeAttr* GameNodeEditorNode::getOutByGid(int globalIndex)
	{
		auto index = getOutputAttrLocalIndexByGid(globalIndex);
		if( index>= 0)
		{
			return m_outAttr[index];
		}
		return nullptr;
	}

	void GameNodeEditorNode::load(rapidjson::Value& partData)
	{
	}

	void GameNodeEditorNode::dump(rapidjson::Value& partDocObj, rapidjson::Document::AllocatorType& allocator)
	{
		partDocObj.AddMember("UID", std::string(getGUID()), allocator);
	}
}
