#include "Defence_Manager.h"
#include "UnitUtil.h"

using namespace UAlbertaBot;


const size_t IdlePriority = 0;
const size_t AttackPriority = 1;
const size_t BaseDefensePriority = 2;
const size_t ScoutDefensePriority = 3;
const size_t DropPriority = 4;


Defence_Manager::Defence_Manager()
: _workerDefence(nullptr)
, _numWorkerDefence(0)
, _DefenceUnderAttack(false)
, _DefenceStatus("None")
{
}


//Defence_Manager & Defence_Manager::Instance()
//{
//	static Defence_Manager instance;
//	return instance;
//}


void Defence_Manager::update(const BWAPI::Unitset & DefenceUnits)
{
	if (!Config::Modules::UsingDefenceCommander)
	{
		return;
	}

	if (!_initialized)
	{
		return;
	}


	std::string Type;
	std::string Type_provider;
	GridCell Cell_D;
	BWAPI::Position Dragoon_position;
	BWAPI::Position Building_position;
	int a=0;
	int ID;

	for (auto & unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus)	//Dragoon->protoss race 
		{
			Cell_D = MapGrid::Instance().getCell(unit);
			Building_position = Cell_D.center;
			BWAPI::Broodwar->drawTextScreen(200, 240, "\x04  %s   place is %d  %d  ", unit->getType().c_str(), Building_position.x, Building_position.y);
		}
		if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)	//Dragoon->protoss race 
		{
			Cell_D = MapGrid::Instance().getCell(unit);
			Dragoon_position = Cell_D.center;
			/*BWAPI::Broodwar->drawTextScreen(300, 240 - a * 20, "\x04  %s id is %d  place is %d  %d  ", unit->getType().c_str(), ID, Dragoon_position.x, Dragoon_position.y);*/
			a++;
		}
	}
}



void Defence_Manager::setWorkerDefence(BWAPI::Unitset & Defence_unit_all)
{
	BWAPI::Unit unit;
	// if we have a previous worker scout, release it back to the worker manager
	if (Defence_unit_all.size() == 5)
	{

	}

	for (auto & unit : Defence_unit_all)
	{
		int id;
		_workerDefence = unit;
		id = _workerDefence->getID();
		WorkerManager::Instance().setDefenceWorker(_workerDefence);

	}
}

void Defence_Manager::MoveDefence()
{
	if (!_workerDefence || !_workerDefence->exists() || !(_workerDefence->getHitPoints() > 0))
	{
		return;
	}

	int scoutHP = _workerDefence->getHitPoints() + _workerDefence->getShields();

}


void Defence_Manager::initializeSquads()
{
	SquadOrder idleOrder(SquadOrderTypes::Idle, BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), 100, "Chill Out");
	_squadData.addSquad("Idle", Squad("Idle", idleOrder, IdlePriority));

	// the main attack squad that will pressure the enemy's closest base location
	SquadOrder mainAttackOrder(SquadOrderTypes::Attack, getMainDefenceLocation(), 800, "Attack Enemy Base");
	_squadData.addSquad("MainAttack", Squad("MainAttack", mainAttackOrder, AttackPriority));

	BWAPI::Position ourBasePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

	// the scout defense squad will handle chasing the enemy worker scout
	SquadOrder enemyScoutDefense(SquadOrderTypes::Defend, ourBasePosition, 900, "Get the scout");
	_squadData.addSquad("ScoutDefense", Squad("ScoutDefense", enemyScoutDefense, ScoutDefensePriority));

	// add a drop squad if we are using a drop strategy
	if (Config::Strategy::StrategyName == "Protoss_Drop")
	{
		SquadOrder zealotDrop(SquadOrderTypes::Drop, ourBasePosition, 900, "Wait for transport");
		_squadData.addSquad("Drop", Squad("Drop", zealotDrop, DropPriority));
	}

	_initialized = true;
}


void Defence_Manager::drawSquadInformation(int x, int y)
{

}


BWAPI::Position Defence_Manager::getMainDefenceLocation()
{
	BWTA::BaseLocation * enemyBaseLocation = InformationManager::Instance().getMainBaseLocation(BWAPI::Broodwar->enemy());
	BWAPI::Player player_1;
	int num;
	UAlbertaBot::UnitData Player_Data;
	Player_Data = InformationManager::Instance().getUnitData(BWAPI::Broodwar->enemy());
	
	// First choice: Attack an enemy region if we can see units inside it
	if (enemyBaseLocation)
	{
		BWAPI::Position enemyBasePosition = enemyBaseLocation->getPosition();

		// get all known enemy units in the area
		BWAPI::Unitset enemyUnitsInArea;
		MapGrid::Instance().GetUnits(enemyUnitsInArea, enemyBasePosition, 800, false, true);
		bool onlyOverlords = true;
		for (auto & unit : enemyUnitsInArea)
		{
			if (unit->getType() != BWAPI::UnitTypes::Zerg_Overlord)
			{
				onlyOverlords = false;
			}
		}

		if (!BWAPI::Broodwar->isExplored(BWAPI::TilePosition(enemyBasePosition)) || !enemyUnitsInArea.empty())
		{
			if (!onlyOverlords)
			{
				return enemyBaseLocation->getPosition();
			}
		}
	}

	// Second choice: Attack known enemy buildings
	for (const auto & kv : InformationManager::Instance().getUnitInfo(BWAPI::Broodwar->enemy()))
	{
		const UnitInfo & ui = kv.second;

		if (ui.type.isBuilding() && ui.lastPosition != BWAPI::Positions::None)
		{
			return ui.lastPosition;
		}
	}

	// Third choice: Attack visible enemy units that aren't overlords
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
		{
			continue;
		}

		if (UnitUtil::IsValidUnit(unit) && unit->isVisible())
		{
			return unit->getPosition();
		}
	}

	// Fourth choice: We can't see anything so explore the map attacking along the way
	return MapGrid::Instance().getLeastExplored();
}
