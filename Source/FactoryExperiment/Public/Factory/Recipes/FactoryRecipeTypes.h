#pragma once

#include "CoreMinimal.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "FactoryRecipeTypes.generated.h"

USTRUCT(BlueprintType)
struct FFactoryResourceAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFactoryResourceType ResourceType = EFactoryResourceType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;
};

USTRUCT(BlueprintType)
struct FFactoryResourceAmountMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EFactoryResourceType, int32> Resources;
};
