#pragma once
#include "DanFunctions.h"
#include "DanInterfaceWnd.h"

void AddObservation(const char* szName, const char* szQuery, bool bSilent) {

    if (!AlreadyObserved(szName, szQuery)) {
        char dobservecmd[MAX_STRING] = { 0 };
        sprintf_s(dobservecmd, "/dobserve %s -q %s", szName, szQuery);
        WriteOut("\ap%s \agAdded \ay%s", szName, szQuery);
        EzCommand(dobservecmd);
    }
    else {
        if (!bSilent) WriteOut("%s is already observing %s", szName, szQuery);
    }
}

void RemoveObservation(const char* szName, const char* szQuery) {
    char dobservecmd[MAX_STRING] = { 0 };
    sprintf_s(dobservecmd, "/dobserve %s -q %s -drop", szName, szQuery);
    WriteOut("\ap%s \arRemoved \ay%s", szName, szQuery);
    EzCommand(dobservecmd);
}

char* GetObserveValue(const char* szName, const char* szQuery) {
    static char result[MAX_STRING] = { 0 };
    sprintf_s(result, "${DanNet[%s].Observe[\"%s\"]}", szName, szQuery);
    if (ParseMacroData(result, sizeof(result))) {
        return result;
    }
    //ParseMacroData failed.
    return "";
}

unsigned int GetPeersCount() {
    char result[MAX_STRING] = { 0 };
    sprintf_s(result, "${DanNet.PeerCount}");
    ParseMacroData(result, sizeof(result));
    return atoi(result);
}

std::vector<std::string> GetPeersList() {
    char result[MAX_STRING * 2] = { 0 };
    std::vector<std::string> vVector;
    sprintf_s(result, "${DanNet.Peers}");
    if (ParseMacroData(result, sizeof(result))) {
        unsigned int count = GetPeersCount();

        for (unsigned int i = 1; i < count + 1; i++) {
            char temp[MAX_STRING] = { 0 };
            GetArg(temp, result, i, 0, 0, 0, '|');

            if (strlen(temp)) {
                vVector.push_back(temp);
            }
        }
    }

    return vVector;
}

unsigned int GetObserveCount(const char* szName) {
    char result[MAX_STRING] = { 0 };
    sprintf_s(result, "${DanNet[%s].ObserveCount}", szName);
    ParseMacroData(result, sizeof(result));
    return atoi(result);
}

std::vector<std::string> GetObserveList(const char* szName) {
    char result[MAX_STRING * 2] = { 0 };
    std::vector<std::string> vVector;
    sprintf_s(result, "${DanNet[%s].Observe}", szName);
    if (ParseMacroData(result, sizeof(result))) {
        unsigned int count = GetObserveCount(szName);

        for (unsigned int i = 1; i < count + 1; i++) {
            char temp[MAX_STRING] = { 0 };
            GetArg(temp, result, i, 0, 0, 0, '|');

            if (strlen(temp)) {
                vVector.push_back(temp);
            }
        }
    }

    return vVector;
}

bool AlreadyObserved(const char* szName, const char* szQuery) {
    std::vector<std::string> vVector = GetObserveList(szName);
    for (unsigned int i = 0; i < vVector.size(); i++) {
        if (!_stricmp(szQuery, vVector.at(i).c_str()))
            return true;
    }
    return false;
}