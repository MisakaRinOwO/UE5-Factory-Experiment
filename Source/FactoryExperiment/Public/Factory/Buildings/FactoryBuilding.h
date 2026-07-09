#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "GameFramework/Actor.h"
#include "Grid/GridCoord.h"
#include "FactoryBuilding.generated.h"

class UFactoryBuildingDataAsset;

UCLASS()
class FACTORYEXPERIMENT_API AFactoryBuilding : public AActor
{
	GENERATED_BODY()

public:
	AFactoryBuilding();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord OriginCoord;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UFactoryBuildingDataAsset> BuildingDefinition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryPlacedBuildingPort> WorldPorts;

	UFUNCTION(BlueprintCallable)
	virtual void InitializeBuilding(int32 InBuildingId, FGridCoord InOriginCoord);

	UFUNCTION(BlueprintCallable)
	void SetWorldPorts(const TArray<FFactoryPlacedBuildingPort>& InWorldPorts);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnBuildingInitialized();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnProductionStateChanged(bool bIsWorking, bool bOutputBlocked);
};