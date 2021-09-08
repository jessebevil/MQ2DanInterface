#pragma once
#include "DanInterfaceWnd.h"

bool WrongUI;
bool bAutoScroll;
std::list<std::string> sPendingChat;
std::string pluginname = "MQ2DanInterface";
std::string pluginmsg = "\ar[\a-tMQ2DanInterface\ar]\ao:: ";
std::string plugincmd = "/seedannet";
char ThisINIFileName[MAX_STRING] = "";
unsigned int iPeersSelection = 0;

CInterfaceWnd* InterfaceWnd = 0;

CInterfaceWnd::CInterfaceWnd() : CCustomWnd("InterfaceWnd") {
	int CheckUI = false;

	if (!(Tabs = (CTabWnd*)GetChildItem("Interface_Tabs"))) {
		CheckUI = true;
		WriteChatf("\arYour UI appears to be missing Interface_Tabs");
	}

	if (!(InterfaceOutput = (CStmlWnd*)GetChildItem("Interface_ChatOutput"))) {
		CheckUI = true;
		WriteChatf("\arYour UI appears to be missing Interface_ChatOutput");
	}

	//Does the UI xml have the Mode_ComboBox?
	if (!(CPeersCombo = (CComboWnd*)GetChildItem("Interface_Peers_ComboBox"))) {
		CheckUI = true;
		WriteChatf("\arYour UI appears to be missing Interface_Peers_ComboBox");
	}

	if (!(CQueryInputBox = (CEditWnd*)GetChildItem("Query_InputBox_Input"))) {
		CheckUI = true;
		WriteChatf("\arYour UI appears to be missing Query_InputBox_Input");
	}

	if (!(CQueryList = (CListWnd*)GetChildItem("Interface_QueryList_Listbox"))) {
		CheckUI = true;
		WriteChatf("\arYour UI appears to be missing Interface_QueryList_Listbox");
	}
	else {
		CQueryList->SetColumnsSizable(true);
	}

	if (!CheckUI) {
		if (InterfaceOutput) {
			InterfaceOutput->MaxLines = 400;
			InterfaceOutput->SetClickable(true);
		}
		else {
			WriteChatf("Interface output isn't valid");
		}
	}
	else {
		WriteChatf("Invalid Interface");
	}

	this->SetBGColor(0xFF000000);//Setting this here sets it for the entire window. Set it black!

	//If something I tried to load up didn't load, then let the user know their shit is out of date and they need a new xml.
	if (CheckUI) {
		WrongUI = true;
		return;
	}
	else {
		WrongUI = false;//Would only happen if they /loadskin after creating the window and there was a change to this window.
	}

	LoadLoc();
	LoadSettings();
	//Add my window to the list of notifiable windows set that we can do stuff with it.
	SetWndNotification(CInterfaceWnd);
}

CInterfaceWnd::~CInterfaceWnd() {}//Decontructor is empty, just destroy that shit.

