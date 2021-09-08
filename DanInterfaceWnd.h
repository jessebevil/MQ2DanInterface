#pragma once
#include <mq/Plugin.h>
#include "DanFunctions.h"

extern bool WrongUI;

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
	CComboWnd* CPeersCombo;
	CStmlWnd* InterfaceOutput;
	CEditWnd* CQueryInputBox;
	CListWnd* CQueryList;

	CInterfaceWnd();
	~CInterfaceWnd();

	void LoadLoc();
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
