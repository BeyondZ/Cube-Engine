#pragma once
#include "2D/GUISystem.h"
#include "Base/GuidObj.h"
#include "rapidjson/document.h"

#define Node_TYPE_TRIGGER 0
#define Node_TYPE_RES 1
#define Node_TYPE_BEHAVIOR 2
#define Node_TYPE_OTHERS 3


#define Node_CLASS_WTF 0
#define Node_CLASS_KEY_TRIGGER 1
#define Node_CLASS_SPIN 2
#define Node_CLASS_VECTOR 3
#define Node_CLASS_USE 4
#define Node_CLASS_CONSTANT_INT 5
#define Node_CLASS_KEY_ANY_TRIGGER 6
#define Node_CLASS_TOGGLE 7

namespace tzw {
struct GameNodeEditorNode;

struct NodeAttrValuePrimitive
{
	enum class Type
	{
		VOID,
	    INT,
	    FLOAT,
  		USER_PTR,
	};
	NodeAttrValuePrimitive(int val);
	NodeAttrValuePrimitive(float val);
	NodeAttrValuePrimitive(void * val);
	NodeAttrValuePrimitive();
	Type m_type;
	int int_val;
	float float_val;
	void * usrPtr;
};
struct NodeAttrValue
{
	NodeAttrValue();
	int getInt();
	float getFloat();
	void * getUsrPtr();
	void setInt(int value);
	void setFloat(float value);
	void setUsrPtr(void * value);
	NodeAttrValuePrimitive getFirst();
	std::vector<NodeAttrValuePrimitive> m_list;
};
	
struct NodeAttr
{
	enum class Type
	{
	INPUT_ATTR,
	OUTPUT_ATTR,
	INVALID
	};
	enum class DataType
	{
		EXECUTE,
		DATA,
		RETURN_VALUE
	};
	std::string m_name;
	int gID;
	int tag;
	Type type;
	DataType dataType;
	NodeAttr();
	NodeAttrValue eval();
	NodeAttrValue m_localAttrValue{};
	GameNodeEditorNode * m_parent;
};

struct GameNodeEditorNode : public GuidObj
{
public:
	NodeAttr* addIn(std::string attrName);
	NodeAttr* addInExe(std::string attrName);
	NodeAttr* addOut(std::string attrName);
	NodeAttr* addOutExe(std::string attrName);
	NodeAttr * addOutReturn();
	std::string name;
	GameNodeEditorNode();
	std::vector<NodeAttr*>& getInAttrs();
	std::vector<NodeAttr*>& getOuAttrs();
	virtual void privateDraw();
	NodeAttr* checkInNodeAttr(int gid);
	NodeAttr* checkOutNodeAttr(int gid);
	virtual void onLinkOut(int startID, int endID, GameNodeEditorNode* other);
	virtual void onRemoveLinkOut(int startID,
	                           int endID,
	                           GameNodeEditorNode* other);
	virtual void onRemoveLinkIn(int startID,
	                          int endID,
	                          GameNodeEditorNode* other);
	virtual void onLinkIn(int startID, int endID, GameNodeEditorNode* other);
	NodeAttr* getAttrByGid(int GID);
	virtual ~GameNodeEditorNode();
	int getInputAttrLocalIndexByGid(int GID);
	int getOutputAttrLocalIndexByGid(int GID);
	NodeAttr* getInByIndex(int localIndex);
	NodeAttr* getOutByIndex(int localIndex);
	NodeAttr* getInByGid(int GID);
	NodeAttr* getOutByGid(int GID);
	virtual vec3 getNodeColor();
	virtual void load(rapidjson::Value& partData);
	virtual void dump(rapidjson::Value& partDocObj,
	                rapidjson::Document::AllocatorType& allocator);
	virtual NodeAttrValue execute();
	virtual void handleExeOut();
	int m_nodeID;
	vec2 m_origin;
	bool isShowed;
	virtual int getType();
	virtual int getNodeClass();
protected:
	std::vector<NodeAttr*> m_inAttr;
	std::vector<NodeAttr*> m_outAttr;
	std::map<int, int> m_inGlobalMap;
	std::map<int, int> m_OutGlobalMap;
};
}
