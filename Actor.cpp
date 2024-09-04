#include "Actor.h"
#include "StudentWorld.h"
#include <iostream>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
/*
----------------------------------------------------------------------------
Actor Declarations
----------------------------------------------------------------------------
*/
Actor::Actor(StudentWorld* world, int startX, int startY, int imageID, int hitPoints, int startDir) : 
	GraphObject(imageID, startX, startY, startDir), m_hitpoints(hitPoints), m_alive(true), m_world(world) 
	{ setVisible(true); }
bool Actor::isAlive() const { return m_alive; }
void Actor::setDead() { m_alive = false; }
void Actor::decHitPoints(int amt) { m_hitpoints -= amt; }
StudentWorld* Actor::getWorld() const { return m_world; }
void Actor::damage(int damageAmt) { decHitPoints(damageAmt); }
int Actor::getHitPoints() const { return m_hitpoints; }
void Actor::setHitPoints(int amt) { m_hitpoints = amt; }
bool Actor::tryToBeKilled(int damageAmt) {
	decHitPoints(damageAmt);
	if (m_hitpoints <= 0) setDead();
	return !isAlive();
}

/*
----------------------------------------------------------------------------
Agent Declarations
----------------------------------------------------------------------------
*/

Agent::Agent(StudentWorld* world, int startX, int startY, int imageID, int hitPoints, int startDir) : 
	Actor(world, startX, startY, imageID, hitPoints, startDir) {}
bool Agent::moveIfPossible() {
	int dir = this->getDirection();
	switch (dir) {
	case up:
		return handleMovement(0, 1);
		break;
	case down:
		return handleMovement(0, -1);
		break;
	case left:
		return handleMovement(-1, 0);
		break;
	case right:
		return handleMovement(1, 0);
		break;
	}
	return false;
}
bool Agent::handleMovement(int dx, int dy) {
	if (getWorld()->canAgentMoveTo(this, this->getX(), this->getY(), dx, dy)) {
		moveTo(this->getX() + dx, this->getY() + dy);
		return true;
	}
	else {
		return false;
	}
}

/*
----------------------------------------------------------------------------
Player Declarations
----------------------------------------------------------------------------
*/

Player::Player(StudentWorld* world, int startX, int startY) : 
	Agent(world, startX, startY, IID_PLAYER, 20, right), m_ammo(20) {}
void Player::doSomething() {
	int key;
	if (getWorld()->getKey(key)) {
		switch (key) {
		case KEY_PRESS_UP:
			setDirection(up);
			moveIfPossible();
			break;
		case KEY_PRESS_DOWN:
			setDirection(down);
			moveIfPossible();
			break;
		case KEY_PRESS_LEFT:
			setDirection(left);
			moveIfPossible();
			break;
		case KEY_PRESS_RIGHT:
			setDirection(right);
			moveIfPossible();
			break;
		case KEY_PRESS_SPACE:
			if (m_ammo > 0) {
				getWorld()->playSound(shootingSound());
				addPeaInFront();
				m_ammo--;
			}
			break;
		case KEY_PRESS_ESCAPE:
			getWorld()->playSound(SOUND_PLAYER_DIE);
			setDead();
		}
	}
}
void Player::damage(int damageAmt) { 
	if (tryToBeKilled(damageAmt)) {
		getWorld()->playSound(SOUND_PLAYER_DIE);
		setDead();
	}
	else {
		getWorld()->playSound(SOUND_PLAYER_IMPACT);
	}
}
int Player::shootingSound() const { return SOUND_PLAYER_FIRE; }
int Player::getHealthPct() const { 
	double healthPercent = getHitPoints();
	healthPercent /= 20;
	healthPercent *= 100;
	return (int)healthPercent; 
}
int Player::getAmmo() const { return m_ammo; }
void Player::restoreHealth() { setHitPoints(20); }
void Player::increaseAmmo() { m_ammo += 20; }

void Player::addPeaInFront() {
	int dir = this->getDirection();
	int dx;
	int dy;
	setDxDy(dx, dy);
	double newX = getX() + dx;
	double newY = getY() + dy;
	getWorld()->addActor(new Pea(getWorld(), (int)newX, (int)newY, getDirection()));
}

void Player::setDxDy(int &dx, int &dy){
	int dir = this->getDirection();
	dx = 0;
	dy = 0;
	switch (dir) {
	case up:
		dy++;
		break;
	case down:
		dy--;
		break;
	case left:
		dx--;
		break;
	case right:
		dx++;
		break;
	}
}

