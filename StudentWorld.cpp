#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_bonusScore(1000), m_player(nullptr), m_amtCrystalsLeft(0), m_gameStatus(0)
{
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()
{
    //check valid level
	std::ostringstream oss;
	oss.fill('0');
	oss << std::setw(2) << getLevel();
	std::string result = oss.str();
	int loadLevelResult = loadLevel("level" + result + ".txt");
	if (loadLevelResult == Level::load_fail_file_not_found || getLevel() > 99) return GWSTATUS_PLAYER_WON;
	if (loadLevelResult == Level::load_fail_bad_format) return GWSTATUS_PLAYER_WON;
    m_bonusScore = 1000;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	m_gameStatus = GWSTATUS_CONTINUE_GAME;
    //UPDATE GAME TEXT
    string formattedText = formatInfo(getScore(), getLevel(), getLives(), m_player->getHealthPct(), m_player->getAmmo(), m_bonusScore);

    setGameStatText(formattedText);

    //ALL ACTORS DO SOMETHING IF NOT DEAD

	for (list<Actor*>::iterator itr = m_actorList.begin(); itr != m_actorList.end(); itr++) {
		(*itr)->doSomething();
		if (!m_player->isAlive()) { 
			decLives();
			while (decCrystals());
			return GWSTATUS_PLAYER_DIED;
		}
	}
	m_player->doSomething();

    //REMOVE DEAD ACTORS 
	destroyActorsIfDeadHelper();

    //REDUCE BONUS BY ONE
    if (m_bonusScore > 0) {
        m_bonusScore--;
    }


	return m_gameStatus;
}

void StudentWorld::cleanUp()
{	
	for (list<Actor*>::iterator itr = m_actorList.begin(); itr != m_actorList.end(); ) {
		delete (*itr);
		(*itr) = nullptr;
		itr = m_actorList.erase(itr);
	}
    delete m_player;
	m_player = nullptr;
}

bool StudentWorld::canAgentMoveTo(Agent* agent, double x, double y, int dx, int dy) const {
	list<Actor*> actorsAtPosition;
	double newX, newY;
	newX = x + dx;
	newY = y + dy;
	if (getActorsAtPosition(newX, newY, actorsAtPosition)){
		for (list<Actor*>::iterator itr = actorsAtPosition.begin(); itr != actorsAtPosition.end(); itr++) {
			newX += dx;
			newY += dy;
			if (agent->canPushMarbles() && (*itr)->bePushedBy(agent, (int)newX, (int)newY)) {
				return true;
			}
			if ((*itr) != nullptr && !(*itr)->allowsAgentColocation()) return false;
		}
	}
	return true;
}

bool StudentWorld::canMarbleMoveTo(int x, int y) const {
	list<Actor*> actorsAtPosition;
	if (getActorsAtPosition(x, y, actorsAtPosition)) {
		for (list<Actor*>::iterator itr = actorsAtPosition.begin(); itr != actorsAtPosition.end(); itr++) {
			if ((*itr) != nullptr && !(*itr)->allowsMarble()) return false;
		}
	}
	return true;
}

bool StudentWorld::swallowSwallowable(Actor* a) {
	bool swallowedOnThisTick = false;
	double pitX = a->getX();
	double pitY = a->getY();

	for (list<Actor*>::iterator itr = m_actorList.begin(); itr != m_actorList.end(); itr++) {
		if ((*itr)->isSwallowable() && (*itr)->getX() == pitX && (*itr)->getY() == pitY) {
			swallowedOnThisTick = true;
			break;
		}
	}
	if (!swallowedOnThisTick) return false;

	for (list<Actor*>::iterator itr = m_actorList.begin(); itr != m_actorList.end(); itr++) {
		if ((*itr)->getX() == pitX && (*itr)->getY() == pitY) {
			(*itr)->setDead();
		}
	}
	return true;
}

bool StudentWorld::existsClearShotToPlayer(int x, int y, int dx, int dy) const {
	double newX = (double)x + dx;
	double newY = (double)y + dy;
	if (isPlayerAtPosition(newX, newY)) return true;
	list<Actor*> actorsAtPosition;
	if (getActorsAtPosition(newX, newY, actorsAtPosition)) {
		for (list<Actor*>::iterator itr = actorsAtPosition.begin(); itr != actorsAtPosition.end(); itr++) {
			if ((*itr)->stopsPea() || (*itr)->isDamageable()) {
				return false;
			}
		}
	}
	return existsClearShotToPlayer(x + dx, y + dy, dx, dy);
}

bool StudentWorld::doFactoryCensus(int x, int y, int distance, int& count) const {
	for (list<Actor*>::const_iterator itr = m_actorList.begin(); itr != m_actorList.end(); itr++) {
		double actorX = (*itr)->getX();
		double actorY = (*itr)->getY();
		bool actorCountsInCensus = (*itr)->countsInFactoryCensus();

		int xDistance = findDistanceHelper(x, (int)actorX);
		int yDistance = findDistanceHelper(y, (int)actorY);

		if (actorCountsInCensus && xDistance == 0 && yDistance == 0) {
			return false;
		}
		
		if (actorCountsInCensus && xDistance <= distance && yDistance <= distance) { 
			count++;
		}
	}
	return true;
}

int StudentWorld::findDistanceHelper(int val1, int val2) const {
	if (val1 > val2) {
		return val1 - val2;
	}
	return val2 - val1;
}

Actor* StudentWorld::getColocatedStealable(int x, int y) const {
	list<Actor*> actorsAtPosition;
	if (getActorsAtPosition(x, y, actorsAtPosition)) {
		for (list<Actor*>::iterator itr = actorsAtPosition.begin(); itr != actorsAtPosition.end(); itr++) {
			if ((*itr)->isStealable()) return (*itr);
		}
	}
	return nullptr;
}

bool StudentWorld::damageSomething(Actor* a, int damageAmt) {
	double peaX = a->getX();
	double peaY = a->getY();
	list<Actor*> actorsAtPosition;
	bool damagedSomething = false;

	if (isPlayerColocatedWith(a)) {
		m_player->damage(damageAmt);
		a->setDead();
		damagedSomething = true;
	}
	if (getActorsAtPosition(peaX, peaY, actorsAtPosition)) {
		for (list<Actor*>::iterator itr = actorsAtPosition.begin(); itr != actorsAtPosition.end(); itr++) {
			if ((*itr)->isDamageable()) {
				(*itr)->damage(damageAmt);
				a->setDead();
				damagedSomething = true;
			}
			if ((*itr)->stopsPea()) {
				a->setDead();
				damagedSomething = true;
			}
		}
	}
	return damagedSomething;
}

void StudentWorld::destroyActorsIfDeadHelper() {
	for (list<Actor*>::iterator itr = m_actorList.begin(); itr != m_actorList.end();) {
		if (!(*itr)->isAlive()) {
			delete (*itr);
			(*itr) = nullptr;
			itr = m_actorList.erase(itr);
		}
		else {
			itr++;
		}
	}
}

bool StudentWorld::anyCrystals() const {
	return m_amtCrystalsLeft > 0;
}

bool StudentWorld::decCrystals() {
	if (m_amtCrystalsLeft <= 0) return false;
	m_amtCrystalsLeft--;
	return true;
}

void StudentWorld::restorePlayerHealth() {
	m_player->restoreHealth();
}

void StudentWorld::increaseAmmo() {
	m_player->increaseAmmo();
}

void StudentWorld::setLevelFinished() {
	playSound(SOUND_FINISHED_LEVEL);
	increaseScore(2000 + getBonus());
	m_gameStatus = GWSTATUS_FINISHED_LEVEL;
}

bool StudentWorld::getActorsAtPosition(double x, double y, list<Actor*>& actorsAtPosition) const {
	for (list<Actor*>::const_iterator itr = m_actorList.begin(); itr != m_actorList.end(); itr++) {
		if ((*itr)->getX() == x && (*itr)->getY() == y) {
		actorsAtPosition.push_back(*itr);
		}
	}
	if (isPlayerAtPosition(x, y)) actorsAtPosition.push_back(m_player);
	if (actorsAtPosition.empty()) return false;
	return true;
}

bool StudentWorld::isPlayerColocatedWith(Actor* a) const {
	if (m_player->getX() == a->getX() && m_player->getY() == a->getY()) return true;
	return false;
}

bool StudentWorld::isPlayerAtPosition(double x, double y) const {
	if (m_player->getX() == x && m_player->getY() == y) return true;
	return false;
}

Player* StudentWorld::getPlayer() {
	return m_player;
}

void StudentWorld::addActor(Actor* actor) {
	m_actorList.push_back(actor);
}

int StudentWorld::getBonus() const {
	return m_bonusScore;
}

int StudentWorld::loadLevel(std::string levelName) {
	Level lev(assetPath());
	Level::LoadResult result = lev.loadLevel(levelName);
	if (result == Level::load_fail_file_not_found) {
		std::cerr << "Could not find " << levelName << " data file\n";
		return Level::load_fail_file_not_found;
	}
	else if (result == Level::load_fail_bad_format) {
		std::cerr << "Your level was improperly formatted\n";
		return Level::load_fail_bad_format;
	}
	else if (result == Level::load_success) {
		std::cerr << "Successfully loaded level\n";
	}

	for (int y = 0; y <= 14; y++) {
		for (int x = 0; x <= 14; x++) {
			double actorX, actorY;
			actorX = x;
			actorY = y;
			Level::MazeEntry item = lev.getContentsOf(x, y);
			switch (item) {
			case Level::ammo:
				m_actorList.push_back(new AmmoGoodie(this, x, y));
				break;
			case Level::crystal:
				m_amtCrystalsLeft++;
				m_actorList.push_back(new Crystal(this, x, y));
				break;
			case Level::exit:
				m_actorList.push_back(new Exit(this, x, y));
				break;
			case Level::extra_life:
				m_actorList.push_back(new ExtraLifeGoodie(this, x, y));
				break;
			case Level::horiz_ragebot:
				m_actorList.push_back(new RageBot(this, x, y, 0));
				break;
			case Level::marble:
				m_actorList.push_back(new Marble(this, x, y));
				break;
			case Level::mean_thiefbot_factory:
				m_actorList.push_back(new ThiefBotFactory(this, x, y, ThiefBotFactory::MEAN));
				break;
			case Level::pit:
				m_actorList.push_back(new Pit(this, x, y));
				break;
			case Level::player:
				m_player = new Player(this, x, y);
				break;
			case Level::restore_health:
				m_actorList.push_back(new RestoreHealthGoodie(this, x, y));
				break;
			case Level::thiefbot_factory:
				m_actorList.push_back(new ThiefBotFactory(this, x, y, ThiefBotFactory::REGULAR));
				break;
			case Level::vert_ragebot:
				m_actorList.push_back(new RageBot(this, x, y, 270));
				break;
			case Level::wall:
				m_actorList.push_back(new Wall(this, x, y));
				break;
			}
		}
	}
	return 0;
}

std::string StudentWorld::formatInfo(int score, int level, int lives, int health, int numPeas, int bonus) {
	std::ostringstream oss;

	oss.fill('0');
	oss << "Score: " << std::setw(7) << score << "  ";
	oss << "Level: " << std::setw(2) << level << "  ";

	oss.fill(' ');
	oss << "Lives: " << std::setw(2) << lives << "  ";

	oss << "Health " << std::setw(3) << health << "%  ";

	oss << "Ammo: " << std::setw(3) << numPeas << "  ";
	oss << "bonus: " << std::setw(4) << bonus << "  ";

	std::string result = oss.str();

	return result;
}

