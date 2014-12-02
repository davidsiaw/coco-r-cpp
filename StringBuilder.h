#if !defined(COCO_STRINGBUILDER_H__)
#define COCO_STRINGBUILDER_H__

#include <stddef.h>
#include <string>
#include <sstream>

namespace Coco {

class StringBuilder  
{
public:
	StringBuilder();
	StringBuilder(std::wstring val);

	virtual ~StringBuilder();
	void Append(const wchar_t val);
	void Append(const std::wstring val);
	std::wstring ToString();
	int GetLength() { return data.str().length(); };

private:
	std::wstringstream data;
};

}; // namespace

#endif // !defined(COCO_STRINGBUILDER_H__)
