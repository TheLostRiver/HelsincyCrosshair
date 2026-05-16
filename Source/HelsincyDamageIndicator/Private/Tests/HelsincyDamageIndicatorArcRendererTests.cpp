#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "IndicatorRenderer/HelsincyIndicatorRendererArc.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyDamageIndicatorArcDesiredSizeTest,
	"HelsincyCrosshair.DamageIndicator.Renderer.ArcDesiredSize",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyDamageIndicatorArcDesiredSizeTest::RunTest(const FString& Parameters)
{
	UHelsincyIndicatorRendererArc* Renderer = NewObject<UHelsincyIndicatorRendererArc>();
	TestNotNull(TEXT("Arc renderer should be constructible."), Renderer);

	FHelsincy_DamageIndicatorProfile Profile;
	Profile.ArcConfig.Size = FVector2D(220.0f, 80.0f);

	Profile.PlacementMode = EHelsincyDamageIndicatorPlacementMode::RadialCircle;
	const FVector2D RadialSize = Renderer->GetDesiredPointerSize(Profile, 1.0f);
	TestEqual(TEXT("RadialCircle should use configured arc width."), RadialSize.X, 220.0f);
	TestEqual(TEXT("RadialCircle should use configured arc height."), RadialSize.Y, 80.0f);

	Profile.PlacementMode = EHelsincyDamageIndicatorPlacementMode::WindowEdge;
	const FVector2D WindowEdgeSize = Renderer->GetDesiredPointerSize(Profile, 1.0f);
	TestEqual(TEXT("WindowEdge should reserve square rotated bounds width."), WindowEdgeSize.X, 220.0f * 1.41421356f);
	TestEqual(TEXT("WindowEdge should reserve square rotated bounds height."), WindowEdgeSize.Y, 220.0f * 1.41421356f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
