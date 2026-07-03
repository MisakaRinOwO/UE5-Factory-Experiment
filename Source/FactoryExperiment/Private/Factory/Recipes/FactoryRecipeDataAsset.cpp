#include "Factory/Recipes/FactoryRecipeDataAsset.h"

const FFactoryRecipe* UFactoryRecipeDataAsset::FindRecipeById(FName RecipeId) const
{
	for (const FFactoryRecipe& Recipe : Recipes)
	{
		if (Recipe.RecipeId == RecipeId)
		{
			return &Recipe;
		}
	}

	return nullptr;
}