/*
----------------------------------------------------------------------------
Robot Declarations
----------------------------------------------------------------------------
*/

Robot::Robot(StudentWorld* world, int startX, int startY, int imageID, int hitPoints, int score, int startDir) : 
	Agent(world, startX, startY, imageID, hitPoints, startDir), m_score(score), m_ticksUntilActive((28 - getWorld()->getLevel()) / 4) , m_currentTick(0)
	{ if (m_ticksUntilActive < 3) m_ticksUntilActive = 3; }

void Robot::damage(int damageAmt) {
	if (tryToBeKilled(damageAmt)) {
		getWorld()->playSound(SOUND_ROBOT_DIE);
		getWorld()->increaseScore(m_score);
		doDifferentiatedDyingStuff();
	}
	else {
		getWorld()->playSound(SOUND_ROBOT_IMPACT);
	}
}
int Robot::shootingSound() const { return SOUND_ENEMY_FIRE; }

bool Robot::isResting() {
	m_currentTick++;
	if (m_currentTick >= m_ticksUntilActive) {
		m_currentTick = 0;
		return false;
	}
	return true;
}

void Robot::addPeaInFront() {
	int dir = this->getDirection();
	int dx, dy;
	setDxDy(dx, dy);
	double newX = getX() + dx;
	double newY = getY() + dy;
	getWorld()->addActor(new Pea(getWorld(), (int)newX, (int)newY, getDirection()));
}

void Robot::setDxDy(int& dx, int& dy) {
	int dir = this->getDirection();
	dx = 0;
	dy = 0;
	switch (dir) {
	case up:
		dy++;
		break;
	case down:
		dy--;
		break;
	case left:
		dx--;
		break;
	case right:
		dx++;
		break;
	}
}


/*
----------------------------------------------------------------------------
RageBot Declarations
----------------------------------------------------------------------------
*/

RageBot::RageBot(StudentWorld* world, int startX, int startY, int startDir) : 
	Robot(world, startX, startY, IID_RAGEBOT, 10, 100, startDir) {}
void RageBot::doSomething() {
	if (!isAlive()) return;
	if (isResting()) return;
	int dx, dy;
	setDxDy(dx, dy);
	if (getWorld()->existsClearShotToPlayer((int)getX(), (int)getY(), dx, dy)) {
		getWorld()->playSound(shootingSound());
		addPeaInFront();
		return;
	}
	if (!moveIfPossible()) {
		int dir = getDirection();
		switch (dir) {
		case up:
			setDirection(down);
			break;
		case down:
			setDirection(up);
			break;
		case left:
			setDirection(right);
			break;
		case right:
			setDirection(left);
			break;
		}
	}
}

/*
----------------------------------------------------------------------------
ThiefBot Declarations
----------------------------------------------------------------------------
*/


ThiefBot::ThiefBot(StudentWorld* world, int startX, int startY, int imageID, int hitPoints, int score) : 
	Robot(world, startX, startY, imageID, hitPoints, score, right), m_stolenGoodie(nullptr), m_distanceBeforeTurning(0) {}

void ThiefBot::doDifferentiatedDyingStuff() {
	int thiefBotX = (int)getX();
	int thiefBotY = (int)getY();
	if (m_stolenGoodie != nullptr) {
		if (getStolenGoodie() != nullptr) {
			getStolenGoodie()->moveTo(thiefBotX, thiefBotY);
		}
		m_stolenGoodie->setVisible(true);
	}
}
Actor* ThiefBot::getStolenGoodie() { return m_stolenGoodie; }
void ThiefBot::setStolenGoodie(Actor* goodie) { m_stolenGoodie = goodie; }
bool ThiefBot::isReachedDistance() {
	m_distanceBeforeTurning--;
	if (m_distanceBeforeTurning < 1) {
		m_distanceBeforeTurning = randInt(1, 6);
		return true;
	}
	return false;
}
void ThiefBot::setRandomDirection() {
	int dir = randInt(0, 3);
	switch (dir) {
	case 0:
		setDirection(up);
		break;
	case 1:
		setDirection(down);
		break;
	case 2:
		setDirection(left);
		break;
	case 3:
		setDirection(right);
		break;
	}
}
/*
----------------------------------------------------------------------------
Regular ThiefBot Declarations
----------------------------------------------------------------------------
*/

RegularThiefBot::RegularThiefBot(StudentWorld* world, int startX, int startY) : 
	ThiefBot(world, startX, startY, IID_THIEFBOT, 5, 10) {}
