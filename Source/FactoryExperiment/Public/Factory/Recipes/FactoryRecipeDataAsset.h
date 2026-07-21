#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Recipes/FactoryRecipeTypes.h"
#include "FactoryRecipeDataAsset.generated.h"

class UFactoryBuildingDataAsset;

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryRecipeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	FName RecipeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TMap<EFactoryResourceType, int32> InputResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TMap<EFactoryResourceType, int32> OutputResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0.0"))
	float CraftTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TSet<TObjectPtr<UFactoryBuildingDataAsset>> AllowedBuildings;

	bool IsAllowedForBuilding(const UFactoryBuildingDataAsset* BuildingData) const;
};
