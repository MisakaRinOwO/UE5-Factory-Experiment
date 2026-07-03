#include "Grid/GridPathfinder.h"

#include "Algo/Reverse.h"
#include "Containers/Queue.h"

bool FGridPathfinder::FindPath(
	const FGridCoord& StartCoord,
	const FGridCoord& GoalCoord,
	TFunctionRef<bool(const FGridCoord&)> IsPassable,
	TArray<FGridCoord>& OutPath
)
{
	OutPath.Reset();

	if (StartCoord == GoalCoord)
	{
		OutPath.Add(StartCoord);
		return true;
	}

	TQueue<FGridCoord> OpenSet;
	TSet<FGridCoord> Visited;
	TMap<FGridCoord, FGridCoord> CameFrom;

	OpenSet.Enqueue(StartCoord);
	Visited.Add(StartCoord);

	while (!OpenSet.IsEmpty())
	{
		FGridCoord CurrentCoord;
		OpenSet.Dequeue(CurrentCoord);

		const FGridCoord Neighbors[] =
		{
			FGridCoord(CurrentCoord.X, CurrentCoord.Y + 1),
			FGridCoord(CurrentCoord.X + 1, CurrentCoord.Y),
			FGridCoord(CurrentCoord.X, CurrentCoord.Y - 1),
			FGridCoord(CurrentCoord.X - 1, CurrentCoord.Y)
		};

		for (const FGridCoord& NeighborCoord : Neighbors)
		{
			if (Visited.Contains(NeighborCoord) || !IsPassable(NeighborCoord))
			{
				continue;
			}

			Visited.Add(NeighborCoord);
			CameFrom.Add(NeighborCoord, CurrentCoord);

			if (NeighborCoord == GoalCoord)
			{
				FGridCoord PathCoord = GoalCoord;
				OutPath.Add(PathCoord);

				while (PathCoord != StartCoord)
				{
					PathCoord = CameFrom[PathCoord];
					OutPath.Add(PathCoord);
				}

				Algo::Reverse(OutPath);
				return true;
			}

			OpenSet.Enqueue(NeighborCoord);
		}
	}

	return false;
}