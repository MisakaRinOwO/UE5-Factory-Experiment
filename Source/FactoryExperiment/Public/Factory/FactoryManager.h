#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

#include "Factory/Buildings/FactoryMachineTypes.h"
#include "Factory/Conveyor/FactoryConveyorTypes.h"
#include "Factory/Grid/FactoryGridTypes.h"
#include "Grid/GridCoord.h"

#include "FactoryManager.generated.h"

class AFactoryBuilding;
class USceneComponent;
class UFactoryBuildingDataAsset;
class UFactoryRecipeDataAsset;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factory")
	TObjectPtr<USceneComponent> TransformComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	float SimulationStepInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugCells = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugChunks = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bDrawDebugHoveredCell = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	bool bUpdateHoveredCellFromMouseRaycast = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Factory|Debug")
	FGridCoord DebugHoveredCellCoord;

private:
	UPROPERTY()
	TMap<FGridCoord, FFactoryChunk> Chunks;

	UPROPERTY()
	TArray<TObjectPtr<AFactoryBuilding>> BuildingActors;

	UPROPERTY()
	TArray<FFactoryMachineRuntimeData> MachineRuntimeData;

	UPROPERTY()
	TArray<FFactoryConveyorSegment> Conveyors;

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

	FFactoryGridCell* GetOrCreateCell(const FGridCoord& Coord);
	const FFactoryGridCell* GetCell(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory")
	TArray<FGridCoord> GetNeighbors(const FGridCoord& Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Factory|Debug")
	void SetDebugHoveredCell(const FGridCoord& Coord);

	UFUNCTION(BlueprintCallable, Category = "Factory|Debug")
	void ClearDebugHoveredCell();

private:
	static constexpr float CellSize = 100.0f;

	void SimulationStep();

	void UpdateMachines(float DeltaTime);
	void UpdateConveyors(float DeltaTime);

	void CreateInitialChunks();
	void UpdateHoveredCellFromMouseRaycast();
	bool TryGetMouseRaycastGridCoord(FGridCoord& OutCoord) const;
	void DrawDebugGrid() const;
	void DrawDebugCellGridForChunk(const FGridCoord& ChunkCoord) const;
	void DrawDebugChunkBounds(const FGridCoord& ChunkCoord) const;
	void DrawDebugCellBounds(const FGridCoord& Coord, const FColor& Color, float Thickness) const;
	void DrawDebugCellBounds(const FGridCoord& MinCoord, const FGridCoord& MaxCoord, const FColor& Color, float Thickness) const;

	FGridCoord WorldLocationToGridCoord(const FVector& WorldLocation) const;
	FVector GridCellCenterToWorld(const FGridCoord& Coord) const;
	FVector GridBoundaryToWorld(float BoundaryX, float BoundaryY) const;

	FGridCoord WorldCoordToChunkCoord(const FGridCoord& WorldCoord) const;
	FGridCoord WorldCoordToLocalCoord(const FGridCoord& WorldCoord) const;

	int32 RegisterBuildingActor(AFactoryBuilding* Building);

	bool bHasDebugHoveredCell = false;
	bool bHasPreviousHoveredCellCoord = false;
	FGridCoord PreviousHoveredCellCoord;
};