#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factory/Resources/FactoryResourceMapDataAsset.h"
#include "FactoryResourceDataAsset.generated.h"

class UStaticMesh;

UCLASS(BlueprintType)
class FACTORYEXPERIMENT_API UFactoryResourceDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources|Visuals")
	TMap<EFactoryResourceType, TObjectPtr<UStaticMesh>> ResourceMeshesByType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources|Visuals")
	TMap<EFactoryResourceType, TObjectPtr<UStaticMesh>> ResourceVeinMeshesByType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources")
	TMap<EFactoryResourceType, int32> StackSizeByResourceType;

	UFUNCTION(BlueprintCallable, Category = "Resources")
	int32 GetStackSize(EFactoryResourceType ResourceType) const;
};
