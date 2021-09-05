#pragma once
#include "..\MQ2Plugin.h"

void AddObservation(const char* szName, const char* szQuery, bool bSilent = 0);
void RemoveObservation(const char* szName, const char* szQuery);
char* GetObserveValue(const char* szName, const char* szQuery);
unsigned int GetPeersCount();
std::vector<std::string> GetPeersList();
unsigned int GetObserveCount(const char* szName);
std::vector<std::string> GetObserveList(const char* szName);
bool AlreadyObserved(const char* szName, const char* szQuery);