void RegularThiefBot::doSomething() { 
	if (!isAlive()) return;
	if (isResting()) return;
	
	int thiefBotX = (int)getX();
	int thiefBotY = (int)getY();

	Actor* stealableItemAtRobotLocation = getWorld()->getColocatedStealable(thiefBotX, thiefBotY);
	if (stealableItemAtRobotLocation != nullptr && getStolenGoodie() == nullptr) {
		if (randInt(1, 10) == 1) {
			getWorld()->playSound(SOUND_ROBOT_MUNCH);
			setStolenGoodie(stealableItemAtRobotLocation);
			stealableItemAtRobotLocation->moveTo(0, 0);
			stealableItemAtRobotLocation->setVisible(false);
			return;
		}
	}

	if (isReachedDistance()){
		setRandomDirection();
		for (int i = 0; i < 3; i++) {
			switch (getDirection()) {
				case up:
					setDirection(right);
					break;
				case right:
					setDirection(down);
					break;
				case down:
					setDirection(left);
					break;
				case left:
					setDirection(up);
					break;
			}
			if (moveIfPossible()) break;
		}
	}
	else {
		moveIfPossible();
	}

}

/*
----------------------------------------------------------------------------
Mean ThiefBot Declarations
----------------------------------------------------------------------------
*/

MeanThiefBot::MeanThiefBot(StudentWorld* world, int startX, int startY) :
	ThiefBot(world, startX, startY, IID_MEAN_THIEFBOT, 8, 20) {}
void MeanThiefBot::doSomething() {
	if (!isAlive()) return;
	if (isResting()) return;

	int thiefBotX = (int)getX();
	int thiefBotY = (int)getY();

	Actor* stealableItemAtRobotLocation = getWorld()->getColocatedStealable(thiefBotX, thiefBotY);
	if (stealableItemAtRobotLocation != nullptr && getStolenGoodie() == nullptr) {
		if (randInt(1, 10) == 1) {
			getWorld()->playSound(SOUND_ROBOT_MUNCH);
			setStolenGoodie(stealableItemAtRobotLocation);
			stealableItemAtRobotLocation->moveTo(0, 0);
			stealableItemAtRobotLocation->setVisible(false);
			return;
		}
	}

	int dx, dy;
	setDxDy(dx, dy);
	if (getWorld()->existsClearShotToPlayer(thiefBotX, thiefBotY, dx, dy)) {
		getWorld()->playSound(shootingSound());
		addPeaInFront();
		return;
	}


	if (isReachedDistance()) {
		setRandomDirection();
		for (int i = 0; i < 3; i++) {
			switch (getDirection()) {
			case up:
				setDirection(right);
				break;
			case right:
				setDirection(down);
				break;
			case down:
				setDirection(left);
				break;
			case left:
				setDirection(up);
				break;
			}
			if (moveIfPossible()) break;
		}
	}
	else {
		moveIfPossible();
	}
}

/*
----------------------------------------------------------------------------
Wall Declarations
----------------------------------------------------------------------------
*/

Wall::Wall(StudentWorld* world, int startX, int startY) : 
	Actor(world, startX, startY, IID_WALL, -1, none) {}

/*
----------------------------------------------------------------------------
Marble Declarations
----------------------------------------------------------------------------
*/

Marble::Marble(StudentWorld* world, int startX, int startY) : Actor(world, startX, startY, IID_MARBLE, 10, none) {}
void Marble::damage(int damageAmt) { tryToBeKilled(damageAmt); }
bool Marble::bePushedBy(Agent* a, int x, int y) {
	if (a->canPushMarbles() && getWorld()->canMarbleMoveTo(x, y)) {
		moveTo(x, y);
		return true;
	}
	return false;
}

/*
----------------------------------------------------------------------------
Pit Declarations
----------------------------------------------------------------------------
*/
Pit::Pit(StudentWorld* world, int startX, int startY) : 
	Actor(world, startX, startY, IID_PIT, -1, none) {}
void Pit::doSomething() {
	if (!isAlive()) return;
	getWorld()->swallowSwallowable(this);
}

/*
----------------------------------------------------------------------------
Pea Declarations
----------------------------------------------------------------------------
*/

Pea::Pea(StudentWorld* world, int startX, int startY, int startDir) : Actor(world, startX, startY, IID_PEA, -1, startDir) {}
void Pea::doSomething(){ 
	if (!isAlive()) return;
	if (getWorld()->damageSomething(this, 2)) return;
	movePeaForward();
	if (getWorld()->damageSomething(this, 2)) return;
}

