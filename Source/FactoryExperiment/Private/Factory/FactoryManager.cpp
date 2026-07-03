#include "Factory/FactoryManager.h"

#include "Engine/World.h"
#include "Factory/Buildings/FactoryBuilding.h"
#include "Factory/Data/FactoryBuildingDataAsset.h"

AFactoryManager::AFactoryManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFactoryManager::BeginPlay()
{
	Super::BeginPlay();

	if (SimulationStepInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			SimulationTimerHandle,
			this,
			&AFactoryManager::SimulationStep,
			SimulationStepInterval,
			true
		);
	}
}

bool AFactoryManager::TryPlaceBuilding(
	UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
)
{
	if (!CanPlaceBuilding(BuildingData, OriginCoord, Direction))
	{
		return false;
	}

	if (!BuildingData || !BuildingData->BuildingActorClass)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	const FVector SpawnLocation = FVector(
		OriginCoord.X * 100.0f,
		OriginCoord.Y * 100.0f,
		0.0f
	);

	AFactoryBuilding* BuildingActor = World->SpawnActor<AFactoryBuilding>(
		BuildingData->BuildingActorClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!BuildingActor)
	{
		return false;
	}

	const int32 BuildingId = RegisterBuildingActor(BuildingActor);

	BuildingActor->BuildingDefinition = BuildingData;
	BuildingActor->InitializeBuilding(BuildingId, OriginCoord);

	for (int32 LocalY = 0; LocalY < BuildingData->FootprintSize.Y; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < BuildingData->FootprintSize.X; ++LocalX)
		{
			const FGridCoord CellCoord(
				OriginCoord.X + LocalX,
				OriginCoord.Y + LocalY
			);

			FFactoryGridCell* Cell = GetOrCreateCell(CellCoord);
			if (!Cell)
			{
				continue;
			}

			Cell->Occupancy = BuildingData->bIsConveyor
				? EFactoryCellOccupancy::Conveyor
				: EFactoryCellOccupancy::Building;
			Cell->Direction = Direction;
			Cell->BuildingId = BuildingId;
		}
	}

	if (!BuildingData->bIsConveyor)
	{
		FFactoryMachineRuntimeData RuntimeData;
		RuntimeData.BuildingId = BuildingId;
		RuntimeData.RecipeId = BuildingData->DefaultRecipeId;
		MachineRuntimeData.Add(RuntimeData);
	}

	return true;
}

bool AFactoryManager::CanPlaceBuilding(
	const UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection /*Direction*/
) const
{
	if (!BuildingData || BuildingData->FootprintSize.X <= 0 || BuildingData->FootprintSize.Y <= 0)
	{
		return false;
	}

	for (int32 LocalY = 0; LocalY < BuildingData->FootprintSize.Y; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < BuildingData->FootprintSize.X; ++LocalX)
		{
			const FGridCoord CellCoord(
				OriginCoord.X + LocalX,
				OriginCoord.Y + LocalY
			);

			if (IsCellOccupied(CellCoord))
			{
				return false;
			}
		}
	}

	return true;
}

bool AFactoryManager::IsCellOccupied(const FGridCoord& Coord) const
{
	const FFactoryGridCell* Cell = GetCell(Coord);

	if (!Cell)
	{
		return false;
	}

	return Cell->IsOccupied();
}

bool AFactoryManager::IsCellValidForPath(const FGridCoord& Coord) const
{
	const FFactoryGridCell* Cell = GetCell(Coord);

	if (!Cell)
	{
		return true;
	}

	return Cell->Occupancy == EFactoryCellOccupancy::Empty
		|| Cell->Occupancy == EFactoryCellOccupancy::Conveyor;
}

FFactoryGridCell* AFactoryManager::GetOrCreateCell(const FGridCoord& Coord)
{
	const FGridCoord ChunkCoord = WorldCoordToChunkCoord(Coord);
	const FGridCoord LocalCoord = WorldCoordToLocalCoord(Coord);

	FFactoryChunk& Chunk = Chunks.FindOrAdd(ChunkCoord);
	return Chunk.GetCell(LocalCoord.X, LocalCoord.Y);
}

const FFactoryGridCell* AFactoryManager::GetCell(const FGridCoord& Coord) const
{
	const FGridCoord ChunkCoord = WorldCoordToChunkCoord(Coord);
	const FGridCoord LocalCoord = WorldCoordToLocalCoord(Coord);

	const FFactoryChunk* Chunk = Chunks.Find(ChunkCoord);
	if (!Chunk)
	{
		return nullptr;
	}

	return Chunk->GetCell(LocalCoord.X, LocalCoord.Y);
}

TArray<FGridCoord> AFactoryManager::GetNeighbors(const FGridCoord& Coord) const
{
	return TArray<FGridCoord>
	{
		FGridCoord(Coord.X, Coord.Y + 1),
		FGridCoord(Coord.X + 1, Coord.Y),
		FGridCoord(Coord.X, Coord.Y - 1),
		FGridCoord(Coord.X - 1, Coord.Y)
	};
}

void AFactoryManager::SimulationStep()
{
	UpdateConveyors(SimulationStepInterval);
	UpdateMachines(SimulationStepInterval);
}

void AFactoryManager::UpdateMachines(float /*DeltaTime*/)
{
}

void AFactoryManager::UpdateConveyors(float /*DeltaTime*/)
{
}

FGridCoord AFactoryManager::WorldCoordToChunkCoord(const FGridCoord& WorldCoord) const
{
	const int32 Size = FFactoryChunk::ChunkSize;

	const int32 ChunkX = FMath::FloorToInt(static_cast<float>(WorldCoord.X) / Size);
	const int32 ChunkY = FMath::FloorToInt(static_cast<float>(WorldCoord.Y) / Size);

	return FGridCoord(ChunkX, ChunkY);
}

FGridCoord AFactoryManager::WorldCoordToLocalCoord(const FGridCoord& WorldCoord) const
{
	const int32 Size = FFactoryChunk::ChunkSize;

	int32 LocalX = WorldCoord.X % Size;
	int32 LocalY = WorldCoord.Y % Size;

	if (LocalX < 0)
	{
		LocalX += Size;
	}

	if (LocalY < 0)
	{
		LocalY += Size;
	}

	return FGridCoord(LocalX, LocalY);
}

int32 AFactoryManager::RegisterBuildingActor(AFactoryBuilding* Building)
{
	if (!Building)
	{
		return INDEX_NONE;
	}

	return BuildingActors.Add(Building);
}