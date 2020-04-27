#pragma once
#include "Engine/EngineDef.h"
#include <unordered_map>

namespace tzw {
	class TranslationMgr : public Singleton<TranslationMgr>
	{
	public:
		TranslationMgr();
		std::string getStr(std::string);
		void dump();
		void load(std::string languageName);
		std::string getCurrLanguage();
	private:
		std::string m_languageName;
		std::unordered_map<std::string, std::string> m_currDict;
	};
} // namespace tzw
#define TR(theString) TranslationMgr::shared()->getStr(theString)
#define TRC(theString) TR(theString).c_str()