#pragma once

#include "CoreMinimal.h"
#include "Grid/GridCoord.h"
#include "Templates/Function.h"

class FACTORYEXPERIMENT_API FGridPathfinder
{
public:
	static bool FindPath(
		const FGridCoord& StartCoord,
		const FGridCoord& GoalCoord,
		TFunctionRef<bool(const FGridCoord&)> IsPassable,
		TArray<FGridCoord>& OutPath
	);
};