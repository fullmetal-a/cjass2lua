#include <iostream>
#include <fstream>

#include <Windows.h>
#include <process.h>
#include <filesystem>
#include <thread>
#include <shlobj_core.h>

#include "cJassParser2.h"
#include "ConfigMgr.h"
#include "Utils.h"
#include "reutils.h"
#include "resource.h"

#define DEFINE_SETTINGS
#include "Settings.h"

namespace fs = std::filesystem;
static bool g_firstRunDone = false;

enum
{
	IDC_INPUT = 201,
	IDC_OUTPUT,
	IDC_STATUS,
	IDC_STATUS_FILES,
	IDC_STATUS_LINES,
	IDC_STATUS_NODES,
	IDC_PROGRESS_FILES,
	IDC_PROGRESS_LINES,
	IDC_PROGRESS_NODES,
	IDC_BROWSE_INPUT,
	IDC_BROWSE_OUTPUT,
	IDC_BUTTON
};

namespace ProgressControls
{
	HWND hLineStatus;
	HWND hLineProgress;

	HWND hNodeStatus;
	HWND hNodeProgress;
	
	HWND hFileStatus;
	HWND hFileProgress;
}


struct ParseArgs
{
	HWND			hwnd;
	HWND			button;
	HWND			status;
	std::string		in;
	std::string		out;
};

cJass::Parser2*	   _parser = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void lineParserCallback(int current, int total)
{
	const char fmt[] = "Lines parsed: %d/%d";
	char buf[128];
	sprintf_s(buf, sizeof(buf), fmt, current, total);
	SetWindowTextA(ProgressControls::hLineStatus, buf);
	SendMessage(ProgressControls::hLineProgress, PBM_SETRANGE, 0, MAKELPARAM(0, total));
	SendMessage(ProgressControls::hLineProgress, PBM_SETPOS, current, 0);
	UpdateWindow(ProgressControls::hLineProgress);
	UpdateWindow(ProgressControls::hLineStatus);
}

void nodeParserCallback(int current, int total)
{
	static int lastTotal;
	if (total == -1)
		total = lastTotal;
	else
		lastTotal = total;
	if (current == -1)
		current = total;
	const char fmt[] = "Nodes written: %d/%d";
	char buf[128];
	sprintf_s(buf, sizeof(buf), fmt, current, total);
	SetWindowTextA(ProgressControls::hNodeStatus, buf);
	SendMessage(ProgressControls::hNodeProgress, PBM_SETRANGE, 0, MAKELPARAM(0, total));
	SendMessage(ProgressControls::hNodeProgress, PBM_SETPOS, current, 0);
	UpdateWindow(ProgressControls::hNodeProgress);
	UpdateWindow(ProgressControls::hNodeStatus);
}

void setFilesParsed(int current, int total)
{
	const char fmt[] = "File progress: %d/%d";
	char buf[128];
	sprintf_s(buf, sizeof(buf), fmt, current, total);
	SetWindowTextA(ProgressControls::hFileStatus, buf);
	SendMessage(ProgressControls::hFileProgress, PBM_SETRANGE, 0, MAKELPARAM(0, total));
	SendMessage(ProgressControls::hFileProgress, PBM_SETPOS, current, 0);
	UpdateWindow(ProgressControls::hFileProgress);
	UpdateWindow(ProgressControls::hFileStatus);
}

