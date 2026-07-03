#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Recipes/FactoryRecipeTypes.h"
#include "FactoryRecipeDataAsset.generated.h"

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryRecipeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFactoryRecipe> Recipes;

	const FFactoryRecipe* FindRecipeById(FName RecipeId) const;
};