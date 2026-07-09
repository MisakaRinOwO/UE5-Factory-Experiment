#include "Factory/FactoryManager.h"

#include "Components/SceneComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Factory/Buildings/FactoryBuilding.h"
#include "Factory/Data/FactoryBuildingDataAsset.h"
#include "Factory/UI/FactoryDeveloperModeWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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

// MARK: Placement control

bool AFactoryManager::TryPlaceBuilding(
	UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
)
{
	if (BuildingData && BuildingData->bIsConveyor)
	{
		return TryPlaceConveyor(BuildingData, OriginCoord, Direction);
	}

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
	const TArray<FGridCoord> OccupiedCoords = BuildFootprintWorldCoords(BuildingData, OriginCoord, Direction);
	const TArray<FFactoryPlacedBuildingPort> WorldPorts = BuildWorldPorts(BuildingData, OriginCoord, Direction);

	BuildingActor->BuildingDefinition = BuildingData;
	BuildingActor->InitializeBuilding(BuildingId, OriginCoord);
	BuildingActor->SetWorldPorts(WorldPorts);

	FFactoryPlacedBuildingInstance BuildingInstance;
	BuildingInstance.BuildingActor = BuildingActor;
	BuildingInstance.BuildingData = BuildingData;
	BuildingInstance.OriginCoord = OriginCoord;
	BuildingInstance.Direction = Direction;
	BuildingInstance.OccupiedCoords = OccupiedCoords;
	BuildingInstance.WorldPorts = WorldPorts;

	for (const FGridCoord& CellCoord : OccupiedCoords)
	{
		FFactoryGridCell* Cell = GetOrCreateCell(CellCoord);
		if (!Cell)
		{
			continue;
		}

		Cell->Occupancy = EFactoryCellOccupancy::Building;
		Cell->Direction = Direction;
		Cell->BuildingId = BuildingId;
		BuildingInstancesByCellCoord.Add(CellCoord, BuildingInstance);
	}

	if (!BuildingData->bIsConveyor)
	{
		FFactoryMachineRuntimeData RuntimeData;
		RuntimeData.BuildingId = BuildingId;
		RuntimeData.RecipeId = BuildingData->DefaultRecipeId;
		MachineRuntimeData.Add(RuntimeData);
	}

	UpdateDeveloperModeCoordDisplay(OriginCoord);

	return true;
}

TArray<FGridCoord> AFactoryManager::BuildFootprintWorldCoords(
	const UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
) const
{
	TArray<FGridCoord> WorldCoords;
	if (!BuildingData)
	{
		return WorldCoords;
	}

	for (int32 LocalY = 0; LocalY < BuildingData->FootprintSize.Y; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < BuildingData->FootprintSize.X; ++LocalX)
		{
			const FGridCoord RotatedLocalCoord = RotateLocalCoord(FGridCoord(LocalX, LocalY), BuildingData->FootprintSize, Direction);
			WorldCoords.Add(FGridCoord(OriginCoord.X + RotatedLocalCoord.X, OriginCoord.Y + RotatedLocalCoord.Y));
		}
	}

	return WorldCoords;
}

bool AFactoryManager::CanPlaceBuilding(
	const UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
) const
{
	if (!BuildingData || BuildingData->FootprintSize.X <= 0 || BuildingData->FootprintSize.Y <= 0)
	{
		return false;
	}

	for (const FGridCoord& CellCoord : BuildFootprintWorldCoords(BuildingData, OriginCoord, Direction))
	{
		if (IsCellOccupied(CellCoord))
		{
			return false;
		}
	}

	return true;
}

