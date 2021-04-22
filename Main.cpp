#define _CRT_SECURE_NO_WARNINGS// for MSVC
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <ctime>//for time
#include <Windows.h>//for Sleep
#include "OregonTrail.hpp"

int g_myID;
int Random(int min, int max) {
	int rng = rand() & 0x7FFFFFFF;//not guaranteed to provide an unsigned number
	return rng % max + min;
}
int Random(int max) {
	return Random(0, max);
}

std::vector<std::string> StringSplit(const std::string& delimiter, const std::string& str)
{
	std::vector<std::string> arr;

	int strleng = str.length();
	int delleng = delimiter.length();
	if (delleng == 0)
		return arr;//no change

	int i = 0;
	int k = 0;
	while (i < strleng)
	{
		int j = 0;
		while (i + j < strleng && j < delleng && str[i + j] == delimiter[j])
			j++;
		if (j == delleng)//found delimiter
		{
			arr.push_back(str.substr(k, i - k));
			i += delleng;
			k = i;
		}
		else
		{
			i++;
		}
	}
	arr.push_back(str.substr(k, i - k));
	return arr;
}
OregonTrailGame* g_pGame = nullptr;

OregonTrailGame* GetGame() {
	if (!g_pGame)
		g_pGame = new OregonTrailGame();

	return g_pGame;
}

void KillGame() {
	if (g_pGame)
		delete g_pGame;
	g_pGame = nullptr;
}

void LogMsg(const char* pFmt, ...) {
	char buffer[5000];
	va_list list;
	va_start(list, pFmt);
	vsprintf(buffer, pFmt, list);
	va_end(list);
	// write to output
	printf("%s\n", buffer);
}
int main()
{
	srand(time(nullptr));
	GetGame()->Init();

	g_myID = GetGame()->GetLocalPlayerID();

	LogMsg("Your player ID is: %d", g_myID);
	// also let us generate a player data for them
	bool garbage;
	GetGame()->AddNewPlayer(g_myID, garbage);

	while (GetGame()->GameRun())
		Sleep(1);

	KillGame();
}