void Pea::movePeaForward() {
	int dir = getDirection();
	switch (dir) {
		case up:
			moveTo(getX(), getY() + 1);
			break;
		case down:
			moveTo(getX(), getY() - 1);
			break;
		case left:
			moveTo(getX() - 1, getY());
			break;
		case right:
			moveTo(getX() + 1, getY());
			break;
	}
}

/*
----------------------------------------------------------------------------
ThiefBotFactory Declarations
----------------------------------------------------------------------------
*/


ThiefBotFactory::ThiefBotFactory(StudentWorld* world, int startX, int startY, ThiefBotFactory::ProductType type) : 
	Actor(world, startX, startY, IID_ROBOT_FACTORY, -1, none), m_productionType(type) {}

void ThiefBotFactory::doSomething() { 
	int censusDistance = 3;
	int robotsInCensus = 0;
	int factoryX = (int)getX();
	int factoryY = (int)getY();
	if (getWorld()->doFactoryCensus(factoryX, factoryY, censusDistance, robotsInCensus)) {
		if (robotsInCensus < 3 && randInt(1, 50) == 1) {
			getWorld()->playSound(SOUND_ROBOT_BORN);
			if (m_productionType == ThiefBotFactory::MEAN) {
				getWorld()->addActor(new MeanThiefBot(getWorld(), factoryX, factoryY));
			} else if (m_productionType == ThiefBotFactory::REGULAR){
				getWorld()->addActor(new RegularThiefBot(getWorld(), factoryX, factoryY));
			}
		}
	}

}

/*
----------------------------------------------------------------------------
Exit Item Declarations
----------------------------------------------------------------------------
*/

Exit::Exit(StudentWorld* world, int startX, int startY) :
	Actor(world, startX, startY, IID_EXIT, -1, none), m_revealed(false)
	{ setVisible(false); }
void Exit::doSomething() {
	if (!getWorld()->anyCrystals() && !isRevealed()) {
		reveal();
		getWorld()->playSound(SOUND_REVEAL_EXIT);
		setVisible(true);
	}
	if (isRevealed() && getWorld()->isPlayerColocatedWith(this)) {
		getWorld()->setLevelFinished();
	}
}

bool Exit::isRevealed() { return m_revealed; }
void Exit::reveal() { m_revealed = true; }
/*
----------------------------------------------------------------------------
Pickupable Item Declarations
----------------------------------------------------------------------------
*/

PickupableItem::PickupableItem(StudentWorld* world, int startX, int startY, int imageID, int score) :
	Actor(world, startX, startY, imageID, -1, none), m_score(score) {}
void PickupableItem::doSomething() {
	if (!isAlive()) return;
	if (getWorld()->isPlayerColocatedWith(this)) {
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(m_score);
		doDifferentiatedStuff();
		setDead();
	}
}

/*
----------------------------------------------------------------------------
Crystal Declarations
----------------------------------------------------------------------------
*/

Crystal::Crystal(StudentWorld* world, int startX, int startY) : 
	PickupableItem(world, startX, startY, IID_CRYSTAL, 50) {}
void Crystal::doDifferentiatedStuff() {
	getWorld()->decCrystals();
}

/*
----------------------------------------------------------------------------
Goodie Declarations
----------------------------------------------------------------------------
*/

Goodie::Goodie(StudentWorld* world, int startX, int startY, int imageID, int score) : 
	PickupableItem(world, startX, startY, imageID, score) {}
/*
----------------------------------------------------------------------------
ExtraLifeGoodie Declarations
----------------------------------------------------------------------------
*/

ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* world, int startX, int startY) : Goodie(world, startX, startY, IID_EXTRA_LIFE, 1000) {}
void ExtraLifeGoodie::doDifferentiatedStuff() {
	getWorld()->incLives();
}

/*
----------------------------------------------------------------------------
RestoreHealthGoodie Declarations
----------------------------------------------------------------------------
*/

RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* world, int startX, int startY) : Goodie(world, startX, startY, IID_RESTORE_HEALTH, 500) {}
void RestoreHealthGoodie::doDifferentiatedStuff() {
	getWorld()->restorePlayerHealth();
}

/*
----------------------------------------------------------------------------
AmmoGoodie Declarations
----------------------------------------------------------------------------
*/
AmmoGoodie::AmmoGoodie(StudentWorld* world, int startX, int startY) : Goodie(world, startX, startY, IID_AMMO, 100) {}
void AmmoGoodie::doDifferentiatedStuff() {
	getWorld()->increaseAmmo();
}