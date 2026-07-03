#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryBuildingTypes.generated.h"

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