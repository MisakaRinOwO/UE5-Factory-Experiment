#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryBuildingDataAsset.generated.h"

class AFactoryBuilding;

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryBuildingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName BuildingTypeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AFactoryBuilding> BuildingActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint FootprintSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFactoryBuildingPort> Ports;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsConveyor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DefaultRecipeId;
};