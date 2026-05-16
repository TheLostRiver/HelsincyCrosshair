// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HelsincyIndicatorRenderer.h"
#include "HelsincyIndicatorRendererArrow.generated.h"

/**
 * 
 */
UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyIndicatorRendererArrow : public UHelsincyIndicatorRenderer
{
	GENERATED_BODY()

public:
	UHelsincyIndicatorRendererArrow();
	
	virtual void DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale) override;
	virtual FVector2D GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& Profile, float Scale) const override;
	virtual void DrawPointerResolved(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, const FHelsincy_ResolvedDamageIndicatorPlacement& Placement, float Alpha, float Scale) override;
	
};