TArray<FFactoryPlacedBuildingPort> AFactoryManager::BuildWorldPorts(
	const UFactoryBuildingDataAsset* BuildingData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
) const
{
	TArray<FFactoryPlacedBuildingPort> WorldPorts;
	if (!BuildingData)
	{
		return WorldPorts;
	}

	for (const FFactoryBuildingPort& Port : BuildingData->Ports)
	{
		if (!IsPortOnFootprintBoundary(Port, BuildingData->FootprintSize))
		{
			continue;
		}

		const FGridCoord RotatedLocalCoord = RotateLocalCoord(Port.LocalCoord, BuildingData->FootprintSize, Direction);

		FFactoryPlacedBuildingPort WorldPort;
		WorldPort.WorldCoord = FGridCoord(OriginCoord.X + RotatedLocalCoord.X, OriginCoord.Y + RotatedLocalCoord.Y);
		WorldPort.Direction = RotateDirection(Port.Direction, Direction);
		WorldPort.PortType = Port.PortType;
		WorldPort.AcceptedItemId = Port.AcceptedItemId;
		WorldPorts.Add(WorldPort);
	}

	return WorldPorts;
}

bool AFactoryManager::IsLocalCoordInsideFootprint(const FGridCoord& LocalCoord, const FIntPoint& FootprintSize) const
{
	return LocalCoord.X >= 0
		&& LocalCoord.Y >= 0
		&& LocalCoord.X < FootprintSize.X
		&& LocalCoord.Y < FootprintSize.Y;
}

