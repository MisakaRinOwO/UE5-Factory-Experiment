#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "FactoryResourceVisualDataAsset.generated.h"

class UStaticMesh;

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryResourceVisualDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TMap<EFactoryResourceType, TObjectPtr<UStaticMesh>> ResourceMeshesByType;
};