void parserThread(ParseArgs args)
{
	try
	{
		std::string status = "Status: Parsing file ";
		std::string status2 = "Status: Generating code for file ";
		std::string fname;

		if (Utils::dirExists(args.in))
		{
			if (Utils::fileExists(args.out))
				throw std::runtime_error("Output path when input is directory must be directory.");
			else if (!Utils::dirExists(args.out))
				SHCreateDirectoryEx(NULL, args.out.c_str(), NULL);

			int fileCounter = 0;
			int filesTotal = 0; 

			for (auto& entry : fs::directory_iterator(args.in))
			{
				fname = entry.path().u8string();
				if (Utils::dirExists(fname))
					continue;
				
				if (!Utils::strEndsWith(fname, ".txt")
					&& !Utils::strEndsWith(fname, ".j")
					&& !Utils::strEndsWith(fname, ".jass")
					&& !Utils::strEndsWith(fname, ".cjass")
					&& !Utils::strEndsWith(fname, ".cj")
					&& !Utils::strEndsWith(fname, ".jj")
					&& !Utils::strEndsWith(fname, ".w3j")
					&& !Utils::strEndsWith(fname, ".w3cj"))
					continue;

				filesTotal++;
			}

			setFilesParsed(0, filesTotal);

			for (auto& entry : fs::directory_iterator(args.in))
			{
				status = "Status: Parsing file ";
				status2 = "Status: Generating code for file ";
				fname = entry.path().u8string();

				if (Utils::dirExists(fname))
					continue;

				if (!Utils::strEndsWith(fname, ".txt")
					&& !Utils::strEndsWith(fname, ".j")
					&& !Utils::strEndsWith(fname, ".jass")
					&& !Utils::strEndsWith(fname, ".cjass")
					&& !Utils::strEndsWith(fname, ".cj")
					&& !Utils::strEndsWith(fname, ".jj")
					&& !Utils::strEndsWith(fname, ".w3j")
					&& !Utils::strEndsWith(fname, ".w3cj"))
					continue;

				status += reu::IndexSubstr(fname, fname.find_last_of("\\/") + 1, fname.length() - 1) + " ...";
				status2 += reu::IndexSubstr(fname, fname.find_last_of("\\/") + 1, fname.length() - 1) + " ...";
				SetWindowTextA(args.status, status.c_str());
				UpdateWindow(args.hwnd);
					_parser->Parse(fname, lineParserCallback);
				SetWindowTextA(args.status, status2.c_str());
				UpdateWindow(args.hwnd);
					_parser->ToLua(args.out, nodeParserCallback);
				fileCounter++;
				if (fileCounter != filesTotal)
					nodeParserCallback(0, 0);
				setFilesParsed(fileCounter, filesTotal);
			}
			nodeParserCallback(-1, -1);
			SetWindowTextA(args.status, "Status: Done!");
			EnableWindow(args.button, true);
			UpdateWindow(args.hwnd);
		}
		else
		{
			if (Utils::fileExists(args.in))
			{
				if (args.in.find("\\") != std::string::npos || args.in.find("/") != std::string::npos)
				{
					status += reu::IndexSubstr(args.in, args.in.find_last_of("\\/") + 1, args.in.length() - 1) + " ...";
					status2 += reu::IndexSubstr(args.in, args.in.find_last_of("\\/") + 1, args.in.length() - 1) + " ...";
				}
				else
				{
					status += args.in + " ...";
					status2 += args.in + " ...";
				}

				SetWindowTextA(args.status, status.c_str());
				UpdateWindow(args.hwnd);
				setFilesParsed(0, 1);
				_parser->Parse(args.in, lineParserCallback);
				SetWindowTextA(args.status, status2.c_str());
				UpdateWindow(args.hwnd);
				_parser->ToLua(args.out, nodeParserCallback);
				setFilesParsed(1, 1);
				SetWindowTextA(args.status, "Status: Done!");
				EnableWindow(args.button, true);
				UpdateWindow(args.hwnd);
			}
			else
				throw std::runtime_error("Input file does not exists.");
		g_firstRunDone = true;
		}
	}
	catch (const std::exception& ex)
	{
		appLog(Fatal) << ex.what();
		SetWindowTextA(args.status, (std::string("Status: Error - ") + ex.what()).c_str());
		EnableWindow(args.button, true);
		UpdateWindow(args.hwnd);
		g_firstRunDone = true;
	}
}

void parse(ParseArgs args)
{
	std::thread(parserThread, args).detach();
}

