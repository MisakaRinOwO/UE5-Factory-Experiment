#pragma once

#include "CoreMinimal.h"
#include "Grid/GridCoord.h"
#include "FactoryItemTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryItemStack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 0;

	bool IsEmpty() const
	{
		return ItemId.IsNone() || Count <= 0;
	}
};

USTRUCT(BlueprintType)
struct FFactoryItemPacket
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName ItemTypeId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasCurrentConveyorCoord = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGridCoord CurrentConveyorCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Progress = 0.0f;
};