bool AFactoryManager::IsPortOnFootprintBoundary(const FFactoryBuildingPort& Port, const FIntPoint& FootprintSize) const
{
	if (!IsLocalCoordInsideFootprint(Port.LocalCoord, FootprintSize) || Port.Direction == EFactoryDirection::None)
	{
		return false;
	}

	const FGridCoord DirectionOffset = DirectionToGridOffset(Port.Direction);
	const FGridCoord NeighborCoord(Port.LocalCoord.X + DirectionOffset.X, Port.LocalCoord.Y + DirectionOffset.Y);
	return !IsLocalCoordInsideFootprint(NeighborCoord, FootprintSize);
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

void AFactoryManager::SetSelectedBuilding(UFactoryBuildingDataAsset* BuildingData)
{
	SelectedBuilding = BuildingData;
}

void AFactoryManager::ClearSelectedBuilding()
{
	SelectedBuilding = nullptr;
}

bool AFactoryManager::HasSelectedBuilding() const
{
	return SelectedBuilding != nullptr;
}

bool AFactoryManager::TryPlaceSelectedBuildingAtHoveredCell()
{
	FGridCoord OriginCoord;
	if (!GetHoveredCellCoord(OriginCoord))
	{
		return false;
	}

	return TryPlaceSelectedBuilding(OriginCoord);
}

bool AFactoryManager::TryPlaceSelectedBuilding(const FGridCoord& OriginCoord)
{
	if (!SelectedBuilding)
	{
		return false;
	}

	return TryPlaceBuilding(SelectedBuilding, OriginCoord, CurrentBuildDirection);
}

// MARK: Removal control

//			     Machine
//			  /
// Buildable  -- Others
//			  \
//			     Conveyor

bool AFactoryManager::RemoveBuildableAtHoveredCell()
{
	FGridCoord Coord;
	if (!GetHoveredCellCoord(Coord))
	{
		return false;
	}

	return RemoveBuildableAtCoord(Coord);
}

bool AFactoryManager::RemoveBuildableAtCoord(const FGridCoord& Coord)
{
	const FFactoryGridCell* Cell = GetCell(Coord);
	if (!Cell || Cell->IsEmpty())
	{
		return false;
	}

	switch (Cell->Occupancy)
	{
	case EFactoryCellOccupancy::Conveyor:
		if (!RemoveConveyorAtCoord(Cell->ConveyorCoord))
		{
			return false;
		}
		break;
	case EFactoryCellOccupancy::Building:
		if (!RemoveBuildingAtCoord(Coord))
		{
			return false;
		}
		break;
	case EFactoryCellOccupancy::Blocked:
	case EFactoryCellOccupancy::Empty:
	default:
		return false;
	}

	RefreshHoveredCellFromMouseRaycast(true);
	return true;
}

bool AFactoryManager::RemoveConveyorAtHoveredCell()
{
	FGridCoord Coord;
	if (!GetHoveredCellCoord(Coord))
	{
		return false;
	}

	return RemoveConveyorAtCoord(Coord);
}

bool AFactoryManager::RemoveConveyorAtCoord(const FGridCoord& Coord)
{
	FFactoryConveyorSegment RemovedSegment;
	if (!ConveyorsByCellCoord.RemoveAndCopyValue(Coord, RemovedSegment))
	{
		return false;
	}

	if (RemovedSegment.ConveyorData)
	{
		if (UHierarchicalInstancedStaticMeshComponent** VisualComponent = ConveyorVisualComponentsByData.Find(RemovedSegment.ConveyorData))
		{
			if (*VisualComponent && RemovedSegment.VisualInstanceIndex != INDEX_NONE)
			{
				(*VisualComponent)->RemoveInstance(RemovedSegment.VisualInstanceIndex);
				RepairConveyorVisualInstanceIndices(RemovedSegment.ConveyorData);
			}
		}
	}

	if (FFactoryGridCell* Cell = GetOrCreateCell(Coord))
	{
		Cell->Occupancy = EFactoryCellOccupancy::Empty;
		Cell->Direction = EFactoryDirection::None;
		Cell->BuildingId = INDEX_NONE;
		Cell->ConveyorCoord = FGridCoord();
	}

	return true;
}

bool AFactoryManager::RemoveBuildingAtCoord(const FGridCoord& Coord)
{
	const FFactoryPlacedBuildingInstance* RemovedInstance = BuildingInstancesByCellCoord.Find(Coord);
	if (!RemovedInstance || !RemovedInstance->BuildingActor)
	{
		return false;
	}

	const FFactoryPlacedBuildingInstance RemovedInstanceCopy = *RemovedInstance;
	AFactoryBuilding* BuildingActor = RemovedInstanceCopy.BuildingActor;
	const int32 BuildingId = BuildingActor->BuildingId;

	for (const FGridCoord& CellCoord : RemovedInstanceCopy.OccupiedCoords)
	{
		BuildingInstancesByCellCoord.Remove(CellCoord);

		FFactoryGridCell* Cell = GetOrCreateCell(CellCoord);
		if (!Cell || Cell->BuildingId != BuildingId)
		{
			continue;
		}

		Cell->Occupancy = EFactoryCellOccupancy::Empty;
		Cell->Direction = EFactoryDirection::None;
		Cell->BuildingId = INDEX_NONE;
		Cell->ConveyorCoord = FGridCoord();
	}

	MachineRuntimeData.RemoveAllSwap(
		[BuildingId](const FFactoryMachineRuntimeData& RuntimeData)
		{
			return RuntimeData.BuildingId == BuildingId;
		}
	);

	BuildingActors[BuildingId] = nullptr;
	BuildingActor->Destroy();

	return true;
}

// MARK: Build direction

void AFactoryManager::RotateBuildDirectionClockwise()
{
	CurrentBuildDirection = GetClockwiseDirection(CurrentBuildDirection);
}

void AFactoryManager::RotateBuildDirectionCounterClockwise()
{
	CurrentBuildDirection = GetCounterClockwiseDirection(CurrentBuildDirection);
}

EFactoryDirection AFactoryManager::GetClockwiseDirection(EFactoryDirection Direction) const
{
	switch (Direction)
	{
	case EFactoryDirection::Up:
		return EFactoryDirection::Right;
	case EFactoryDirection::Right:
		return EFactoryDirection::Down;
	case EFactoryDirection::Down:
		return EFactoryDirection::Left;
	case EFactoryDirection::Left:
		return EFactoryDirection::Up;
	case EFactoryDirection::None:
	default:
		return EFactoryDirection::Up;
	}
}

EFactoryDirection AFactoryManager::GetCounterClockwiseDirection(EFactoryDirection Direction) const
{
	switch (Direction)
	{
	case EFactoryDirection::Up:
		return EFactoryDirection::Left;
	case EFactoryDirection::Left:
		return EFactoryDirection::Down;
	case EFactoryDirection::Down:
		return EFactoryDirection::Right;
	case EFactoryDirection::Right:
		return EFactoryDirection::Up;
	case EFactoryDirection::None:
	default:
		return EFactoryDirection::Up;
	}
}

// MARK: Hover state queries

bool AFactoryManager::HasHoveredCell() const
{
	return bHasHoveredCell;
}

bool AFactoryManager::GetHoveredCellCoord(FGridCoord& OutCoord) const
{
	if (!bHasHoveredCell)
	{
		return false;
	}

	OutCoord = HoveredCellCoord;
	return true;
}

// MARK: Grid storage queries

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

// MARK: Debug/UI entry points

void AFactoryManager::SetDebugHoveredCell(const FGridCoord& Coord)
{
	SetHoveredCell(Coord);
}

void AFactoryManager::ClearDebugHoveredCell()
{
	ClearHoveredCell();
}

void AFactoryManager::SetDeveloperModeWidget(UFactoryDeveloperModeWidget* Widget)
{
	DeveloperModeWidget = Widget;

	if (bHasHoveredCell)
	{
		UpdateDeveloperModeCoordDisplay(HoveredCellCoord);
	}
}

// Simulation

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
	Chunks.FindOrAdd(FGridCoord(-1, 0));
	Chunks.FindOrAdd(FGridCoord(0, -1));
	Chunks.FindOrAdd(FGridCoord(-1, -1));
}

