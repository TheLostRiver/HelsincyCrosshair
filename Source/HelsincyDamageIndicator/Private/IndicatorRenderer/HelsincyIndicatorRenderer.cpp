// Copyright , Helsincy Games. All Rights Reserved.


#include "IndicatorRenderer/HelsincyIndicatorRenderer.h"



void UHelsincyIndicatorRenderer::DrawPointer(UCanvas* Canvas, const FHelsincy_DamageIndicatorProfile& Profile, FVector2D Center, float Angle, float Alpha, float Scale)
{
	// 如果 Canvas 有效，且这是蓝图子类（或者没重写这个虚函数的C++类），调用蓝图事件
	// If Canvas is valid and this is a blueprint subclass (or a C++ class that didn't override this virtual), call blueprint event
	if (Canvas)
	{
		ReceiveDrawPointer(Canvas, Profile, Center, Angle, Alpha, Scale);
	}
}

FVector2D UHelsincyIndicatorRenderer::GetDesiredPointerSize(const FHelsincy_DamageIndicatorProfile& /*Profile*/, float /*Scale*/) const
{
	return FVector2D::ZeroVector;
}

void UHelsincyIndicatorRenderer::DrawPointerResolved(
	UCanvas* Canvas,
	const FHelsincy_DamageIndicatorProfile& Profile,
	const FHelsincy_ResolvedDamageIndicatorPlacement& Placement,
	float Alpha,
	float Scale)
{
	DrawPointer(Canvas, Profile, Placement.Origin, Placement.RotationAngle, Alpha, Scale);
}
