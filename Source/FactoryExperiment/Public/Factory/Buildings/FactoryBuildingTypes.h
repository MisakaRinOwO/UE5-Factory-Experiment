#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryBuildingTypes.generated.h"

class AFactoryBuilding;
class UFactoryBuildingDataAsset;

USTRUCT(BlueprintType)
struct FFactoryBuildingPort
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridCoord LocalCoord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryPortType PortType = EFactoryPortType::Input;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AcceptedItemId;
};

USTRUCT(BlueprintType)
struct FFactoryPlacedBuildingPort
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord WorldCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryPortType PortType = EFactoryPortType::Input;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName AcceptedItemId;
};

USTRUCT(BlueprintType)
struct FFactoryPlacedBuildingInstance
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AFactoryBuilding> BuildingActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UFactoryBuildingDataAsset> BuildingData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord OriginCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FGridCoord> OccupiedCoords;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryPlacedBuildingPort> WorldPorts;
};