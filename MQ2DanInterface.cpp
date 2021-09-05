// MQ2DanInterface.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.


/*
Still needed.
An input box with a button (Add observation) to easily add an observation.
A way to click on an option in the above mentioned listbox, and a button to click to remove the observation.
a Listbox to show channels
a Listbox to show existing Observations and the current value of it.
a Listbox to show currently connected peers.

Move things from the test function to the interface.

Reading Material:
Dannet Webpage: https://www.redguides.com/community/resources/mq2dannet.322/

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
	while (pPlugin)
	{
		if (!_stricmp("MQ2Dannet", pPlugin->szFilename))
		{
			return pPlugin;
		}

		pPlugin = pPlugin->pNext;
	}

	return nullptr;
}

using fPeer_Connected = bool(*)(const std::string& name);//
bool IsPeerConnected(const std::string& szName) {
    MQPlugin* pDannet = FindMQ2DanNetPlugin();
    if (!pDannet)
        return false;

    fPeer_Connected peer_connected = (fPeer_Connected)GetProcAddress(pDannet->hModule, "peer_connected");
    if (peer_connected)
    {
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

void FuckingAroundAgain(char* szQuery = "") {
    //static bool firstuse = true;
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
void print_options()
{
    std::map<std::string, fFunction>::iterator it;
    for (it = options.begin(); it != options.end(); it++)
    {
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
            result->second();
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

// Called once directly before shutdown of the new ui system, and also
// every time the game calls CDisplay::CleanGameUI()
PLUGIN_API VOID OnCleanUI(VOID)
{
    DestroyInterfaceWindow();
}

// Called once directly after the game ui is reloaded, after issuing /loadskin
PLUGIN_API VOID OnReloadUI(VOID)
{
    if (InGame()) {
        CreateInterfaceWindow();
    }
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
    DebugSpewAlways("MQ2DanInterface::SetGameState()");
    if (InGame()) {
        sprintf_s(ThisINIFileName, MAX_STRING, "%s\\MQ2DanInterface_%s.ini", gPathConfig, GetCharInfo()->Name);

        if (!InterfaceWnd)
            CreateInterfaceWindow();
    }
}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse()
{
    if (InGame() && !WrongUI) {
        if (InterfaceWnd) {
            InterfaceWnd->UpdateListBox();
        }
    }
}

// This is called when we receive the EQ_END_ZONE packet is received
PLUGIN_API VOID OnEndZone(VOID)
{
//    WriteChatf("OnEndZone %llu", GetTickCount64());
//    DebugSpewAlways("MQ2DanInterface::OnEndZone");
}
// This is called when pChar!=pCharOld && We are NOT zoning
// honestly I have no idea if its better to use this one or EndZone (above)
PLUGIN_API VOID Zoned(VOID)//this never outputs anything that I've seen. Not sure "Zoned" is the same as OnEndZone anymore, or it's not being called?
{
//    WriteChatf("OnZoned %llu", GetTickCount64());
//    DebugSpewAlways("MQ2DanInterface::Zoned");
}