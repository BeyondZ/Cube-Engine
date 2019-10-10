#pragma once
#include "2D/GUISystem.h"
#include "GameNodeEditorNode.h"
#include "rapidjson/document.h"
#include "Base/GuidObj.h"
#include "TriggerNode.h"

namespace tzw {

struct LinkInfo
{
	int	Id;
	int  InputId;
    int  OutputId;
};

class GameNodeEditor : public IMGUIObject, public EventListener
{
public:
	GameNodeEditor();
	// ͨ�� IMGUIObject �̳�
	void drawIMGUI(bool * isOpen);

	void addNode(GameNodeEditorNode * newNode);
	void removeNode(GameNodeEditorNode * node);
	void removeAllLink(GameNodeEditorNode * node);
	void raiseEventToNode(int startAttr, int endAttr);
	void handleLinkDump(rapidjson::Value &partDocObj, rapidjson::Document::AllocatorType& allocator);
	void handleLinkLoad(rapidjson::Value &partDocObj);
	int getNodeIdByNode(GameNodeEditorNode * node);
	void makeLinkByNode(GameNodeEditorNode * NodeA, GameNodeEditorNode * NodeB, int indexOfA, int indeOfB);
	void newNodeEditorDraw(bool * isOpen);
	NodeAttr * findAttr(int attrID);
	void drawPinIcon(const NodeAttr* pin, bool connected, int alpha);
	void ShowLeftPane(float paneWidth);
	GameNodeEditorNode * findNode(int nodeID);
	bool onKeyPress(int keyCode) override;
	bool onKeyRelease(int keyCode) override;
	void findNodeLinksToAttr(NodeAttr * attr, std::vector<GameNodeEditorNode*>&nodeList);
	GameNodeEditorNode * findNodeByAttr(NodeAttr * attr);
	GameNodeEditorNode * findNodeByAttrGid(unsigned int gid);
	GameNodeEditorNode * findNodeLinksFromAttr(NodeAttr * attr);
	NodeAttr * findAttrLinksFromAttr(NodeAttr * attr);
protected:
	std::vector<GameNodeEditorNode * > m_gameNodes;
	std::vector<LinkInfo> m_links;
	std::vector<TriggerNode * > m_triggerList;
};
}