// MARK: Conveyor placement and visuals

bool AFactoryManager::TryPlaceConveyor(
	UFactoryBuildingDataAsset* ConveyorData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
)
{
	if (!CanPlaceConveyor(ConveyorData, OriginCoord, Direction))
	{
		return false;
	}

	FFactoryGridCell* Cell = GetOrCreateCell(OriginCoord);
	if (!Cell)
	{
		return false;
	}

	FFactoryConveyorSegment ConveyorSegment;
	ConveyorSegment.Coord = OriginCoord;
	ConveyorSegment.Direction = Direction;
	ConveyorSegment.ConveyorData = ConveyorData;
	ConveyorSegment.WorldPorts = BuildWorldPorts(ConveyorData, OriginCoord, Direction);
	ConveyorSegment.VisualInstanceIndex = AddConveyorVisual(ConveyorData, OriginCoord, Direction);
	ConveyorsByCellCoord.Add(OriginCoord, ConveyorSegment);

	Cell->Occupancy = EFactoryCellOccupancy::Conveyor;
	Cell->Direction = Direction;
	Cell->BuildingId = INDEX_NONE;
	Cell->ConveyorCoord = OriginCoord;

	UpdateDeveloperModeCoordDisplay(OriginCoord);

	return true;
}

bool AFactoryManager::CanPlaceConveyor(
	UFactoryBuildingDataAsset* ConveyorData,
	const FGridCoord& OriginCoord,
	EFactoryDirection Direction
) const
{
	return ConveyorData
		&& ConveyorData->bIsConveyor
		&& ConveyorData->ConveyorMesh
		&& CanPlaceBuilding(ConveyorData, OriginCoord, Direction);
}

UHierarchicalInstancedStaticMeshComponent* AFactoryManager::GetOrCreateConveyorVisualComponent(UFactoryBuildingDataAsset* ConveyorData)
{
	if (!ConveyorData || !ConveyorData->ConveyorMesh)
	{
		return nullptr;
	}

	if (UHierarchicalInstancedStaticMeshComponent** ExistingComponent = ConveyorVisualComponentsByData.Find(ConveyorData))
	{
		return *ExistingComponent;
	}

	const FName ComponentName = MakeUniqueObjectName(
		this,
		UHierarchicalInstancedStaticMeshComponent::StaticClass(),
		*FString::Printf(TEXT("ConveyorVisual_%s"), *ConveyorData->GetName())
	);

	UHierarchicalInstancedStaticMeshComponent* ConveyorVisualComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, ComponentName);
	if (!ConveyorVisualComponent)
	{
		return nullptr;
	}

	ConveyorVisualComponent->SetupAttachment(TransformComponent);
	ConveyorVisualComponent->SetStaticMesh(ConveyorData->ConveyorMesh);
	ConveyorVisualComponent->RegisterComponent();
	AddInstanceComponent(ConveyorVisualComponent);

	ConveyorVisualComponentsByData.Add(ConveyorData, ConveyorVisualComponent);
	return ConveyorVisualComponent;
}