//Save all the information about the window so when we load the window again on another session it goes back to where it was.
void CInterfaceWnd::SaveLoc() {
	if (!GetCharInfo())
		return;

	char szTemp[MAX_STRING] = { 0 };
	sprintf_s(szTemp, "%i", IsVisible());
	WritePrivateProfileString(GetCharInfo()->Name, "Show", szTemp, INIFileName);

	{
		//All the normal window info.
		//Say that the location has been saved, then we're going to save it to the INI.
		WritePrivateProfileString(GetCharInfo()->Name, "Saved", "1", INIFileName);

		sprintf_s(szTemp, "%i", GetLocation().top);
		WritePrivateProfileString(GetCharInfo()->Name, "Top", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", GetLocation().bottom);
		WritePrivateProfileString(GetCharInfo()->Name, "Bottom", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", GetLocation().left);
		WritePrivateProfileString(GetCharInfo()->Name, "Left", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", GetLocation().right);
		WritePrivateProfileString(GetCharInfo()->Name, "Right", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", GetAlpha());
		WritePrivateProfileString(GetCharInfo()->Name, "Alpha", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", GetFadeToAlpha());
		WritePrivateProfileString(GetCharInfo()->Name, "FadeToAlpha", szTemp, INIFileName);

		sprintf_s(szTemp, "%i", IsMinimized());
		WritePrivateProfileString(GetCharInfo()->Name, "Minimized", szTemp, INIFileName);
	}
}

enum Column : int {
	Number,
	Query,
	Value
};

void CInterfaceWnd::UpdateListBox()
{
	static uint64_t refreshTimer = 0;

	if (refreshTimer + 500 > GetTickCount64())
		return;

	refreshTimer = GetTickCount64();
	if (CQueryList) {
		int ScrollPos = CQueryList->GetVScrollPos();//Saving this so we can put it back where we found it.
		int Selection = CQueryList->GetCurSel();//Should save this before we delete all the entries.
		CQueryList->DeleteAll();

		std::vector<std::string> peers = GetPeersList();
		char szName[MAX_STRING] = { 0 };
		if (iPeersSelection < peers.size()) {
			strcpy_s(szName, peers.at(iPeersSelection).c_str());
		}

		std::vector<std::string> vObservations = GetObserveList(szName);
		//WriteChatf("Num Observations: %u", vObservations.size());
		for (unsigned int i = 0; i < vObservations.size(); i++) {//output each observation and it's value.
			char temp[64] = { 0 };
			_itoa_s(i, temp, 10);
			int LineNum = CQueryList->AddString(CXStr(""), 0, 0, 0);


			CQueryList->SetItemText(LineNum, Column::Number, &CXStr(temp));
			CQueryList->SetItemColor(LineNum, Column::Number, 0xFF00CC00);//makes it green. Could be any color honestly.
			CQueryList->SetItemText(LineNum, Column::Query, &CXStr(vObservations.at(i).c_str()));
			CQueryList->SetItemColor(LineNum, Column::Query, 0xFF00CC00);
			CQueryList->SetItemText(LineNum, Column::Value, &CXStr(GetObserveValue(szName, vObservations.at(i).c_str())));
			CQueryList->SetItemColor(LineNum, Column::Value, 0xFF00CC00);
		}

		if (Selection != -1)
			CQueryList->SetCurSel(Selection);

		CQueryList->SetVScrollPos(ScrollPos);
	}
}

//Should add all buffs to all peers for monitoring.
//Seems that it might go too fast, and ends up missing some. So multiple iterations are needed to correctly add them all.
void CInterfaceWnd::AddAllBuffs()
{
	std::vector<std::string> peers = GetPeersList();
	char szName[MAX_STRING] = { 0 };
	for (unsigned int i = 0; i < peers.size(); i++) {//For each Peer connected.
		strcpy_s(szName, peers.at(i).c_str());

		for (int i = 1; i < NUM_LONG_BUFFS; i++) {
			char szBuffNum[MAX_STRING] = { 0 };
			sprintf_s(szBuffNum, "Me.Buff[%i].Name", i);
			AddObservation(szName, szBuffNum, true);

		}

		for (int i = 1; i < NUM_SHORT_BUFFS; i++) {
			char szBuffNum[MAX_STRING] = { 0 };
			sprintf_s(szBuffNum, "Me.Song[%i].Name", i);
			AddObservation(szName, szBuffNum, true);
		}
	}
}

//If the window was saved, we can load it back where it was.
void CInterfaceWnd::LoadLoc() {
	if (!GetCharInfo())
		return;

	bool Saved;
	char szName[256] = { 0 };

	strcpy_s(szName, GetCharInfo()->Name);

	Saved = (GetPrivateProfileInt(szName, "Saved", 0, INIFileName) > 0 ? true : false);
	if (Saved) {//Setting the window location is causing fuckery. Let's stop setting the location and just let EQ handle it since it's being a pain in my ass.
		/*InterfaceWnd->SetLocation({
			(LONG)GetPrivateProfileInt(szName, "Left", 0, INIFileName),
			(LONG)GetPrivateProfileInt(szName, "Top", 0, INIFileName),
			(LONG)GetPrivateProfileInt(szName, "Right", 0, INIFileName),
			(LONG)GetPrivateProfileInt(szName, "Bottom", 0, INIFileName)
		});*/

		SetAlpha((BYTE)GetPrivateProfileInt(szName, "Alpha", 0, INIFileName));
		SetFadeToAlpha((BYTE)GetPrivateProfileInt(szName, "FadeToAlpha", 0, INIFileName));
		SetMinimized((BYTE)GetPrivateProfileInt(szName, "Minimized", 0, INIFileName));
	}
	//Want it to set this visible if it wasn't saved, otherwise use the INI if the entry was found.
	SetVisible((GetPrivateProfileInt(szName, "Show", 1, INIFileName) > 0 ? true : false));

	//Load the saved font. This should be "font #" not just "#", since we're passing it to the font command.
	//Same as doing "/shd font 4" the number needs to be the second arguement.
	char savedFont[MAX_STRING] = { 0 };
	GetPrivateProfileString("General", "FontSize", "font 4", savedFont, MAX_STRING, INIFileName);
	FontChange(savedFont);
}

//Fill in those checkboxes!
bool CInterfaceWnd::LoadSettings() {
	if (WrongUI)
		return true;

	std::vector<std::string> peers = GetPeersList();

	CPeersCombo->DeleteAll();//Clear the list
	CPeersCombo->SetTooltip("List of peers connected to Dannet.");

	//add all the options to the Mode ComboBox
	for (unsigned int i = 0; i < peers.size(); i++) {//For each Peer connected.
		CPeersCombo->InsertChoice((char*)peers.at(i).c_str());
	}

	CPeersCombo->InsertChoice("");//Blank choice at the end. Without it, sometimes the last entry isn't shown.
	CPeersCombo->SetChoice(iPeersSelection);//Let's set it to whatever the mode currently is.

	return true;
}


//Going to handle what happens when I change a checkbox, inputbox or combo box here.
bool bDebugWnd = false;
int CInterfaceWnd::WndNotification(CXWnd* pWnd, unsigned int Message, void* unknown) {
	//If somehow there was no pWnd, lets just leave now.
	if (!pWnd)
		return CSidlScreenWnd::WndNotification(pWnd, Message, unknown);

	if (bDebugWnd) {
		switch (Message) {
			case 1://leftmousedown
			case 2://leftmouseup
			case 21://21 is mouseover listbox. It spams really fast so we're ignoring that for output.
			case 35:
			case 36:
				break;
			default:
				WriteChatf("Message Number: %i", Message);
				break;
		}
	}


	switch (Message) {
	case XWM_LCLICK:
		//Mode
		if (pWnd == (CXWnd*)GetChildItem("Interface_Peers_ComboBox")) {
			iPeersSelection = CPeersCombo->GetCurChoice();
		}
		LoadSettings();
		break;
	case XWM_LMOUSEUP:
		break;
	case XWM_RCLICK://Right click seems to select a listbox item by default, despite no code being provided.
		break;
	case XWM_LDBLCLICK://accurate
		break;
	case XWM_HITENTER:
		break;
	case XWM_CLOSE:
		break;
	case XWN_TOOLTIP:
		break;
	case XWM_NEWVALUE://One of the Inputboxes has a new value. This Happens on resize atm.
		break;
	case XWM_SORTREQUEST:
		break;
	case XWM_IS_LINK_ACTIVE:
		break;
	case XWM_MENUSELECT:
		break;
	case 21://Mouseover listbox
		break;
	case XWM_HISTORY:
		break;
	case XWM_LCLICKHOLD://This is accurate. tested on IgnoreList listbox
		break;
	case 24://LeftClickHoldReleased
		break;
	case XWM_LINK:
		break;
	case XWM_MAXIMIZEBOX:
		break;
	case XWM_ACHIEVEMENTLINK:
		break;
	case XWN_DIALOGRESPONSELINK:
		break;
	case XWM_FOCUS://This seems more like LeftMouseDown
		break;
	case XWM_LOSTFOCUS://This seems more like LeftMouseUp (meaning it was down, and now it's released?)
		break;
	case XWM_TEXTENTRY_COMPLETE:
		break;
	case XWN_THUMBTRACK://Slider moved - 46
		break;
	case 47://Happens when I press a button? Doesn't apply to CheckBoxButtons.
		{
			if (pWnd == GetChildItem("Interface_AddQuery_Button")) {
				//CQueryList->ClearAllSel();
				char szQuery[MAX_STRING] = { 0 };
				char szName[128] = { 0 };
				if (CQueryInputBox) {
					GetCXStr(CQueryInputBox->InputText, szQuery, MAX_STRING);
				}

				{//Scope peers vector.
					std::vector<std::string> peers = GetPeersList();
					if (iPeersSelection < peers.size()) {
						strcpy_s(szName, peers.at(iPeersSelection).c_str());
					}
				}

				bool bValid = true;
				if (!strlen(szQuery)) {
					WriteOut("\apQuery \aris empty");
					bValid = false;
				}

				if (!strlen(szName)) {
					WriteOut("\apName \aris empty");
					bValid = false;
				}

				if (!bValid)//If either the query or the name was empty, let's not do anything else.
					break;

				AddObservation(szName, szQuery);
			}

			if (pWnd == GetChildItem("Interface_RemoveSelectedQuery_Button")) {
				char szQuery[MAX_STRING] = { 0 };
				char szName[128] = { 0 };
				int Selection = CQueryList->GetCurSel();
				CQueryList->ClearAllSel();

				{//Scope peers vector.
					std::vector<std::string> peers = GetPeersList();
					if (iPeersSelection < (int)peers.size()) {
						strcpy_s(szName, peers.at(iPeersSelection).c_str());
					}
				}

				std::vector<std::string> vObserveList = GetObserveList(szName);
				unsigned int count = GetObserveCount(szName);
				strcpy_s(szQuery, vObserveList.at(Selection).c_str());

				RemoveObservation(szName, szQuery);
			}
		}
		break;
	case XWN_OUTPUT_TEXT:
		break;
	case XWN_COMMANDLINK:
		break;
	default:
		if (InterfaceWnd)
			InterfaceWnd->SaveLoc();
		break;
	}
	return CSidlScreenWnd::WndNotification(pWnd, Message, unknown);
}

void CInterfaceWnd::SaveSetting(char* Key, char* Value, ...) {
	//Nothing to save atm lol.
}

void DestroyInterfaceWindow() {
	if (InterfaceWnd) {
		InterfaceWnd->SaveLoc();
		delete InterfaceWnd;
		InterfaceWnd = 0;
	}
}

void CreateInterfaceWindow() {
	if (InterfaceWnd)
		DestroyInterfaceWindow();

	if (pSidlMgr->FindScreenPieceTemplate("InterfaceWnd")) {
		InterfaceWnd = new CInterfaceWnd();

		if (InterfaceWnd->IsVisible() && !WrongUI) {
			InterfaceWnd->LoadLoc();
			((CXWnd*)InterfaceWnd)->Show(1, 1);
			InterfaceWnd->SaveLoc();
		}

		char szTitle[MAX_STRING];
		sprintf_s(szTitle, "%s %s", pluginname.c_str(), COMPILED);
		InterfaceWnd->CSetWindowText(szTitle);
	}
}

void Output(char* szText)
{
	((CXWnd*)InterfaceWnd)->Show(1, 1);
	bool bScrollDown = (InterfaceWnd->InterfaceOutput->GetVScrollPos() == InterfaceWnd->InterfaceOutput->GetVScrollMax()) ? true : false;
	char szProcessed[MAX_STRING] = { 0 };
	StripMQChat(szText, szProcessed);
	CheckChatForEvent(szProcessed);
	MQToSTML(szText, szProcessed, MAX_STRING);
	strcat_s(szProcessed, "<br>");
	CXStr NewText(szProcessed);
	ConvertItemTags(NewText, TRUE);
	(InterfaceWnd->InterfaceOutput)->AppendSTML(NewText);

	if (bScrollDown)
		((CXWnd*)InterfaceWnd->InterfaceOutput)->SetVScrollPos(InterfaceWnd->InterfaceOutput->GetVScrollMax());
};

void WriteOut(char* szText, ...)
{
	if (gFilterMQ)
		return;

	va_list vaList;
	va_start(vaList, szText);
	int len = _vscprintf(szText, vaList) + 1;// _vscprintf doesn't count // terminating '\0'
	if (char* szOutput = (char*)LocalAlloc(LPTR, len + 32)) {
		vsprintf_s(szOutput, len, szText, vaList);
		char szTemp[MAX_STRING] = { 0 };
		sprintf_s(szTemp, "%s %s", pluginmsg.c_str(), szOutput);//Add the pluginmsg to the beginning of every output.
		if (InterfaceWnd && !WrongUI && InterfaceWnd->IsVisible())
		{
			Output(szTemp);
		}
		else
			WriteChatColor(szTemp);
		LocalFree(szOutput);
	}
}

void MakeWindowVisible() {
	if (InterfaceWnd) {
		if (!InterfaceWnd->IsVisible())
			InterfaceWnd->SetVisible(1);
	}
	else {
		WriteOut("InterfaceWnd doesn't exist, sounds like your UI file isn't loaded. Try reloading your UI.");
	}
}

void MakeWindowInvisible() {
	if (InterfaceWnd) {
		if (InterfaceWnd->IsVisible())
			InterfaceWnd->SetVisible(0);
	}
}

void MakeWindowToggle() {
	if (InterfaceWnd) {
		if (InterfaceWnd->IsVisible())
			InterfaceWnd->SetVisible(0);
		else
			InterfaceWnd->SetVisible(1);
	}
}

void FontChange(const char* szLine) {//Not currently used. But hey, the code is already here!
	char Arg[MAX_STRING] = { 0 };
	GetArg(Arg, szLine, 2);

	if (InterfaceWnd)
	{
		struct FONTDATA
		{
			unsigned long NumFonts;
			char** Fonts;
		};

		FONTDATA* Fonts;            // font array structure
		unsigned long* SelFont;             // selected font
		Fonts = (FONTDATA*)&(((char*)pWndMgr)[EQ_CHAT_FONT_OFFSET]);

		if (Arg[0])
		{
			int size = atoi(Arg);

			if (size < 0 || size>10)
			{
				WriteOut("%sUsage: /%s font 0-10", pluginmsg.c_str(), plugincmd.c_str());
				return;
			}

			// check font array bounds and pointers
			if (size < 0 || size >= (int)Fonts->NumFonts)
			{
				return;
			}

			if (!Fonts->Fonts)
			{
				return;
			}

			SelFont = (unsigned long*)Fonts->Fonts[size];
			// Save the text, change the font, then restore the text
			CXStr str(((CStmlWnd*)InterfaceWnd->InterfaceOutput)->GetSTMLText());
			((CXWnd*)InterfaceWnd->InterfaceOutput)->SetFont(SelFont);
			((CStmlWnd*)InterfaceWnd->InterfaceOutput)->SetSTMLText(str, 1, 0);
			((CStmlWnd*)InterfaceWnd->InterfaceOutput)->ForceParseNow();
			// scroll to bottom of chat window
			InterfaceWnd->FontSize = size;
			WritePrivateProfileString("General", "FontSize", szLine, INIFileName);//Save the font size.
		}
		else {
			WriteOut("%s to set the font, please use /%s font #\n# must be between 0 and 10:", pluginmsg.c_str(), plugincmd.c_str());
		}
	}
}