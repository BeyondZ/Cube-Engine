#include "GameNodeEditor.h"
#include "2D/imnodes.h"
#include "NodeEditorNodes/ControlPartNode.h"
#include "NodeEditorNodes/BearingPartNode.h"
//#include <minwindef.h>
#include "Base/GuidMgr.h"
#include "ControlPart.h"
#include "SpringPart.h"

namespace tzw
{
	GameNodeEditor::GameNodeEditor()
	{
		m_nodeGlobalCount = 0;
		
	}

	void GameNodeEditor::drawIMGUI(bool * isOpen)
	{
		ImGui::Begin(u8"�ڵ�༭��", isOpen);
		//if (ImGui::Button(u8"�������ڵ�")) 
		//{
		//	auto graphNode = new ControlPartNode(nullptr);
		//	addNode(graphNode);
		//}
		//ImGui::SameLine();
		//if (ImGui::Button(u8"��нڵ�")) 
		//{
		//	auto bearPartNode = new BearingPartNode(nullptr);
		//	addNode(bearPartNode);
		//}
	
		imnodes::BeginNodeEditor();

		for (size_t i = 0; i < m_gameNodes.size(); i ++) 
		{
			auto node = m_gameNodes[i];
			imnodes::BeginNode(node->m_nodeID);
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
			const std::pair<int, int> p = *i;
			if(node->checkInNodeAttr(p.first) || node->checkOutNodeAttr(p.second))
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
			const std::pair<int, int> p = *i;
			auto startAttr = i->first;
			auto endAttr = i->second;
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
			imnodes::SetNodeOrigin(newNode->m_nodeID, ImVec2(node["orgin_x"].GetDouble(), node["orgin_y"].GetDouble()));

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
		m_links.push_back(std::make_pair(start_attr, end_attr));
		raiseEventToNode(start_attr, end_attr);
	}
}
