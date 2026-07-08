#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryDebugTypes.generated.h"

UENUM(BlueprintType)
enum class EFactoryCoordType : uint8
{
	Cell UMETA(DisplayName = "Cell"),
	Chunk UMETA(DisplayName = "Chunk")
};

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
	FGridCoord ConveyorCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsValidForPath = true;
};