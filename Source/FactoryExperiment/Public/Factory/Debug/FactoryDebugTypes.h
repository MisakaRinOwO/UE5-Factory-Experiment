#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryDebugTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryCellDebugInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord Coord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryCellOccupancy Occupancy = EFactoryCellOccupancy::Empty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ConveyorId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsValidForPath = true;
};