// MQ2DanInterface.cpp
// By Chatwiththisname

/*
Still needed.
a Listbox to show channels
a Listbox to show currently connected peers. (handled in the peers combo box?)
Make the peers combo box repopulate when the number of peers changes. Right now it's only populated when the plugin loads the window I believe.

Reading Material:
Dannet Resource Page: https://www.redguides.com/community/resources/mq2dannet.322/
*/


#include <mq/Plugin.h>
#include "DanInterfaceWnd.h"
#include <functional>

PreSetup("MQ2DanInterface");
typedef void(*fFunction)();
std::map<std::string, fFunction> options;

MQPlugin* FindMQ2DanNetPlugin()
{
	MQPlugin* pPlugin = pPlugins;
	while (pPlugin) {
		if (!_stricmp("MQ2Dannet", pPlugin->szFilename)) {
			return pPlugin;
		}

		pPlugin = pPlugin->pNext;
	}

	return nullptr;
}

using fPeer_Connected = bool(*)(const std::string& name);
//From MQ2DanNet exports.
bool IsPeerConnected(const std::string& szName) {
    MQPlugin* pDannet = FindMQ2DanNetPlugin();
    if (!pDannet)
        return false;

    fPeer_Connected peer_connected = (fPeer_Connected)GetProcAddress(pDannet->hModule, "peer_connected");
    if (peer_connected) {
        if (peer_connected(szName))
            return true;
    }

    return false;
}

inline bool InGame()
{
    return(GetGameState() == GAMESTATE_INGAME && GetCharInfo() && GetCharInfo()->pSpawn && GetPcProfile());
}

void SeeDanShow() {
    MakeWindowVisible();
};

void SeeDanHide() {
    MakeWindowInvisible();
};

void SeeDanToggle() {
    MakeWindowToggle();
};

//szQuery used for the commented out code block, if you /seedan test, and the function call includes a query, it will add it for all connected peers.
void FuckingAroundAgain(char* szQuery = "") {
    //Commented out code work to dump out all known information to the MQ2Window.
    //std::vector<std::string> peers = GetPeersList();
    //unsigned int count = GetPeersCount();

    //for (unsigned int i = 0; i < count; i++) {//For each Peer connected.
    //    AddObservation(peers.at(i).c_str(), szQuery);//Add the query.
    //}

    //for (unsigned int i = 0; i < count; i++) {//For each peer connected.
    //    WriteChatf("Name: %s", peers.at(i).c_str());
    //    std::vector<std::string> vObservations = GetObserveList(peers.at(i).c_str());
    //    unsigned int observecount = GetObserveCount(peers.at(i).c_str());

    //    for (unsigned int i = 0; i < observecount; i++) {//output each observation and it's value.
    //        WriteChatf("Observation: %s Value: %s", (char*)vObservations.at(i).c_str(), GetObserveValue(peers.at(i).c_str(), (char*)vObservations.at(i).c_str()));
    //    }
    //}

    //Currently this will attempt to add all the `Me.Buff[#].Name` and `Me.Song[#].Name` options for all peers in the peers list.
    if (InterfaceWnd) {
        InterfaceWnd->AddAllBuffs();
    }
}

char* GetServer() {
    if (EQADDR_SERVERNAME[0])
    {
        return EQADDR_SERVERNAME;
    }

    return "";
}

void TestCommand() {
    FuckingAroundAgain();
}

void SetupOptions() {//input all of these in lowercase.
	//Both of these show the window.
    options.insert(std::make_pair("on", SeeDanShow));
	options.insert(std::make_pair("show", SeeDanShow));

	//Both of these hide the window.
    options.insert(std::make_pair("off", SeeDanHide));
	options.insert(std::make_pair("hide", SeeDanHide));

	//Toggle the window's visibility.
    options.insert(std::make_pair("toggle", SeeDanToggle));

	//Run whatever crap I've smashed into the test command.
    options.insert(std::make_pair("test", TestCommand));
}

//prints all commands from the available options map. TODO: consider using a struct to create the command, the function it should fire, and the help to explain it's use.
void print_options() {//outputs all available options.
    std::map<std::string, fFunction>::iterator it;
    for (it = options.begin(); it != options.end(); it++) {
        WriteOut("%s", it->first.c_str());
    }
}

void SeeDannet(PSPAWNINFO pSpawn, char* szLine) {
    if (!strlen(szLine)) {//if the command `/seedan` was used without a parameter at the end, spit out available options.
        WriteOut("Invalid option");
        print_options();
        return;
    }

    char temp[64] = { 0 };
    strcpy_s(temp, szLine);
    _strlwr_s(temp, 64);
    auto result = options.find(temp);
    if (result != options.end()) {//if we found the parameter the user passed in the options, then run the function for that option.
        if (result->second) {
            result->second();//If so, fire the appropriate function.
        }
    } else {//otherwise, tell the user it wasn't valid, and output the available options.
        WriteOut("Invalid option - %s", szLine);
        print_options();
    }
}
// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
    SetupOptions();
    AddXMLFile("MQUI_DanNet.xml");
    AddCommand("/seedannet",SeeDannet);
    if (InGame())
        CreateInterfaceWindow();
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
    RemoveCommand("/seedannet");
}

PLUGIN_API VOID OnCleanUI(VOID)
{
    DestroyInterfaceWindow();
}

PLUGIN_API VOID OnReloadUI(VOID)
{
    if (InGame()) {
        CreateInterfaceWindow();
    }
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
    if (InGame()) {
        sprintf_s(ThisINIFileName, MAX_STRING, "%s\\MQ2DanInterface_%s.ini", gPathConfig, GetCharInfo()->Name);

        if (!InterfaceWnd)
            CreateInterfaceWindow();
    }
}

PLUGIN_API VOID OnPulse()
{
    if (InGame() && !WrongUI) {
        if (InterfaceWnd) {
            InterfaceWnd->UpdateListBox();
        }
    }
}