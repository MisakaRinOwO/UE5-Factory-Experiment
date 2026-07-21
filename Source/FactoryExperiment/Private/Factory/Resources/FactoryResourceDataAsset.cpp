#include "Factory/Resources/FactoryResourceDataAsset.h"

int32 UFactoryResourceDataAsset::GetStackSize(EFactoryResourceType ResourceType) const
{
	if (const int32* StackSize = StackSizeByResourceType.Find(ResourceType))
	{
		return FMath::Max(0, *StackSize);
	}

	return 1;
}
