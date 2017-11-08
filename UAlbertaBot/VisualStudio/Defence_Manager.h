#pragma once

#include "Common.h"
#include "Squad.h"
#include "SquadData.h"
#include "InformationManager.h"
#include "StrategyManager.h"

namespace UAlbertaBot
{
	class Defence_Manager
	{
		std::string     _scoutStatus;
		std::string     _gasStealStatus;
		SquadData       _squadData;
		BWAPI::Unitset  _DefenceUnits;
		BWAPI::Unit		_workerDefence;
		int				_numWorkerDefence;
		bool			_DefenceUnderAttack;
		bool            _initialized;
		bool			_DefenceStatus;

	public:

		Defence_Manager();

		/*static Defence_Manager & Instance();*/

		void update(const BWAPI::Unitset & DefenceUnits);

		void drawSquadInformation(int x, int y);

		void initializeSquads();

		void setWorkerDefence(BWAPI::Unitset & Defence_unit_all);

		void MoveDefence();

		BWAPI::Position getMainDefenceLocation(); 

	};
}