// Copyright , Helsincy Games. All Rights Reserved.

#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "GameplayTagsManager.h"

const FGameplayTag FHelsincyDamageIndicator_Tags::Indicator_Style_Arrow = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Indicator.Style.Arrow"));
const FGameplayTag FHelsincyDamageIndicator_Tags::Indicator_Style_Image = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Indicator.Style.Image"));
const FGameplayTag FHelsincyDamageIndicator_Tags::Indicator_Style_Arc = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Indicator.Style.Arc"));

void FHelsincyDamageIndicator_Tags::InitializeNativeTags()
{
	// Tags are registered via static initialization
}
