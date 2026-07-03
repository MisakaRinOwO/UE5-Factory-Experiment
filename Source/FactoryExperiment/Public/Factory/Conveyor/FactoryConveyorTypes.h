#pragma once

#include "CoreMinimal.h"
#include "Factory/FactoryTypes.h"
#include "Grid/GridCoord.h"
#include "FactoryConveyorTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryConveyorSegment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord Coord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 SegmentId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 NextSegmentId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentItemId = -1;
};