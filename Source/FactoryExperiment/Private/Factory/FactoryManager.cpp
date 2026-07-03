#include "Factory/FactoryManager.h"

#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Factory/Buildings/FactoryBuilding.h"
#include "Factory/Data/FactoryBuildingDataAsset.h"
#include "GameFramework/PlayerController.h"

namespace
{
	constexpr float DebugGridZOffset = 5.0f;
	constexpr float DebugCellLineThickness = 0.0f;
	constexpr float DebugChunkLineThickness = 3.0f;
	constexpr float DebugHoveredCellLineThickness = 5.0f;

	const FColor DebugCellColor(192, 192, 192);
	const FColor DebugChunkColor(160, 32, 240);
	const FColor DebugHoveredCellColor(0, 255, 0);
}

AFactoryManager::AFactoryManager()
{
	PrimaryActorTick.bCanEverTick = true;

	TransformComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TransformComponent"));
	RootComponent = TransformComponent;
}

void AFactoryManager::BeginPlay()
{
	Super::BeginPlay();

	CreateInitialChunks();

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

void AFactoryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bUpdateHoveredCellFromMouseRaycast)
	{
		UpdateHoveredCellFromMouseRaycast();
	}

	if (bDrawDebugCells || bDrawDebugChunks || bDrawDebugHoveredCell)
	{
		DrawDebugGrid();
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

	const FVector SpawnLocation = GridCellCenterToWorld(OriginCoord);

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

void AFactoryManager::SetDebugHoveredCell(const FGridCoord& Coord)
{
	DebugHoveredCellCoord = Coord;
	bHasDebugHoveredCell = true;
}

void AFactoryManager::ClearDebugHoveredCell()
{
	bHasDebugHoveredCell = false;
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

void AFactoryManager::CreateInitialChunks()
{
	Chunks.FindOrAdd(FGridCoord(0, 0));
}

void AFactoryManager::UpdateHoveredCellFromMouseRaycast()
{
	FGridCoord NewHoveredCellCoord;
	if (!TryGetMouseRaycastGridCoord(NewHoveredCellCoord) || !GetCell(NewHoveredCellCoord))
	{
		if (bHasPreviousHoveredCellCoord)
		{
			ClearDebugHoveredCell();
			bHasPreviousHoveredCellCoord = false;
		}

		return;
	}

	if (bHasPreviousHoveredCellCoord && PreviousHoveredCellCoord == NewHoveredCellCoord)
	{
		return;
	}

	PreviousHoveredCellCoord = NewHoveredCellCoord;
	bHasPreviousHoveredCellCoord = true;
	SetDebugHoveredCell(NewHoveredCellCoord);
}

bool AFactoryManager::TryGetMouseRaycastGridCoord(FGridCoord& OutCoord) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return false;
	}

	FVector RayOrigin;
	FVector RayDirection;
	if (!PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection) || FMath::IsNearlyZero(RayDirection.Z))
	{
		return false;
	}

	const float PlaneZ = GetActorLocation().Z;
	const float DistanceToPlane = (PlaneZ - RayOrigin.Z) / RayDirection.Z;
	if (DistanceToPlane < 0.0f)
	{
		return false;
	}

	const FVector HitLocation = RayOrigin + RayDirection * DistanceToPlane;
	OutCoord = WorldLocationToGridCoord(HitLocation);
	return true;
}

void AFactoryManager::DrawDebugGrid() const
{
	if (!GetWorld())
	{
		return;
	}

	for (const TPair<FGridCoord, FFactoryChunk>& ChunkPair : Chunks)
	{
		const FGridCoord& ChunkCoord = ChunkPair.Key;

		if (bDrawDebugCells)
		{
			DrawDebugCellGridForChunk(ChunkCoord);
		}

		if (bDrawDebugChunks)
		{
			DrawDebugChunkBounds(ChunkCoord);
		}
	}

	if (bDrawDebugHoveredCell && bHasDebugHoveredCell && GetCell(DebugHoveredCellCoord))
	{
		DrawDebugCellBounds(DebugHoveredCellCoord, DebugHoveredCellColor, DebugHoveredCellLineThickness);
	}
}

