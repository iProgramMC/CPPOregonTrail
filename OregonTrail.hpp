#pragma once
#define _CRT_SECURE_NO_WARNINGS// for MSVC
#include <string>
#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <stdarg.h>

void LogMsg(const char* pFmt, ...);
std::vector<std::string> StringSplit(const std::string& delimiter, const std::string& str);
int Random(int min, int max);
int Random(int max);

#define MIN_MILES_PER_TRAVEL 30
#define MAX_MILES_PER_TRAVEL 90
#define MIN_DAYS_PER_TRAVEL 3
#define MAX_DAYS_PER_TRAVEL 7

#define MIN_DAYS_PER_REST 2
#define MAX_DAYS_PER_REST 5
#define HEALTH_CHANGE_PER_REST 1
#define MAX_HEALTH 5

#define MIN_DAYS_PER_RANDOM_EVENT 1
#define MAX_DAYS_PER_RANDOM_EVENT 10
#define MIN_FOOD_PER_RANDOM_EVENT 1
#define MAX_FOOD_PER_RANDOM_EVENT 100

#define FOOD_PER_HUNT 100
#define MIN_DAYS_PER_HUNT 2
#define MAX_DAYS_PER_HUNT 5

#define MILES_BETWEEN_NYC_AND_OREGON 2000

enum class GameState {
	Normal,
	Quitting,
	NameAsk,
	BeingMugged,
};
enum class RandomEventType {
	Flood,
	Dysentery,
	Mugging,
	Stash
};

class OregonTrailGame;
class TrailPlayer {
public:
	int m_id;
	OregonTrailGame* m_game;

	std::queue<std::string> m_input;
	bool m_inputAvailable = false;
	bool m_requestingInput = false;
	std::string m_name = "Player";
	GameState m_gameState = GameState::NameAsk;

	int m_milesTraveled = 0;
	int m_foodRemaining = 500;
	int m_healthLevel = 5;
	int m_appetite = 5;
	int m_month = 3;
	int m_day = 1;
	int m_sicknessesThisMonth = 0;
	int m_randomEventsThisMonth = 0;
	bool m_playerWin = false;

	void OnSickness();
	void RandomSicknessOccurs();
	void OnConsumeFood();
	void OnRandomEffect();
	void RandomEventOccurs();
	void AdvanceGameClock(int days);
	bool DidDaysRollover();
	bool IsGameOver();
	void OnTravel();
	void OnRest();
	void OnHunt();
	void PrintStatus();
	void PrintHelp();
	bool CheckPlayerWin();
	bool CheckPlayerLose();
	void LossReport();
	void WinReport();
};

class OregonTrailGame
{
private:
	std::map<int, TrailPlayer*> m_Player;
	int m_localPlayerID;

public:
	TrailPlayer* GetPlayerForID(int id);
	int GetLocalPlayerID();
	void Init();
	void AskForInput(int id);
	void SendInputToGame(int id, std::string input);
	bool ReadInputFromQueue(int id, std::string& command);
	bool UpdateForPlayer(int id);
	bool GameRun();
	bool ProcessCommand(int id, std::string command);
	void SendLogMsg(int id, const char* text, ...);
	void SendIntroText(int id);
	void SendHelpText(int id);
	void SendGoodLuckText(int id);
	void SendAboutText(int id);
	void LogMsgAll(const char* text, ...);
	TrailPlayer* AddNewPlayer(int id, bool& createdNewPlayer);
	void RemovePlayer(int id);
	void OnPlayerQuit(int id);
	int GetDaysPerMonth(int monthID);
	const char* GetMonthName(int monthID);
};

