#include "OregonTrail.hpp"

// inspired by jefftastic's project, see here:
// https://github.com/jefftasticgames/school-projects/blob/main/oregon.py
// The reason why I'm writing this so split away like this is because
// I plan on making mods on this game + porting it to platforms that may not 
// have this type of i/o.


/**
	TrailPlayer - Controls a player's data directly
**/

void TrailPlayer::OnSickness() {
	m_sicknessesThisMonth++;
	m_healthLevel--;
	m_game->SendLogMsg(m_id, "Oh no! You got sick and lost 1 HP.");
}
void TrailPlayer::OnConsumeFood() {
	m_foodRemaining -= m_appetite;
}
void TrailPlayer::RandomSicknessOccurs() {
	int daysLeft = m_game->GetDaysPerMonth(m_month) - m_day;
	int randSick = 1;
	if (daysLeft >= 1) {
		randSick = Random(daysLeft);
	}

	if (m_sicknessesThisMonth == 0) {
		if (randSick < 2) OnSickness();
	}
	else if (m_sicknessesThisMonth == 1) {
		if (randSick < 1) OnSickness();
	}
}
void TrailPlayer::RandomEventOccurs() {
	if (Random(25) == 0)
		if (!m_randomEventsThisMonth)
			OnRandomEffect();
}
void TrailPlayer::OnRandomEffect() {
	RandomEventType type = (RandomEventType)(rand() % 4);
	switch (type) {
	case RandomEventType::Flood: {
		int randomDays = Random(MIN_DAYS_PER_RANDOM_EVENT, MAX_DAYS_PER_RANDOM_EVENT);
		int randomFood = Random(MIN_FOOD_PER_RANDOM_EVENT, MAX_FOOD_PER_RANDOM_EVENT);
		m_foodRemaining -= randomFood;
		AdvanceGameClock(randomDays);
		m_game->SendLogMsg(m_id, "A rain shower came through and flooded your wagon! You lost %d lbs. of food and took %d days to recover.", randomFood, randomDays);
		break;
	}
	case RandomEventType::Dysentery: {
		int randomDays = Random(MIN_DAYS_PER_RANDOM_EVENT, MAX_DAYS_PER_RANDOM_EVENT);
		int randomHealth = Random(1, 2);
		m_healthLevel -= randomHealth;
		AdvanceGameClock(randomDays);
		m_game->SendLogMsg(m_id, "You fell sick to dysentery and lost %d HP. You took %d days to recover.", randomHealth, randomDays);
		break;
	}
	case RandomEventType::Mugging: {
		m_gameState = GameState::BeingMugged;
		break;
	}
	case RandomEventType::Stash: {
		int randomFood = Random(MIN_FOOD_PER_RANDOM_EVENT, MAX_FOOD_PER_RANDOM_EVENT);
		m_foodRemaining += randomFood;
		m_game->SendLogMsg(m_id, "LUCKY DAY! You found a stash, which contains %d lbs. of food!", randomFood);
		break;
	}
	}
	m_randomEventsThisMonth++;
}
bool TrailPlayer::DidDaysRollover() {
	return(m_day > m_game->GetDaysPerMonth(m_month));
}

void TrailPlayer::AdvanceGameClock(int days) {
	while (days > 0) {
		if (IsGameOver()) days = 0;
		else {
			m_day += 1;
			if (DidDaysRollover()) {
				m_sicknessesThisMonth = 0;
				m_randomEventsThisMonth = 0;
				m_day = 1;
				m_month++;
			}
			OnConsumeFood();
			RandomSicknessOccurs();
			RandomEventOccurs();
			days--;
		}
	}
}

void TrailPlayer::OnTravel() {
	int randMiles = Random(MIN_MILES_PER_TRAVEL, MAX_MILES_PER_TRAVEL),
		randDays  = Random(MIN_DAYS_PER_TRAVEL,  MAX_DAYS_PER_TRAVEL);
	m_milesTraveled += randMiles;
	m_appetite = 5;
	AdvanceGameClock(randDays);
	if (!CheckPlayerWin())
		m_game->SendLogMsg(m_id, "You traveled %d miles in %d days! Only %d miles remain.", randMiles, randDays, MILES_BETWEEN_NYC_AND_OREGON - m_milesTraveled);
	else
		m_game->SendLogMsg(m_id, "You traveled %d miles in %d days!", randMiles, randDays);
}

