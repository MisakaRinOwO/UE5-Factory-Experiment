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

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	TObjectPtr<UFactoryRecipeDataAsset> RecipeDatabase;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Factory")
	float SimulationStepInterval = 0.2f;

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

private:
	void SimulationStep();

	void UpdateMachines(float DeltaTime);
	void UpdateConveyors(float DeltaTime);

	FGridCoord WorldCoordToChunkCoord(const FGridCoord& WorldCoord) const;
	FGridCoord WorldCoordToLocalCoord(const FGridCoord& WorldCoord) const;

	int32 RegisterBuildingActor(AFactoryBuilding* Building);
};