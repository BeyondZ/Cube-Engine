#include "GameNodeEditor.h"
#include "2D/imnodes.h"
#include "NodeEditorNodes/ControlPartNode.h"
#include "NodeEditorNodes/BearingPartNode.h"
//#include <minwindef.h>
#include "Base/GuidMgr.h"
#include "ControlPart.h"
#include "SpringPart.h"
#include "Engine/Engine.h"
#include "NodeEditor/Include/imgui_node_editor.h"
#include "NodeEditor/Source/imgui_node_editor_internal.h"
#include "NodeEditor/BlueprintUtilities/Include/ax/Drawing.h"
#include "NodeEditor/BlueprintUtilities/Include/ax/Widgets.h"
#include "NodeEditor/BlueprintUtilities/Include/ax/Builders.h"

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
// Struct to hold basic information about connection between
// pins. Note that connection (aka. link) has its own ID.
// This is useful later with dealing with selections, deletion
// or other operations.


namespace tzw
{
	enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
};
	static int newLinkPin  = 0;
	using IconType = ax::Drawing::IconType;
	int g_uniqueLinkIndex;
	static ed::EditorContext* g_Context = nullptr;
	GameNodeEditor::GameNodeEditor()
	{
		m_nodeGlobalCount = 0;
		g_Context = ed::CreateEditor();
	}

	void GameNodeEditor::drawIMGUI(bool * isOpen)
	{

		newNodeEditorDraw(isOpen);
	return;
		/*
		auto screenSize = Engine::shared()->winSize();
		float m_initW = 300.0f;
		float m_initH = 300.0f;
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(screenSize.x/ 2.0f - m_initW / 2.0f, screenSize.y / 2.0f - m_initH / 2.0f), ImGuiCond_Once);
		ImGui::SetNextWindowContentSize(ImVec2(1280, 960));
		ImGui::Begin(u8"�ڵ�༭��", isOpen, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		imnodes::BeginNodeEditor();
		for (size_t i = 0; i < m_gameNodes.size(); i ++) 
		{
			auto node = m_gameNodes[i];
			imnodes::BeginNode(node->m_nodeID);
			ImGui::PushItemWidth(80);
			imnodes::Name(node->name.c_str());

			node->privateDraw();
			auto inAttrList = node->getInAttrs();
			for (auto attr : inAttrList) 
			{
			    imnodes::BeginInputAttribute(attr->gID);
			    ImGui::Text(attr->m_name.c_str());
				imnodes::EndAttribute();
			}

			auto outAttrList = node->getOuAttrs();
			for (auto attr : outAttrList) 
			{
				imnodes::BeginOutputAttribute(attr->gID);
				// in between Begin|EndAttribute calls, you can call ImGui
				// UI functions
				ImGui::Text(attr->m_name.c_str());
				imnodes::EndAttribute();
			}
			ImGui::PopItemWidth();
			imnodes::EndNode();
		}

		for (int i = 0; i < m_links.size(); ++i)
		{
			const std::pair<int, int> p = m_links[i];
			// in this case, we just use the array index of the link
			// as the unique identifier
			imnodes::Link(i, p.first, p.second);
		}


		imnodes::EndNodeEditor();


		int start_attr, end_attr;
		if (imnodes::IsLinkCreated(&start_attr, &end_attr))
		{
			m_links.push_back(std::make_pair(start_attr, end_attr));
			raiseEventToNode(start_attr, end_attr);
		}
		ImGui::End();
		*/
	}

	void GameNodeEditor::addNode(GameNodeEditorNode* newNode)
	{
		m_gameNodes.push_back(newNode);
		m_nodeGlobalCount +=1;
		newNode->m_nodeID = m_nodeGlobalCount;
	}

	void GameNodeEditor::removeNode(GameNodeEditorNode* node)
	{
		auto result = std::find(m_gameNodes.begin(), m_gameNodes.end(), node);
		if(result != m_gameNodes.end())
		{
			m_gameNodes.erase(result);
			removeAllLink(node);
		}
		
	}

	void GameNodeEditor::removeAllLink(GameNodeEditorNode* node)
	{
		for (auto i = m_links.begin(); i != m_links.end();)
		{

			if(node->checkInNodeAttr(i->InputId) || node->checkOutNodeAttr(i->OutputId))
			{
				i = m_links.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	void GameNodeEditor::raiseEventToNode(int startAttr, int endAttr)
	{
		GameNodeEditorNode * startNode = nullptr;
		GameNodeEditorNode * endNode = nullptr;
		for (size_t i = 0; i < m_gameNodes.size(); i ++) 
		{
			auto node = m_gameNodes[i];
			if (!startNode) 
			{     
				if(node->checkOutNodeAttr(startAttr)) 
				{
					startNode = node;
				}
            }
			if (!endNode) 
			{     
				if(node->checkInNodeAttr(endAttr)) 
				{
					endNode = node;
				}
            }
			if (endNode && startNode) 
			{     
				break;
            }
		}

		if(startNode && endNode) 
		{
			startNode->onLinkOut(startAttr, endAttr, endNode);
			endNode->onLinkIn(startAttr, endAttr, startNode);
		}

	}

	void GameNodeEditor::handleLinkDump(rapidjson::Value& partDocObj, rapidjson::Document::AllocatorType& allocator)
	{
		rapidjson::Value NodeGraphObj(rapidjson::kObjectType);

		rapidjson::Value NodeListObj(rapidjson::kArrayType);
		for (size_t i = 0; i < m_gameNodes.size(); i ++) 
		{
			rapidjson::Value NodeObj(rapidjson::kObjectType);
			auto node = m_gameNodes[i];
			auto origin = imnodes::GetNodeOrigin(node->m_nodeID);

			NodeObj.AddMember("orgin_x", origin.x, allocator);
			NodeObj.AddMember("orgin_y", origin.y, allocator);
			node->dump(NodeObj, allocator);
			NodeListObj.PushBack(NodeObj, allocator);
		}
		NodeGraphObj.AddMember("NodeList", NodeListObj, allocator);

		rapidjson::Value linkListDoc(rapidjson::kArrayType);
		for (auto i = m_links.begin(); i != m_links.end(); ++i)
		{

			auto startAttr = i->InputId;
			auto endAttr = i->OutputId;
			GameNodeEditorNode * startNode = nullptr;
			GameNodeEditorNode * endNode = nullptr;
			for (size_t i = 0; i < m_gameNodes.size(); i ++) 
			{
				auto node = m_gameNodes[i];
				if (!startNode) 
				{     
					if(node->checkOutNodeAttr(startAttr)) 
					{
						startNode = node;
					}
	            }
				if (!endNode) 
				{     
					if(node->checkInNodeAttr(endAttr)) 
					{
						endNode = node;
					}
	            }
				if (endNode && startNode) 
				{     
					break;
	            }
			}

			if(startNode && endNode)
			{
				rapidjson::Value linkObj(rapidjson::kObjectType);
				linkObj.AddMember("from", std::string(startNode->getGUID()), allocator);
				linkObj.AddMember("to", std::string(endNode->getGUID()), allocator);
				linkObj.AddMember("fromOutputID", startNode->getOutputAttrLocalIndexByGid(startAttr), allocator);
				linkObj.AddMember("toInputID", endNode->getInputAttrLocalIndexByGid(endAttr), allocator);
				linkListDoc.PushBack(linkObj, allocator);
			}
		}
		NodeGraphObj.AddMember("NodeLinkList", linkListDoc, allocator);
		partDocObj.AddMember("NodeGraph", NodeGraphObj, allocator);
	}

	void GameNodeEditor::handleLinkLoad(rapidjson::Value& NodeGraphObj)
	{
		//read node
		auto& NodeList = NodeGraphObj["NodeList"];
		for(unsigned int i = 0; i < NodeList.Size(); i++)
		{
			auto& node = NodeList[i];
			GameNodeEditorNode * newNode = nullptr;
			//we skip create resource Node, but find exist resource node, and set properly UID
			if(strcmp(node["Type"].GetString(), "Resource") == 0)
			{
				auto resUID = node["ResUID"].GetString();
				if(strcmp(node["ResType"].GetString(), "ControlPart") == 0)//ControlPart
				{
					auto controlPart = reinterpret_cast<ControlPart*>(GUIDMgr::shared()->get(resUID));
					newNode = controlPart->getGraphNode();
					
				}
				else if(strcmp(node["ResType"].GetString(), "BearPart") == 0)//BearPart
				{
					auto bearPart = reinterpret_cast<BearPart*>(GUIDMgr::shared()->get(resUID));
					newNode = bearPart->getGraphNode();
				}
				else if(strcmp(node["ResType"].GetString(), "SpringPart") == 0)//SpringPart
				{
					auto bearPart = reinterpret_cast<SpringPart*>(GUIDMgr::shared()->get(resUID));
					newNode = bearPart->getGraphNode();
				}
			}
			else //normal logic & function node, we need create here
			{

				
			}
			//load Node Origin from file
			//imnodes::SetNodeOrigin(newNode->m_nodeID, ImVec2(node["orgin_x"].GetDouble(), node["orgin_y"].GetDouble()));
			newNode->m_origin = vec2(node["orgin_x"].GetDouble(), node["orgin_y"].GetDouble());
			//update UID from file
			newNode->setGUID(node["UID"].GetString());
		}
		//read link
		auto& linkList = NodeGraphObj["NodeLinkList"];
		for(unsigned int i = 0; i < linkList.Size(); i++) 
		{
			auto& linkObj = linkList[i];
			GameNodeEditorNode * nodeA = reinterpret_cast<GameNodeEditorNode*>(GUIDMgr::shared()->get(linkObj["from"].GetString()));
			GameNodeEditorNode * nodeB = reinterpret_cast<GameNodeEditorNode*>(GUIDMgr::shared()->get(linkObj["to"].GetString()));
			int fromOutputID = linkObj["fromOutputID"].GetInt();
			int toInputID = linkObj["toInputID"].GetInt();
			makeLinkByNode(nodeA, nodeB, fromOutputID, toInputID);
		}
	}




	void GameNodeEditor::makeLinkByNode(GameNodeEditorNode* NodeA, GameNodeEditorNode* NodeB, int indexOfA, int indeOfB)
	{
		int start_attr = NodeA->getOutByIndex(indexOfA)->gID;
		int end_attr = NodeB->getInByIndex(indeOfB)->gID;
		LinkInfo info;
		info.Id = ++g_uniqueLinkIndex;
		info.InputId = start_attr;
		info.OutputId = end_attr;
		m_links.push_back(info);
		raiseEventToNode(start_attr, end_attr);
	}

	void GameNodeEditor::newNodeEditorDraw(bool* isOpen)
	{
		static bool g_FirstFrame = true;

		auto screenSize = Engine::shared()->winSize();
		float m_initW = 500.0f;
		float m_initH = 400.0f;
		ImGui::SetNextWindowSize(ImVec2(m_initW, m_initH), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(screenSize.x/ 2.0f - m_initW / 2.0f, screenSize.y / 2.0f - m_initH / 2.0f), ImGuiCond_Once);
		ImGuiIO& io = ImGui::GetIO();
		//ImGui::Begin(u8"�ڵ�༭��", isOpen,             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Content", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus);
	    ed::SetCurrentEditor(g_Context);

	    ed::Begin("My Editor");
		auto cursorTopLeft = ImGui::GetCursorScreenPos();
		util::BlueprintNodeBuilder builder;



	    int uniqueId = 1;
	    // Start drawing nodes.
		for (size_t i = 0; i < m_gameNodes.size(); i ++) 
		{
			auto node = m_gameNodes[i];

			builder.Begin(node->m_nodeID);
			if (!node->isShowed) 
			{
				ed::SetNodePosition(node->m_nodeID, ImVec2(node->m_origin.x, node->m_origin.y));
				node->isShowed = true;
			}
			builder.Header();
				ImGui::Spring(0);
                ImGui::TextUnformatted(node->name.c_str());
                ImGui::Spring(1);
                ImGui::Dummy(ImVec2(0, 28));
				ImGui::Spring(0);
			builder.EndHeader();

			
			auto inAttrList = node->getInAttrs();
			for (auto attr : inAttrList) 
			{
				builder.Input(attr->gID);
				drawPinIcon(attr, true, (int)(1.0 * 255));
				ImGui::Text(attr->m_name.c_str());
				builder.EndInput();
			}

			auto outAttrList = node->getOuAttrs();
			for (auto attr : outAttrList) 
			{
				//ed::BeginPin(attr->gID, ed::PinKind::Output);
				builder.Output(attr->gID);
				// in between Begin|EndAttribute calls, you can call ImGui
				// UI functions
				ImGui::Text(attr->m_name.c_str());
				drawPinIcon(attr, true, (int)(1.0 * 255));
				//ed::EndPin();
				builder.EndOutput();
			}
			node->privateDraw();
			builder.End();

			//ed::EndNode();
		}

		for (int i = 0; i < m_links.size(); ++i)
		{
			auto info = m_links[i];
			// in this case, we just use the array index of the link
			// as the unique identifier
			ed::Link(info.Id, info.InputId, info.OutputId);
		}

 // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate(ImVec4(0, 1, 0, 1), 5.0f))
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
            	auto inAttr = findAttr(inputPinId.Get());
            	auto outAttr = findAttr(outputPinId.Get());
            	if(inputPinId == outputPinId)
            	{
            		ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
            	}
            	else if(inAttr->type == outAttr->type)
            	{
            		ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
            	}
            	else 
				{
	                if (ed::AcceptNewItem())
	                {
                		LinkInfo info = { ++g_uniqueLinkIndex, inputPinId.Get(), outputPinId.Get() };
                		
	                    // Since we accepted new link, lets add one to our list of links.
	                    m_links.push_back(info);

	                    // Draw new link.
	                    ed::Link(m_links.back().Id, m_links.back().InputId, m_links.back().OutputId);
                		raiseEventToNode(inputPinId.Get(), outputPinId.Get());
	                }
                }
                // ed::AcceptNewItem() return true when user release mouse button.


                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
        ed::PinId pinId = 0;
    	// when Dragging
        if (ed::QueryNewNode(&pinId))
        {
	        newLinkPin = pinId.Get();
        }
    } else {
		newLinkPin = 0;
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    // Handle deletion action
    if (ed::BeginDelete())
    {
        // There may be many links marked for deletion, let's loop over them.
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            // If you agree that link can be deleted, accept deletion.
            if (ed::AcceptDeletedItem())
            {
                for (int i = 0; i < m_links.size(); i++)
                {
                    if (m_links[i].Id == deletedLinkId.Get())
                    {
                        m_links.erase(m_links.begin() + i);
                        break;
                    }
                }
            }

            // You may reject link deletion by calling:
            // ed::RejectDeletedItem();
        }
    }
    ed::EndDelete(); // Wrap up deletion action

	ImGui::SetCursorScreenPos(cursorTopLeft);
    if (g_FirstFrame)
		ed::NavigateToContent(0.0f);
	ed::End();

	g_FirstFrame = false;
	ImGui::End();
	}

	NodeAttr* GameNodeEditor::findAttr(int attrID)
	{
		for(auto node : m_gameNodes)
		{
			auto attr = node->getAttrByGid(attrID);
			if(attr) 
			{
				return attr;
			}
		}
		return nullptr;
	}
	ImColor GetIconColor(PinType type)
	{
	    switch (type)
	    {
	        default:
	        case PinType::Flow:     return ImColor(255, 255, 255);
	        case PinType::Bool:     return ImColor(220,  48,  48);
	        case PinType::Int:      return ImColor( 68, 201, 156);
	        case PinType::Float:    return ImColor(147, 226,  74);
	        case PinType::String:   return ImColor(124,  21, 153);
	        case PinType::Object:   return ImColor( 51, 150, 215);
	        case PinType::Function: return ImColor(218,   0, 183);
	        case PinType::Delegate: return ImColor(255,  48,  48);
	    }
	}
	static const int            s_PinIconSize = 24;
	void GameNodeEditor::drawPinIcon(const NodeAttr* pin, bool connected, int alpha)
	{
		ax::Drawing::IconType iconType;
		auto type =PinType::Int;
	    ImColor  color = GetIconColor(type);
	    color.Value.w = alpha / 255.0f;
	    switch (type)
	    {
	        case PinType::Flow:     iconType = IconType::Flow;   break;
	        case PinType::Bool:     iconType = IconType::Circle; break;
	        case PinType::Int:      iconType = IconType::Circle; break;
	        case PinType::Float:    iconType = IconType::Circle; break;
	        case PinType::String:   iconType = IconType::Circle; break;
	        case PinType::Object:   iconType = IconType::Circle; break;
	        case PinType::Function: iconType = IconType::Circle; break;
	        case PinType::Delegate: iconType = IconType::Square; break;
	        default:
	            return;
	    }

	    ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
	}
}