void TrailPlayer::OnRest() {
	if (m_healthLevel < MAX_HEALTH) {
		int randDays = Random(MIN_DAYS_PER_REST, MAX_DAYS_PER_REST);
		m_appetite = 3;
		m_healthLevel += HEALTH_CHANGE_PER_REST;
		if (m_healthLevel > MAX_HEALTH) m_healthLevel = MAX_HEALTH;
		m_game->SendLogMsg(m_id, "You rest for %d days and gain 1 HP.", randDays);
	}
	else {
		m_game->SendLogMsg(m_id, "No need to rest, you're just fine.");
	}
}

void TrailPlayer::OnHunt() {
	int randDays = Random(MIN_DAYS_PER_HUNT, MAX_DAYS_PER_HUNT);
	m_foodRemaining += FOOD_PER_HUNT;
	m_appetite = 8;
	m_game->SendLogMsg(m_id, "You hunted for %d days, gathering %d pounds of food.", randDays, FOOD_PER_HUNT);
	AdvanceGameClock(randDays);
}

void TrailPlayer::PrintStatus() {
	m_game->SendLogMsg(m_id, "Food: %d\nHealth: %d\nDistance traveled so far: %d miles\nDate: %s %d, 1850",
			m_foodRemaining, m_healthLevel, m_milesTraveled, m_game->GetMonthName(m_month), m_day);
}

bool TrailPlayer::CheckPlayerWin() {
	return (m_milesTraveled >= MILES_BETWEEN_NYC_AND_OREGON);
}
bool TrailPlayer::CheckPlayerLose() {
	if (m_day >= 31 && m_month >= 12) {
		LogMsg("[DEBUG] Player %s (%d) lost the game by being late", m_name.c_str(), m_id);
		return true;
	}
	if (m_healthLevel <= 0) {
		LogMsg("[DEBUG] Player %s (%d) lost the game by having 0 HP", m_name.c_str(), m_id);
		return true;
	}
	if (m_foodRemaining <= 0) {
		LogMsg("[DEBUG] Player %s (%d) lost the game by being starved to death!", m_name.c_str(), m_id);
		return true;
	}
	return false;
}

bool TrailPlayer::IsGameOver() {
	return (CheckPlayerLose() || CheckPlayerWin());
}

void TrailPlayer::PrintHelp() {
	m_game->SendHelpText(m_id);
}

void TrailPlayer::LossReport() {
	m_game->SendLogMsg(m_id, "Alas, you lost! Here's what you had:\nFood: %d\nHealth: %d\nDistance traveled so far: %d",
		m_foodRemaining, m_healthLevel, m_milesTraveled);
}
void TrailPlayer::WinReport() {
	m_game->SendLogMsg(m_id, "YOU WON! And you made it to Oregon alive. Here's what you have:\nFood: %d\nHealth: %d\nDate of arrival: %s %d",
		m_foodRemaining, m_healthLevel, m_game->GetMonthName(m_month), m_day);
}

/**
	OregonTrailGame - Controls the game itself
**/

const int g_daysPerMonth[] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const char* g_monthNames[] = { "What?!?", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
int OregonTrailGame::GetDaysPerMonth(int monthID) {
	return g_daysPerMonth[monthID];
}
const char* OregonTrailGame::GetMonthName(int monthID) {
	return g_monthNames[monthID];
}

void OregonTrailGame::AskForInput(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	pData->m_inputAvailable = false;
	char line[5001];
	std::cin.getline(line, 5000, '\n');
	SendInputToGame(id, std::string(line));
}
void OregonTrailGame::SendLogMsg(int id, const char* pFmt, ...) {
	char buffer[5000];
	va_list list;
	va_start(list, pFmt);
	vsprintf(buffer, pFmt, list);
	va_end(list);
	// write to output
	printf("[P%d]: %s\n", id, buffer);
}
void OregonTrailGame::LogMsgAll(const char* pFmt, ...) {
	char buffer[5000];
	va_list list;
	va_start(list, pFmt);
	vsprintf(buffer, pFmt, list);
	va_end(list);
	// write to output
	printf("[ALL]: %s\n", buffer);
}

void OregonTrailGame::SendInputToGame(int id, std::string input) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	pData->m_inputAvailable = true;
	pData->m_input.push(input);
}