void parse_thisThread(const std::string& in, const std::string& out)
{
	try
	{
		std::string status = "Status: Parsing file ";
		std::string status2 = "Status: Generating code for file ";
		std::string fname;

		if (Utils::dirExists(in))
		{
			if (Utils::fileExists(out))
				throw std::runtime_error("Output path when input is directory must be directory.");
			else if (!Utils::dirExists(out))
				SHCreateDirectoryEx(NULL, out.c_str(), NULL);

			for (auto& entry : fs::directory_iterator(in))
			{
				status = "Status: Parsing file ";
				status2 = "Status: Generating code for file ";
				fname = entry.path().u8string();

				if (Utils::dirExists(fname))
					continue;

				if (!Utils::strEndsWith(fname, ".txt")
					&& !Utils::strEndsWith(fname, ".j")
					&& !Utils::strEndsWith(fname, ".jass")
					&& !Utils::strEndsWith(fname, ".cjass")
					&& !Utils::strEndsWith(fname, ".cj")
					&& !Utils::strEndsWith(fname, ".jj")
					&& !Utils::strEndsWith(fname, ".w3j")
					&& !Utils::strEndsWith(fname, ".w3cj"))
					continue;

				status += reu::IndexSubstr(fname, fname.find_last_of("\\/") + 1, fname.length() - 1) + " ...";
				status2 += reu::IndexSubstr(fname, fname.find_last_of("\\/") + 1, fname.length() - 1) + " ...";

				std::cout << "Parsing file " << fname << " ..." << std::endl;
				_parser->Parse(fname);
				std::cout << "Writing code... " << std::endl;
				_parser->ToLua(out);
			}

			std::cout << "Done!" << std::endl;
		}
		else
		{
			if (Utils::fileExists(in))
			{
				std::cout << "Parsing file " << in << " ..." << std::endl;
				_parser->Parse(in);
				std::cout << "Writing code... " << std::endl;
				_parser->ToLua(out);

				std::cout << "Done!" << std::endl;
			}
			else
				throw std::runtime_error("Input file does not exists.");
		}
		g_firstRunDone = true;
	}
	catch (const std::exception& ex)
	{
		appLog(Fatal) << ex.what();
		std::cout << "Error: " << ex.what() << std::endl;
		g_firstRunDone = true;
	}
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	try
	{
		const char CLASS_NAME[] = "MainWn";
		WNDCLASS wc = {};
		wc.lpfnWndProc = &WndProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = CLASS_NAME;
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));

		int width = 436;
		int height = 300;
		int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;


		RegisterClassA(&wc);
							
		HWND hwnd = CreateWindowExA(
			NULL,
			CLASS_NAME,
			(std::string("cJass2Lua v") + APP_VER + " " + APP_BUILD).c_str(),
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN,
			xPos, yPos, width, height,
			NULL,   
			NULL,
			hInstance,
			NULL
		);

		if (!hwnd)
			throw (std::runtime_error("Failed to create window."));

		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		HFONT hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
		::SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

		//Begin constructing window
		int ox = 10, oy = 4;
		Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("Input file or directory (Hold Shift and press Browse to select a folder)"), hInstance, WS_VISIBLE, NULL, NULL, ox, oy, 350, 20, false);
		oy += 17;
		Utils::CreateWindowElement(hwnd, ET_EDIT, Settings::lastInputPath.c_str(), hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_DISABLED, NULL, HMENU(IDC_INPUT), ox, oy, 330, 23, false);
		Utils::CreateWindowElement(hwnd, ET_BUTTON, TEXT("Browse"), hInstance, WS_VISIBLE | WS_TABSTOP | BS_FLAT, NULL, HMENU(IDC_BROWSE_INPUT), ox + 340, oy, 60, 23, false);
		oy += 27;
		Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("Output file or directory (Hold Shift and press Browse to select a folder)"), hInstance, WS_VISIBLE, NULL, NULL, ox, oy, 350, 20, false);
		oy += 17;
		Utils::CreateWindowElement(hwnd, ET_EDIT, Settings::lastOutputPath.c_str(), hInstance, WS_VISIBLE | WS_BORDER | WS_TABSTOP | WS_DISABLED, NULL, HMENU(IDC_OUTPUT), ox, oy, 330, 23, false);
		Utils::CreateWindowElement(hwnd, ET_BUTTON, TEXT("Browse"), hInstance, WS_VISIBLE | WS_TABSTOP | BS_FLAT, NULL, HMENU(IDC_BROWSE_OUTPUT), ox + 340, oy, 60, 23, false);
		oy += 27;
		Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("Status: Ready"), hInstance, WS_VISIBLE, NULL, HMENU(IDC_STATUS), ox, oy, 400, 25, false);
		oy += 31;
		ProgressControls::hLineStatus = Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("Lines parsed: 0/0"), hInstance, WS_VISIBLE, NULL, HMENU(IDC_STATUS_LINES), ox, oy, 200, 20, false);
		ProgressControls::hNodeStatus = Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("Nodes written: 0/0"), hInstance, WS_VISIBLE, NULL, HMENU(IDC_STATUS_NODES), ox + 205, oy, 200, 20, false);
		oy += 21;
		ProgressControls::hLineProgress = Utils::CreateWindowElement(hwnd, ET_PROGRESS, TEXT(""), hInstance, WS_VISIBLE, NULL, HMENU(IDC_PROGRESS_LINES), ox, oy, 195, 20, false);
		ProgressControls::hNodeProgress = Utils::CreateWindowElement(hwnd, ET_PROGRESS, TEXT(""), hInstance, WS_VISIBLE, NULL, HMENU(IDC_PROGRESS_NODES), ox + 205, oy, 195, 20, false);
		oy += 26;
		ProgressControls::hFileStatus = Utils::CreateWindowElement(hwnd, ET_STATIC, TEXT("File progress: 0/0"), hInstance, WS_VISIBLE, NULL, HMENU(IDC_STATUS_FILES), ox, oy, 200, 20, false);
		oy += 21;
		ProgressControls::hFileProgress = Utils::CreateWindowElement(hwnd, ET_PROGRESS, TEXT(""), hInstance, WS_VISIBLE, NULL, HMENU(IDC_PROGRESS_FILES), ox, oy, 400, 20, false);
		oy += 30;
		Utils::CreateWindowElement(hwnd, ET_BUTTON, TEXT("Translate"), hInstance, WS_VISIBLE | WS_TABSTOP | BS_FLAT, NULL, HMENU(IDC_BUTTON), ox, oy, 400, 30, false);

		//End constructing window

		ShowWindow(hwnd, SW_SHOW);

		MSG msg = { 0 };
		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	catch (const std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Fatal error", MB_OK | MB_ICONERROR);
		appLog(Fatal) << ex.what();
		appLog(Fatal) << "Exiting with fatal error.";

		return -1;
	}

	appLog(Info) << "Exiting normally";
	return 0;
}