int32 AFactoryManager::AddConveyorVisual(UFactoryBuildingDataAsset* ConveyorData, const FGridCoord& Coord, EFactoryDirection Direction)
{
	UHierarchicalInstancedStaticMeshComponent* ConveyorVisualComponent = GetOrCreateConveyorVisualComponent(ConveyorData);
	if (!ConveyorVisualComponent)
	{
		return INDEX_NONE;
	}

	const FTransform InstanceTransform(
		FRotator(0.0f, DirectionToYaw(Direction) + ConveyorData->ConveyorMeshYawOffset, 0.0f),
		GridCellCenterToWorld(Coord),
		FVector::OneVector
	);

	return ConveyorVisualComponent->AddInstance(InstanceTransform, true);
}

void AFactoryManager::RepairConveyorVisualInstanceIndices(UFactoryBuildingDataAsset* ConveyorData)
{
	if (!ConveyorData)
	{
		return;
	}

	UHierarchicalInstancedStaticMeshComponent** VisualComponent = ConveyorVisualComponentsByData.Find(ConveyorData);
	if (!VisualComponent || !*VisualComponent)
	{
		return;
	}

	for (TPair<FGridCoord, FFactoryConveyorSegment>& ConveyorPair : ConveyorsByCellCoord)
	{
		FFactoryConveyorSegment& ConveyorSegment = ConveyorPair.Value;
		if (ConveyorSegment.ConveyorData != ConveyorData)
		{
			continue;
		}

		const FVector ExpectedLocation = GridCellCenterToWorld(ConveyorSegment.Coord);
		ConveyorSegment.VisualInstanceIndex = INDEX_NONE;

		const int32 InstanceCount = (*VisualComponent)->GetInstanceCount();
		for (int32 InstanceIndex = 0; InstanceIndex < InstanceCount; ++InstanceIndex)
		{
			FTransform InstanceTransform;
			if (!(*VisualComponent)->GetInstanceTransform(InstanceIndex, InstanceTransform, true))
			{
				continue;
			}

			if (InstanceTransform.GetLocation().Equals(ExpectedLocation, 1.0f))
			{
				ConveyorSegment.VisualInstanceIndex = InstanceIndex;
				break;
			}
		}
	}
}

float AFactoryManager::DirectionToYaw(EFactoryDirection Direction) const
{
	switch (Direction)
	{
	case EFactoryDirection::Up:
		return 0.0f;
	case EFactoryDirection::Right:
		return 90.0f;
	case EFactoryDirection::Down:
		return 180.0f;
	case EFactoryDirection::Left:
		return 270.0f;
	case EFactoryDirection::None:
	default:
		return 0.0f;
	}
}

// MARK: Hover and UI updates

void AFactoryManager::UpdateHoveredCellFromMouseRaycast()
{
	RefreshHoveredCellFromMouseRaycast(false);
}

