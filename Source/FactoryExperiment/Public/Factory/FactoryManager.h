#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Factory/Buildings/FactoryMachineTypes.h"
#include "Factory/Conveyor/FactoryConveyorTypes.h"
#include "Factory/Debug/FactoryDebugTypes.h"
#include "Factory/Grid/FactoryGridTypes.h"
#include "Grid/GridCoord.h"

#include "FactoryManager.generated.h"

class AFactoryBuilding;
class UHierarchicalInstancedStaticMeshComponent;
class USceneComponent;
class UFactoryBuildingDataAsset;
class UFactoryDeveloperModeWidget;
class UFactoryRecipeDataAsset;
class UFactoryResourceMapDataAsset;
class UUserWidget;

UCLASS()
class FACTORYEXPERIMENT_API AFactoryManager : public AActor
{
	GENERATED_BODY()

public:
	AFactoryManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	TObjectPtr<UFactoryRecipeDataAsset> RecipeDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	TObjectPtr<UFactoryResourceMapDataAsset> ResourceMapData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory")
	TObjectPtr<USceneComponent> TransformComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	float SimulationStepInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Placement")
	TObjectPtr<UFactoryBuildingDataAsset> SelectedBuilding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Placement")
	EFactoryDirection CurrentBuildDirection = EFactoryDirection::Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugCells = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugChunks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugHoveredCell = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bUpdateHoveredCellFromMouseRaycast = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory|Debug")
	FGridCoord DebugHoveredCellCoord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug|UI")
	TObjectPtr<UFactoryDeveloperModeWidget> DeveloperModeWidget;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TMap<FGridCoord, FFactoryChunk> Chunks;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AFactoryBuilding>> BuildingActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FFactoryMachineRuntimeData> MachineRuntimeData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TMap<FGridCoord, FFactoryPlacedBuildingInstance> BuildingInstancesByCellCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
	TMap<FGridCoord, FFactoryConveyorSegment> ConveyorsByCellCoord;

	TMap<UFactoryBuildingDataAsset*, UHierarchicalInstancedStaticMeshComponent*> ConveyorVisualComponentsByData;

	FTimerHandle SimulationTimerHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Factory")
	bool TryPlaceBuilding(
		UFactoryBuildingDataAsset* BuildingData,
		const FGridCoord& OriginCoord,
		EFactoryDirection Direction
	);

	UFUNCTION(BlueprintCallable, Category = "Factory")
	bool CanPlaceBuilding(
		const UFactoryBuildingDataAsset* BuildingData,
		const FGridCoord& OriginCoord,
		EFactoryDirection Direction
	) const;

