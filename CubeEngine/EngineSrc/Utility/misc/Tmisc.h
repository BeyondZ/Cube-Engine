#ifndef TZW_TMISC_H
#define TZW_TMISC_H

#include <string>
#define T_INVALID(p) !p
#include "time.h"

namespace tzw {

class Tmisc
{
public:
	static std::wstring StringToWString(const std::string &str);
	static std::string WstringToString(const std::wstring &wstr);
	static std::string StrFormat(const char * format,...);
	static void DurationBegin();
	static clock_t DurationEnd();
	static float clamp(float val, float min, float max);
	static std::string getUserPath(std::string filePath);
};

} // namespace tzw

#endif // TZW_TMISC_H
