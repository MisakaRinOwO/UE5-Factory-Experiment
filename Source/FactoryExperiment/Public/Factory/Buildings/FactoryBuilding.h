#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintCallable)
	virtual void InitializeBuilding(int32 InBuildingId, FGridCoord InOriginCoord);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnBuildingInitialized();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnProductionStateChanged(bool bIsWorking, bool bOutputBlocked);
};