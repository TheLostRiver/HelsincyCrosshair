#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Library/HelsincyDamageIndicatorPlacementResolver.h"

namespace
{
	FHelsincy_DamageIndicatorProfile MakeWindowEdgeProfile()
	{
		FHelsincy_DamageIndicatorProfile Profile;
		Profile.PlacementMode = EHelsincyDamageIndicatorPlacementMode::WindowEdge;
		Profile.EdgeMargin = 48.0f;
		Profile.EdgeCornerPadding = 24.0f;
		return Profile;
	}

	FHelsincy_ResolvedDamageIndicatorPlacement ResolveWindowEdgeTestPlacement(const float Angle)
	{
		return HelsincyDamageIndicatorPlacement::ResolveDamageIndicatorPlacement(
			MakeWindowEdgeProfile(),
			FVector2D(1280.0f, 720.0f),
			FVector2D(640.0f, 360.0f),
			Angle,
			1.0f,
			FVector2D(48.0f, 48.0f));
	}

	float ScreenDeltaToIndicatorAngle(const FVector2D& Delta)
	{
		return FMath::RadiansToDegrees(FMath::Atan2(Delta.X, -Delta.Y));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyDamageIndicatorWindowEdgeCornerContinuityTest,
	"HelsincyCrosshair.DamageIndicator.Placement.WindowEdgeCornerContinuity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyDamageIndicatorWindowEdgeCornerContinuityTest::RunTest(const FString& Parameters)
{
	const FVector2D Center(640.0f, 360.0f);
	const FVector2D SafeMin(72.0f, 72.0f);
	const FVector2D SafeMax(1208.0f, 648.0f);
	constexpr float CornerPadding = 24.0f;
	const float CornerTransitionAngles[] = {
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMax.X - CornerPadding, SafeMin.Y) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMax.X, SafeMin.Y + CornerPadding) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMax.X, SafeMax.Y - CornerPadding) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMax.X - CornerPadding, SafeMax.Y) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMin.X + CornerPadding, SafeMax.Y) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMin.X, SafeMax.Y - CornerPadding) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMin.X, SafeMin.Y + CornerPadding) - Center),
		ScreenDeltaToIndicatorAngle(FVector2D(SafeMin.X + CornerPadding, SafeMin.Y) - Center)
	};
	constexpr float EpsilonDegrees = 0.1f;
	constexpr float MaxAllowedStep = 8.0f;

	for (const float CornerAngle : CornerTransitionAngles)
	{
		const FHelsincy_ResolvedDamageIndicatorPlacement Before = ResolveWindowEdgeTestPlacement(CornerAngle - EpsilonDegrees);
		const FHelsincy_ResolvedDamageIndicatorPlacement After = ResolveWindowEdgeTestPlacement(CornerAngle + EpsilonDegrees);

		TestTrue(FString::Printf(TEXT("Placement before %.1f degrees should draw."), CornerAngle), Before.bShouldDraw);
		TestTrue(FString::Printf(TEXT("Placement after %.1f degrees should draw."), CornerAngle), After.bShouldDraw);

		const float StepDistance = FVector2D::Distance(Before.Position, After.Position);
		TestTrue(
			FString::Printf(TEXT("WindowEdge placement should move smoothly through %.1f degree corner. Step=%.2f"), CornerAngle, StepDistance),
			StepDistance <= MaxAllowedStep);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyDamageIndicatorWindowEdgeCornerArcTest,
	"HelsincyCrosshair.DamageIndicator.Placement.WindowEdgeCornerArc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyDamageIndicatorWindowEdgeCornerArcTest::RunTest(const FString& Parameters)
{
	const FVector2D Center(640.0f, 360.0f);
	const FVector2D SafeMin(72.0f, 72.0f);
	const FVector2D SafeMax(1208.0f, 648.0f);
	const FVector2D Corners[] = {
		FVector2D(SafeMax.X, SafeMin.Y),
		FVector2D(SafeMax.X, SafeMax.Y),
		FVector2D(SafeMin.X, SafeMax.Y),
		FVector2D(SafeMin.X, SafeMin.Y)
	};
	constexpr float MinInsetFromHardCorner = 2.0f;

	for (const FVector2D& Corner : Corners)
	{
		const float Angle = ScreenDeltaToIndicatorAngle(Corner - Center);
		const FHelsincy_ResolvedDamageIndicatorPlacement Placement = ResolveWindowEdgeTestPlacement(Angle);

		TestTrue(FString::Printf(TEXT("Placement at hard corner angle %.1f should draw."), Angle), Placement.bShouldDraw);

		const float InsetX = FMath::Min(FMath::Abs(Placement.Position.X - SafeMin.X), FMath::Abs(Placement.Position.X - SafeMax.X));
		const float InsetY = FMath::Min(FMath::Abs(Placement.Position.Y - SafeMin.Y), FMath::Abs(Placement.Position.Y - SafeMax.Y));
		TestTrue(
			FString::Printf(TEXT("WindowEdge placement at %.1f degrees should stay on the rounded arc. Inset=(%.2f, %.2f)"), Angle, InsetX, InsetY),
			InsetX > MinInsetFromHardCorner && InsetY > MinInsetFromHardCorner);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