	UFUNCTION(BlueprintCallable, Category = "Factory")
	bool IsCellOccupied(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory")
	bool IsCellValidForPath(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	void SetSelectedBuilding(UFactoryBuildingDataAsset* BuildingData);

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	void ClearSelectedBuilding();

	UFUNCTION(BlueprintPure, Category = "Factory|Placement")
	bool HasSelectedBuilding() const;

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool TryPlaceSelectedBuildingAtHoveredCell();

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool TryPlaceSelectedBuilding(const FGridCoord& OriginCoord);

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool RemoveBuildableAtHoveredCell();

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool RemoveBuildableAtCoord(const FGridCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool RemoveConveyorAtHoveredCell();

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	bool RemoveConveyorAtCoord(const FGridCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	void RotateBuildDirectionClockwise();

	UFUNCTION(BlueprintCallable, Category = "Factory|Placement")
	void RotateBuildDirectionCounterClockwise();

	UFUNCTION(BlueprintPure, Category = "Factory|Placement")
	EFactoryDirection GetClockwiseDirection(EFactoryDirection Direction) const;

	UFUNCTION(BlueprintPure, Category = "Factory|Placement")
	EFactoryDirection GetCounterClockwiseDirection(EFactoryDirection Direction) const;

	UFUNCTION(BlueprintPure, Category = "Factory|Hover")
	bool HasHoveredCell() const;

	UFUNCTION(BlueprintPure, Category = "Factory|Hover")
	bool GetHoveredCellCoord(FGridCoord& OutCoord) const;

	FFactoryGridCell* GetOrCreateCell(const FGridCoord& Coord);
	const FFactoryGridCell* GetCell(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory")
	TArray<FGridCoord> GetNeighbors(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory|Debug")
	void SetDebugHoveredCell(const FGridCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Factory|Debug")
	void ClearDebugHoveredCell();

	UFUNCTION(BlueprintCallable, Category = "Factory|Debug|UI")
	void SetDeveloperModeWidget(UFactoryDeveloperModeWidget* Widget);

private:
	static constexpr float CellSize = 100.0f;

	// Simulation
	void SimulationStep();
	void UpdateMachines(float DeltaTime);
	void UpdateConveyors(float DeltaTime);

	// Placement internals
	void CreateInitialChunks();
	bool TryPlaceConveyor(UFactoryBuildingDataAsset* ConveyorData, const FGridCoord& OriginCoord, EFactoryDirection Direction);
	bool CanPlaceConveyor(UFactoryBuildingDataAsset* ConveyorData, const FGridCoord& OriginCoord, EFactoryDirection Direction) const;
	int32 RegisterBuildingActor(AFactoryBuilding* Building);
	TArray<FGridCoord> BuildFootprintWorldCoords(const UFactoryBuildingDataAsset* BuildingData, const FGridCoord& OriginCoord, EFactoryDirection Direction) const;
	TArray<FFactoryPlacedBuildingPort> BuildWorldPorts(const UFactoryBuildingDataAsset* BuildingData, const FGridCoord& OriginCoord, EFactoryDirection Direction) const;
	bool IsLocalCoordInsideFootprint(const FGridCoord& LocalCoord, const FIntPoint& FootprintSize) const;
	bool IsPortOnFootprintBoundary(const FFactoryBuildingPort& Port, const FIntPoint& FootprintSize) const;

	// Removal internals
	bool RemoveBuildingAtCoord(const FGridCoord& Coord);

	// Conveyor visuals
	UHierarchicalInstancedStaticMeshComponent* GetOrCreateConveyorVisualComponent(UFactoryBuildingDataAsset* ConveyorData);
	int32 AddConveyorVisual(UFactoryBuildingDataAsset* ConveyorData, const FGridCoord& Coord, EFactoryDirection Direction);
	void RepairConveyorVisualInstanceIndices(UFactoryBuildingDataAsset* ConveyorData);
	float DirectionToYaw(EFactoryDirection Direction) const;

	// Hover and UI
	void UpdateHoveredCellFromMouseRaycast();
	void RefreshHoveredCellFromMouseRaycast(bool bForceUpdate);
	bool TryGetMouseRaycastGridCoord(FGridCoord& OutCoord) const;
	void SetHoveredCell(const FGridCoord& Coord);
	void ClearHoveredCell();
	void UpdateDeveloperModeCoordDisplay(const FGridCoord& CellCoord);

	// Debug drawing
	void DrawDebugGrid() const;
	void DrawDebugCellGridForChunk(const FGridCoord& ChunkCoord) const;
	void DrawDebugChunkBounds(const FGridCoord& ChunkCoord) const;
	void DrawDebugCellBounds(const FGridCoord& Coord, const FColor& Color, float Thickness) const;
	void DrawDebugCellBounds(const FGridCoord& MinCoord, const FGridCoord& MaxCoord, const FColor& Color, float Thickness) const;

	// Grid helpers
	FGridCoord WorldLocationToGridCoord(const FVector& WorldLocation) const;
	FVector GridCellCenterToWorld(const FGridCoord& Coord) const;
	FVector GridBoundaryToWorld(float BoundaryX, float BoundaryY) const;

	FGridCoord DirectionToGridOffset(EFactoryDirection Direction) const;
	FGridCoord RotateLocalCoord(const FGridCoord& LocalCoord, const FIntPoint& FootprintSize, EFactoryDirection Direction) const;
	FIntPoint GetRotatedFootprintSize(const FIntPoint& FootprintSize, EFactoryDirection Direction) const;
	EFactoryDirection RotateDirection(EFactoryDirection BaseDirection, EFactoryDirection BuildDirection) const;
	FGridCoord WorldCoordToChunkCoord(const FGridCoord& WorldCoord) const;
	FGridCoord WorldCoordToLocalCoord(const FGridCoord& WorldCoord) const;

	bool bHasDebugHoveredCell = false;
	bool bHasHoveredCell = false;
	FGridCoord HoveredCellCoord;
	bool bHasPreviousHoveredCellCoord = false;
	FGridCoord PreviousHoveredCellCoord;
};