int main()
{
	try
	{
		appLog(Info) << "Starting cJass2Lua application ( version" << APP_VER << "/" << APP_BUILD << ")";
		std::cout << std::string("cJass2Lua v") + APP_VER + " " + APP_BUILD << std::endl;
		LPSTR* szArgList;
		int argCount;
		szArgList = Utils::CommandLineToArgvA(GetCommandLine(), &argCount);

#ifndef _DEBUG
		if (argCount <= 1)
			::ShowWindow(::GetConsoleWindow(), SW_HIDE);
#endif

		if (!Utils::fileExists("config.ini"))
			Settings::Reset();
		Settings::config.Load("config.ini");
		Settings::Load();

		APP_LOG_LEVEL(Settings::LogLevel);
		if (Settings::OutputLanguage != "Lua")
			throw std::runtime_error("Error! Unsupported output language '" + Settings::OutputLanguage + "' in config file.");

		appLog(Info) << "Config loaded";

		_parser = new cJass::Parser2;

		if (argCount > 1)
		{
			std::string out = "";
			if (argCount > 2)
				out = szArgList[2];
			std::cout << "Starting parser..." << std::endl;
			parse_thisThread(szArgList[1], out);
			system("pause");
			return 0;
		}
		else
		{
			auto res = WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
			if (_parser)
				delete _parser;
			return res;
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "Error: " << ex.what() << std::endl;
		appLog(Fatal) << ex.what();
		appLog(Fatal) << "Exiting with fatal error.";

		if (_parser)
			delete _parser;

		return -1;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ParseArgs args;
	HWND btn, input, status, output;
	input = GetDlgItem(hwnd, IDC_INPUT);
	output = GetDlgItem(hwnd, IDC_OUTPUT);
	status = GetDlgItem(hwnd, IDC_STATUS);
	btn = GetDlgItem(hwnd, IDC_BUTTON);
	auto wp = LOWORD(wParam);

	bool isShiftDown = ((1 << 15) & GetAsyncKeyState(VK_SHIFT));
		
	char buf[MAX_PATH];
	GetWindowText(input, buf, sizeof(buf));

	switch (uMsg)
	{
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		PostQuitMessage(0);
		quick_exit(0);
		return 0;

	case WM_COMMAND:
		switch (wp)
		{
		case IDC_BUTTON:
			if (g_firstRunDone && Settings::ClearLogsOnNewTranslate)
				appLog(Clear);
			setFilesParsed(0, 0);
			lineParserCallback(0, 0);
			nodeParserCallback(0, 0);
			GetWindowText(input, buf, sizeof(buf));
			args.in = buf;
			GetWindowText(output, buf, sizeof(buf));
			args.out = buf;
			args.button = btn;
			args.status = status;
			args.hwnd = hwnd;
			EnableWindow(btn, false);
			SetWindowTextA(status, "Status: Starting parser...");
			UpdateWindow(hwnd);
			parse(args);
			break;

		case IDC_BROWSE_INPUT:
			if (isShiftDown)
			{
				Settings::lastInputPath = Utils::browseDir(input);
				Settings::Save();
			}
			else
			{
				Settings::lastInputPath = Utils::browse(hwnd, input, true);
				Settings::Save(); //TODO: Fix this
			}
	
			break;

		case IDC_BROWSE_OUTPUT:
			if (isShiftDown)
			{
				Settings::lastOutputPath = Utils::browseDir(output);
				Settings::Save();
			}
			else
			{
				Settings::lastOutputPath = Utils::browse(hwnd, output, true);
				Settings::Save();
			}
			
			break;
		}
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}