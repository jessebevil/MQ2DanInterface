#pragma once
#include "..\MQ2Plugin.h"
#include "DanFunctions.h"

extern bool WrongUI;

//This is a hacky way of defining a value for the number of checkboxes in one place so that we can use it in the header and the .cpp
enum BoxCount {//Number of checkboxes being used plus 1.
	NumCheckBoxes = 61
};

enum InputCount {//Number of InputBoxes.
	NumInputBoxes = 21
};

extern bool bAutoScroll;
extern std::list<std::string> sPendingChat;
extern std::string pluginname;
const char* const COMPILED = "-- Compiled: " __DATE__ " Time: " __TIME__;
extern char ThisINIFileName[MAX_STRING];
extern unsigned int iPeersSelection;


void CreateInterfaceWindow();
void DestroyInterfaceWindow();
void WriteOut(char* szText, ...);
void MakeWindowVisible();
void MakeWindowInvisible();
void MakeWindowToggle();
void GetWindowINIFile(char* storage, int size);
void FontChange(const char* szLine);
inline bool InGame();


class CInterfaceWnd : public CCustomWnd {
public:
	CTabWnd* Tabs;
	CCheckBoxWnd* CCheckBox[NumCheckBoxes];
	CComboWnd* CPeersCombo;
	CStmlWnd* InterfaceOutput;
	CEditWnd* CQueryInputBox;
	CListWnd* CQueryList;

	CInterfaceWnd();
	~CInterfaceWnd();

	void LoadLoc(char szChar[64] = 0);
	bool LoadSettings();
	void SaveLoc();
	void UpdateListBox();
	void AddAllBuffs();

	int WndNotification(CXWnd* pWnd, unsigned int Message, void* unknown);
	void SaveSetting(char* Key, char* Value, ...);
	unsigned long OutBoxLines;
	unsigned long FontSize;
private:
	std::vector<std::string> sCmdHistory;
	int iCurrentCmd;

};
extern CInterfaceWnd* InterfaceWnd;
