#pragma once

#include "CoreMinimal.h"
#include "Factory/Buildings/FactoryBuildingTypes.h"
#include "Factory/FactoryTypes.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "Grid/GridCoord.h"
#include "FactoryConveyorTypes.generated.h"

class UFactoryBuildingDataAsset;

USTRUCT(BlueprintType)
struct FFactoryConveyorSegment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord Coord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryDirection Direction = EFactoryDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UFactoryBuildingDataAsset> ConveyorData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryPlacedBuildingPort> WorldPorts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 VisualInstanceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasNextCoord = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord NextCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentItemId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EFactoryResourceType CurrentResourceType = EFactoryResourceType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ResourceVisualInstanceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord ResourceVisualFromCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord ResourceVisualToCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ResourceVisualMoveElapsed = 0.0f;
};