void AFactoryManager::RefreshHoveredCellFromMouseRaycast(bool bForceUpdate)
{
	FGridCoord NewHoveredCellCoord;
	if (!TryGetMouseRaycastGridCoord(NewHoveredCellCoord) || !GetCell(NewHoveredCellCoord))
	{
		if (bHasPreviousHoveredCellCoord || bForceUpdate)
		{
			ClearHoveredCell();
			bHasPreviousHoveredCellCoord = false;
		}

		return;
	}

	if (!bForceUpdate && bHasPreviousHoveredCellCoord && PreviousHoveredCellCoord == NewHoveredCellCoord)
	{
		return;
	}

	PreviousHoveredCellCoord = NewHoveredCellCoord;
	bHasPreviousHoveredCellCoord = true;
	SetHoveredCell(NewHoveredCellCoord);
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

void AFactoryManager::SetHoveredCell(const FGridCoord& Coord)
{
	HoveredCellCoord = Coord;
	bHasHoveredCell = true;

	DebugHoveredCellCoord = Coord;
	bHasDebugHoveredCell = true;

	UpdateDeveloperModeCoordDisplay(Coord);
}

void AFactoryManager::ClearHoveredCell()
{
	bHasHoveredCell = false;
	bHasDebugHoveredCell = false;

	if (DeveloperModeWidget)
	{
		DeveloperModeWidget->ClearCurrentCellCoord();
		DeveloperModeWidget->ClearBuildingOnCell();
	}
}

void AFactoryManager::UpdateDeveloperModeCoordDisplay(const FGridCoord& CellCoord)
{
	if (!DeveloperModeWidget)
	{
		return;
	}

	DeveloperModeWidget->SetCurrentCellCoord(CellCoord, EFactoryCoordType::Cell);
	DeveloperModeWidget->SetCurrentCellCoord(WorldCoordToChunkCoord(CellCoord), EFactoryCoordType::Chunk);

	const FFactoryGridCell* Cell = GetCell(CellCoord);
	if (Cell && Cell->IsOccupied())
	{
		DeveloperModeWidget->UpdateBuildingOnCell(*Cell);
	}
	else
	{
		DeveloperModeWidget->ClearBuildingOnCell();
	}
}

// MARK: Debug drawing

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

	if (bDrawDebugHoveredCell && bHasHoveredCell && GetCell(HoveredCellCoord))
	{
		DrawDebugCellBounds(HoveredCellCoord, DebugHoveredCellColor, DebugHoveredCellLineThickness);
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

// MARK: Grid helpers

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

FGridCoord AFactoryManager::DirectionToGridOffset(EFactoryDirection Direction) const
{
	switch (Direction)
	{
	case EFactoryDirection::Up:
		return FGridCoord(1, 0);
	case EFactoryDirection::Right:
		return FGridCoord(0, 1);
	case EFactoryDirection::Down:
		return FGridCoord(-1, 0);
	case EFactoryDirection::Left:
		return FGridCoord(0, -1);
	case EFactoryDirection::None:
	default:
		return FGridCoord(0, 0);
	}
}

FGridCoord AFactoryManager::RotateLocalCoord(const FGridCoord& LocalCoord, const FIntPoint& FootprintSize, EFactoryDirection Direction) const
{
	switch (Direction)
	{
	case EFactoryDirection::Up:
	case EFactoryDirection::None:
		return LocalCoord;
	case EFactoryDirection::Right:
		return FGridCoord(LocalCoord.Y, FootprintSize.X - 1 - LocalCoord.X);
	case EFactoryDirection::Down:
		return FGridCoord(FootprintSize.X - 1 - LocalCoord.X, FootprintSize.Y - 1 - LocalCoord.Y);
	case EFactoryDirection::Left:
		return FGridCoord(FootprintSize.Y - 1 - LocalCoord.Y, LocalCoord.X);
	default:
		return LocalCoord;
	}
}

FIntPoint AFactoryManager::GetRotatedFootprintSize(const FIntPoint& FootprintSize, EFactoryDirection Direction) const
{
	if (Direction == EFactoryDirection::Right || Direction == EFactoryDirection::Left)
	{
		return FIntPoint(FootprintSize.Y, FootprintSize.X);
	}

	return FootprintSize;
}

EFactoryDirection AFactoryManager::RotateDirection(EFactoryDirection BaseDirection, EFactoryDirection BuildDirection) const
{
	switch (BuildDirection)
	{
	case EFactoryDirection::Up:
	case EFactoryDirection::None:
		return BaseDirection;
	case EFactoryDirection::Right:
		return GetClockwiseDirection(BaseDirection);
	case EFactoryDirection::Down:
		return GetClockwiseDirection(GetClockwiseDirection(BaseDirection));
	case EFactoryDirection::Left:
		return GetCounterClockwiseDirection(BaseDirection);
	default:
		return BaseDirection;
	}
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