void AFactoryManager::DrawDebugCellGridForChunk(const FGridCoord& ChunkCoord) const
{
	const int32 ChunkMinX = ChunkCoord.X * FFactoryChunk::ChunkSize;
	const int32 ChunkMinY = ChunkCoord.Y * FFactoryChunk::ChunkSize;
	const int32 ChunkMaxX = ChunkMinX + FFactoryChunk::ChunkSize;
	const int32 ChunkMaxY = ChunkMinY + FFactoryChunk::ChunkSize;

	for (int32 LineOffset = 0; LineOffset <= FFactoryChunk::ChunkSize; ++LineOffset)
	{
		const float GridX = ChunkMinX + LineOffset - 0.5f;
		const float GridY = ChunkMinY + LineOffset - 0.5f;

		DrawDebugLine(
			GetWorld(),
			GridBoundaryToWorld(GridX, ChunkMinY - 0.5f),
			GridBoundaryToWorld(GridX, ChunkMaxY - 0.5f),
			DebugCellColor,
			false,
			0.0f,
			0,
			DebugCellLineThickness
		);

		DrawDebugLine(
			GetWorld(),
			GridBoundaryToWorld(ChunkMinX - 0.5f, GridY),
			GridBoundaryToWorld(ChunkMaxX - 0.5f, GridY),
			DebugCellColor,
			false,
			0.0f,
			0,
			DebugCellLineThickness
		);
	}
}

void AFactoryManager::DrawDebugChunkBounds(const FGridCoord& ChunkCoord) const
{
	const int32 ChunkMinX = ChunkCoord.X * FFactoryChunk::ChunkSize;
	const int32 ChunkMinY = ChunkCoord.Y * FFactoryChunk::ChunkSize;
	const int32 ChunkMaxX = ChunkMinX + FFactoryChunk::ChunkSize - 1;
	const int32 ChunkMaxY = ChunkMinY + FFactoryChunk::ChunkSize - 1;

	DrawDebugCellBounds(
		FGridCoord(ChunkMinX, ChunkMinY),
		FGridCoord(ChunkMaxX, ChunkMaxY),
		DebugChunkColor,
		DebugChunkLineThickness
	);
}

void AFactoryManager::DrawDebugCellBounds(const FGridCoord& Coord, const FColor& Color, float Thickness) const
{
	DrawDebugCellBounds(Coord, Coord, Color, Thickness);
}

void AFactoryManager::DrawDebugCellBounds(
	const FGridCoord& MinCoord,
	const FGridCoord& MaxCoord,
	const FColor& Color,
	float Thickness
) const
{
	const FVector BottomLeft = GridBoundaryToWorld(MinCoord.X - 0.5f, MinCoord.Y - 0.5f);
	const FVector BottomRight = GridBoundaryToWorld(MaxCoord.X + 0.5f, MinCoord.Y - 0.5f);
	const FVector TopRight = GridBoundaryToWorld(MaxCoord.X + 0.5f, MaxCoord.Y + 0.5f);
	const FVector TopLeft = GridBoundaryToWorld(MinCoord.X - 0.5f, MaxCoord.Y + 0.5f);

	DrawDebugLine(GetWorld(), BottomLeft, BottomRight, Color, false, 0.0f, 0, Thickness);
	DrawDebugLine(GetWorld(), BottomRight, TopRight, Color, false, 0.0f, 0, Thickness);
	DrawDebugLine(GetWorld(), TopRight, TopLeft, Color, false, 0.0f, 0, Thickness);
	DrawDebugLine(GetWorld(), TopLeft, BottomLeft, Color, false, 0.0f, 0, Thickness);
}

FGridCoord AFactoryManager::WorldLocationToGridCoord(const FVector& WorldLocation) const
{
	const FVector RelativeLocation = WorldLocation - GetActorLocation();
	return FGridCoord(
		FMath::FloorToInt(RelativeLocation.X / CellSize + 0.5f),
		FMath::FloorToInt(RelativeLocation.Y / CellSize + 0.5f)
	);
}

FVector AFactoryManager::GridCellCenterToWorld(const FGridCoord& Coord) const
{
	return GetActorLocation() + FVector(Coord.X * CellSize, Coord.Y * CellSize, 0.0f);
}

FVector AFactoryManager::GridBoundaryToWorld(float BoundaryX, float BoundaryY) const
{
	return GetActorLocation() + FVector(BoundaryX * CellSize, BoundaryY * CellSize, DebugGridZOffset);
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