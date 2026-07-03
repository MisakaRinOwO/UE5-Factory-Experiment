#pragma once

#include "CoreMinimal.h"
#include "Factory/Items/FactoryItemTypes.h"
#include "FactoryMachineTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryMachineRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 BuildingId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CraftProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsWorking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bOutputBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryItemStack> InputInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FFactoryItemStack> OutputInventory;
};