void OregonTrailGame::OnPlayerQuit(int id) {
	// player quit?

	RemovePlayer(id);
}

bool OregonTrailGame::ReadInputFromQueue(int id, std::string& command) {
	auto pData = GetPlayerForID(id);
	if (!pData) return false;
	// read the command from the queue
	if (pData->m_input.empty() || !pData->m_inputAvailable) {
		pData->m_inputAvailable = false;
		return false;//nothing in queue
	}
	// We have something in the queue?!  That's great, read it in
	command = pData->m_input.front();
	pData->m_input.pop();
	pData->m_inputAvailable = !pData->m_input.empty();
	return true;
}
void OregonTrailGame::SendIntroText(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	SendLogMsg(id, "Welcome to the Oregon Trail, %s! The year is 1850 and Americans are headed out "
		"West to populate the frontier. Your goal is to travel by wagon train from "
		"Independence, MO to Oregon (roughly 2000 miles). You start on March 1st, and "
		"your goal is to reach Oregon by Dec 31st. The trail is arduous. Each day costs "
		"you food and health. You can hunt and rest, but you have to get there before "
		"winter!\n", pData->m_name.c_str());
}
void OregonTrailGame::SendHelpText(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	SendLogMsg(id, "Each turn you can take one of these 3 actions:\n"
		"  [t]ravel  - moves you randomly between %d-%d miles and takes %d-%d days\n"
		"  [r]est    - increases %d HP (up to %d maximum) and takes %d-%d days\n"
		"  [h]unt    - adds %d lbs of food and takes %d-%d days\n\n"
		"When prompted for an action, you may also enter one of the following commands without using up your turn:\n"
		"  [s]tatus  - lists food, health, distance traveled (progress) and current day\n"
		"  ? or help - prints this screen\n"
		"  [q]uit    - will end the game\n\n"
		"NOTE: You may also use one of the shortcuts, just type the letter correspondent of the action or ? for help.\n", 
		pData->m_name.c_str(),
		MIN_MILES_PER_TRAVEL,
		MAX_MILES_PER_TRAVEL,
		MIN_DAYS_PER_TRAVEL,
		MAX_DAYS_PER_TRAVEL,
		HEALTH_CHANGE_PER_REST,
		MAX_HEALTH,
		MIN_DAYS_PER_REST,
		MAX_DAYS_PER_REST,
		FOOD_PER_HUNT,
		MIN_DAYS_PER_HUNT,
		MAX_DAYS_PER_HUNT);
}
void OregonTrailGame::SendGoodLuckText(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	SendLogMsg(id, "Good luck, and see you in Oregon!", pData->m_name.c_str());
}
bool OregonTrailGame::ProcessCommand(int id, std::string command) {
	auto pData = GetPlayerForID(id);
	if (!pData) return true;
	auto split = StringSplit("|", command);
	if (split.size() < 0) {
		SendLogMsg(id, "No command specified.");
		return false;
	}
	switch (pData->m_gameState) {
	case GameState::NameAsk: {

		// got their name
		pData->m_name = split[0];
		SendIntroText(id);
		SendHelpText(id);
		SendGoodLuckText(id);

		pData->m_gameState = GameState::Normal;
		break;
	}
	case GameState::Normal: {
		// accept commands
		if (split[0].size() > 0) {
			switch (split[0][0]) {
			case 'h':
			case 'H':
			{
				bool isHunt = true;
				if (split[0].size() > 2)
					if (split[0][1] == 'e' || split[0][1] == 'E')
						isHunt = false;
				if (isHunt) {
					pData->OnHunt();
					break;
				}
			}
			case '?': {
				SendHelpText(id);
				break;
			}
			case 'q':case'Q': {
				pData->m_gameState = GameState::Quitting;
				break;
			}
			case 's':case'S': {
				pData->PrintStatus();
				break;
			}
			case 'r':case'R': {
				pData->OnRest();
				break;
			}
			case 't':case'T': {
				pData->OnTravel();
				break;
			}
			default: {
				SendLogMsg(id, "Invalid command, use ? for help");
			}
			}
		}
		break;
	}
	case GameState::BeingMugged: {
		if (split[0].size() > 0) {
			int randomFood = Random(MIN_FOOD_PER_RANDOM_EVENT, MAX_FOOD_PER_RANDOM_EVENT);
			if (split[0][0] == 'y' || split[0][0] == 'Y')
			{
				// For some reason, they let the outlaws rob them?
				SendLogMsg(id, "You let the outlaws take your stuff and lost %d lbs. of food.", randomFood);
				pData->m_gameState = GameState::Normal;
			}
			else if (split[0][0] == 'n' || split[0][0] == 'N') 
			{
				// Trying to defend yourself
				SendLogMsg(id, "You pull out your hunting rifle to defend yourself.");
				if (Random(1)) {
					pData->m_healthLevel -= 3;
					SendLogMsg(id, "You got shot, and lost 3 HP.");
				}
				else {
					SendLogMsg(id, "You shot the outlaws and gained %d lbs. of food.", randomFood);
					pData->m_foodRemaining += randomFood;
				}
				pData->m_gameState = GameState::Normal;
			}
			else SendLogMsg(id, "That is not a valid response to being mugged. Try again.");
			//SendLogMsg(id, "Outlaws approach your wagon, do you want to let them take your food?! (Y/N)");
		}
		break;
	}
	case GameState::Quitting: {
		if (split[0].size() > 0) {
			if (split[0][0] == 'y' || split[0][0] == 'Y')
			{
				SendLogMsg(id, "Quitting...");
				return true;
			}
		}
		break;
	}
	}
	return false;
}

