#pragma once

#include "CoreMinimal.h"
#include "Factory/Items/FactoryItemTypes.h"
#include "FactoryRecipeTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CraftTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFactoryItemStack> Inputs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFactoryItemStack> Outputs;
};