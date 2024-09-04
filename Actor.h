#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Agent;

class Actor : public GraphObject
{
public:
    Actor(StudentWorld* world, int startX, int startY, int imageID, int hitPoints, int startDir);

    // Action to perform each tick
    virtual void doSomething() = 0;

    // Is this actor alive?
    bool isAlive() const;

    // Mark this actor as dead
    void setDead();

    // Reduce this actor's hit points
    void decHitPoints(int amt);

    // Get this actor's world
    StudentWorld* getWorld() const;

    // Can an agent occupy the same square as this actor?
    virtual bool allowsAgentColocation() const { return false; }

    // Can a marble occupy the same square as this actor?
    virtual bool allowsMarble() const { return false; }

    // Does this actor count when a factory counts items near it?
    virtual bool countsInFactoryCensus() const { return false; }

    // Does this actor stop peas from continuing?
    virtual bool stopsPea() const { return false; }

    // Can this actor be damaged by peas?
    virtual bool isDamageable() const { return false; }

    // Cause this Actor to sustain damageAmt hit points of damage.
    virtual void damage(int damageAmt);

    // Can this actor be pushed by a to location x,y?
    virtual bool bePushedBy(Agent* a, int x, int y) { return false; }

    // Can this actor be swallowed by a pit?
    virtual bool isSwallowable() const { return false; }

    // Can this actor be picked up by a ThiefBot?
    virtual bool isStealable() const { return false; }

    // How many hit points does this actor have left?
    virtual int getHitPoints() const;

    // Set this actor's hit points to amt.
    virtual void setHitPoints(int amt);

    // Make the actor sustain damage.  Return true if this kills the
    // actor, and false otherwise.
    virtual bool tryToBeKilled(int damageAmt);

private:
    int m_hitpoints;
    bool m_alive;
    StudentWorld* m_world;
};

class Agent : public Actor
{
public:
    Agent(StudentWorld* world, int startX, int startY, int imageID,
        int hitPoints, int startDir);

    // Move to the adjacent square in the direction the agent is facing
    // if it is not blocked, and return true.  Return false if the agent
    // can't move.
    bool moveIfPossible();

    // Return true if this agent can push marbles (which means it's the
    // player).
    virtual bool canPushMarbles() const { return false; }

    // Return true if this agent doesn't shoot unless there's an unobstructed
    // path to the player.
    virtual bool needsClearShot() const { return false; };

    // Return the sound effect ID for a shot from this agent.
    virtual int shootingSound() const = 0;
private:
    bool handleMovement(int dx, int dy);
};

class Player : public Agent
{
public:
    Player(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
    virtual bool isDamageable() const { return true; }
    virtual void damage(int damageAmt);
    virtual bool canPushMarbles() const { return true; }
    //virtual bool needsClearShot() const; IS FALSE
    virtual int shootingSound() const;

    // Get player's health percentage
    int getHealthPct() const;

    // Get player's amount of ammunition
    int getAmmo() const;

    // Restore player's health to the full amount.
    void restoreHealth();

    // Increase player's amount of ammunition.
    void increaseAmmo();
private:
    int m_ammo;
    void addPeaInFront();
    void setDxDy(int& dx, int& dy);
};

class Robot : public Agent
{
public:
    Robot(StudentWorld* world, int startX, int startY, int imageID,
        int hitPoints, int score, int startDir);
    virtual void doSomething() const { return; }
    virtual bool isDamageable() const { return true; }
    virtual void damage(int damageAmt);
    virtual bool canPushMarbles() const { return false; }
    virtual bool needsClearShot() const { return true; }
    virtual int shootingSound() const;

    // Does this robot shoot?
    virtual bool isShootingRobot() const { return true; }
    bool isResting();

    void addPeaInFront();
    void setDxDy(int& dx, int& dy);
private:
    int m_score;
    int m_ticksUntilActive;
    int m_currentTick;
    virtual void doDifferentiatedDyingStuff() = 0;
};

class RageBot : public Robot
{
public:
    RageBot(StudentWorld* world, int startX, int startY, int startDir);
    virtual void doSomething();
private:
    virtual void doDifferentiatedDyingStuff() { return; }
};

class ThiefBot : public Robot
{
public:
    ThiefBot(StudentWorld* world, int startX, int startY, int imageID,
        int hitPoints, int score);
    virtual void doSomething() { return; }
    virtual bool countsInFactoryCensus() const { return true; }
    Actor* getStolenGoodie();
    void setStolenGoodie(Actor* goodie);
    bool isReachedDistance();
    void setRandomDirection();
private:
    int m_distanceBeforeTurning;
    Actor* m_stolenGoodie;
    virtual void doDifferentiatedDyingStuff();
};

class RegularThiefBot : public ThiefBot
{
public:
    RegularThiefBot(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
    virtual bool isShootingRobot() const { return false; }
};

class MeanThiefBot : public ThiefBot
{
public:
    MeanThiefBot(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
};

class Exit : public Actor
{
public:
    Exit(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
    virtual bool allowsAgentColocation() const { return true; }
private:
    bool m_revealed;
    bool isRevealed();
    void reveal();
};

class Wall : public Actor
{
public:
    Wall(StudentWorld* world, int startX, int startY);
    virtual void doSomething() { return; }
    virtual bool stopsPea() const { return true; }
};

class Marble : public Actor
{
public:
    Marble(StudentWorld* world, int startX, int startY);
    virtual void doSomething() { return; }
    virtual bool isDamageable() const { return true; }
    virtual void damage(int damageAmt);
    virtual bool isSwallowable() const { return true; } 
    virtual bool bePushedBy(Agent* a, int x, int y);
};

class Pit : public Actor
{
public:
    Pit(StudentWorld* world, int startX, int startY);
    virtual void doSomething();
    virtual bool allowsMarble() const { return true; }
};

class Pea : public Actor
{
public:
    Pea(StudentWorld* world, int startX, int startY, int startDir);
    virtual void doSomething();
    virtual bool allowsAgentColocation() const { return true; }
private:
    void movePeaForward();
};

class ThiefBotFactory : public Actor
{
public:
    enum ProductType { REGULAR, MEAN };

    ThiefBotFactory(StudentWorld* world, int startX, int startY, ProductType type);
    virtual void doSomething();
    virtual bool stopsPea() const { return true; }
private:
    ProductType m_productionType;
};

class PickupableItem : public Actor
{
public:
    PickupableItem(StudentWorld* world, int startX, int startY, int imageID, int score);
    virtual void doSomething();
    virtual bool allowsAgentColocation() const { return true; }
private:
    int m_score;
    virtual void doDifferentiatedStuff() = 0;
};

class Crystal : public PickupableItem
{
public:
    Crystal(StudentWorld* world, int startX, int startY);
private:
    virtual void doDifferentiatedStuff();
};

class Goodie : public PickupableItem
{
public:
    Goodie(StudentWorld* world, int startX, int startY, int imageID,
        int score);
    virtual bool isStealable() const { return true; }
private:
    virtual void doDifferentiatedStuff() = 0;
};

class ExtraLifeGoodie : public Goodie
{
public:
    ExtraLifeGoodie(StudentWorld* world, int startX, int startY);
private:
    virtual void doDifferentiatedStuff();
};

class RestoreHealthGoodie : public Goodie
{
public:
    RestoreHealthGoodie(StudentWorld* world, int startX, int startY);
private:
    virtual void doDifferentiatedStuff();
};

class AmmoGoodie : public Goodie
{
public:
    AmmoGoodie(StudentWorld* world, int startX, int startY);
private:
    virtual void doDifferentiatedStuff();
};

#endif // ACTOR_H_