// One update of the game, this returns false when we win.
bool OregonTrailGame::UpdateForPlayer(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return false;

	// Do we have anything in the queue?
	while (pData->m_inputAvailable) {
		// Yes, process it
		std::string cmd;
		if (ReadInputFromQueue(id, cmd))
		{
			bool quitting = ProcessCommand(id, cmd);
			if (quitting) return false;
		}
		else
			SendLogMsg(id, "WTF?! m_inputAvailable is true but we couldn't read from command?!");
	}

	if (pData->IsGameOver()) {
		if (pData->CheckPlayerWin()) {
			pData->WinReport();
		}
		else if (pData->CheckPlayerLose()) {
			pData->LossReport();
		}
		else {
			LogMsg("ERROR: %s's game is over by unknown way", pData->m_name.c_str());
		}
		return false;//leave the game
	} else {
		switch (pData->m_gameState) {
		case GameState::NameAsk: {
			SendLogMsg(id, "What is your name, traveler?!");
			break;
		}
		case GameState::Normal: {
			SendLogMsg(id, "Write a command, or ? for help.");
			break;
		}
		case GameState::BeingMugged: {
			SendLogMsg(id, "Outlaws approach your wagon, do you want to let them take your food?! (Y/N)");
			break;
		}
		case GameState::Quitting: {
			SendLogMsg(id, "Really quit?! (y/N) -- typing anything other than y or Y will not quit");
			break;
		}
		}
	}

#ifdef _CONSOLE
	// Now that we don't, since we are in console mode, ask player for input
	//else {
	AskForInput(id);
	//}
#endif

	return true;// For now we've not won
}
bool OregonTrailGame::GameRun() {
restart_the_loop:
	for (auto kvp : m_Player) {
		bool k = UpdateForPlayer(kvp.first);
		if (!k) {
			m_Player.erase(kvp.first);
			goto restart_the_loop;
		}
	}
	if (m_Player.empty()) return false;
	return true;
}
TrailPlayer* OregonTrailGame::GetPlayerForID(int id) {
	if (m_Player.count(id))
		return m_Player[id];

	// player left?
	return nullptr;
}
void OregonTrailGame::Init() {
	m_localPlayerID = -1;
}
int OregonTrailGame::GetLocalPlayerID() {
	if (m_localPlayerID == -1) {
		m_localPlayerID = rand() & 0xFF;
	}
	return m_localPlayerID;
}
TrailPlayer* OregonTrailGame::AddNewPlayer(int id, bool& createdNewPlayer) {
	auto pData = GetPlayerForID(id);
	if (pData) return pData;
	pData = new TrailPlayer();
	pData->m_id = id;
	pData->m_game = this;
	m_Player[id] = pData;
	LogMsgAll("Player %s joined the game.", pData->m_name.c_str());
	return pData;
}
void OregonTrailGame::RemovePlayer(int id) {
	auto pData = GetPlayerForID(id);
	if (!pData) return;
	m_Player.erase(id);
	LogMsgAll("Player %d left the game.", pData->m_name.c_str());
	delete pData;
}