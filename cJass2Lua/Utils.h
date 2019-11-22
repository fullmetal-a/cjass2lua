#pragma once

#include <string>
#include <Windows.h>
#include "cJassNodes.h"

#define ET_STATIC						0
#define ET_EDIT							1
#define ET_COMBOBOX						2
#define ET_LISTBOX						3
#define ET_MDICLIENT					4
#define ET_SCROLLBAR					5
#define ET_BUTTON						6
#define ET_CHECKBOX						7
#define ET_RADIOBUTTON					8
#define ET_GROUPBOX						9
#define ET_RICHEDIT						10
#define ET_PROGRESS						11
#define ET_SLIDER_V						12
#define ET_SLIDER_H						13

namespace Utils
{
	void RemoveBOMFromString(std::string& str);
	bool fileExists(const std::string& fileName);
	bool dirExists(const std::string& str);
	bool entryExists(const std::string& str);
	std::string FileToString(std::string path);
	void StringToFile(std::string path, std::string str);
	bool strEndsWith(const std::string& str, const std::string& suffix);
	HWND CreateWindowElement(HWND Parent, UINT Type, const char* Title, HINSTANCE hInst, DWORD Style, DWORD StyleEx, HMENU ElementID, INT pos_x, INT pos_y, INT Width, INT Height, BOOL NewRadioGroup);
	char** CommandLineToArgvA(char* CmdLine, int* _argc);
	std::string op2lua(const std::string& op, cJass::OperationObject::ConstType prevType = cJass::OperationObject::ConstType::Undefined);
	std::string op2lua(const std::string& op, bool isString = false);
	int rawCodeToInt(std::string code);
	std::string const2lua(const std::string& cnst);
	cJass::OperationObject::ConstType determConstType(const std::string& cjConst);
	std::string browse(HWND mainWnd, HWND outputWindow, bool save);
	std::string browseDir(HWND ouputWindow);
}