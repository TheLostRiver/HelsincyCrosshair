#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Library/HelsincySingleHitMarkerRenderCore.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyHitMarkerCrosshairVisibilityDrawProfileTest,
	"HelsincyCrosshair.HitMarker.CrosshairVisibility.ScaleAlphaUsesVisualOpacity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyHitMarkerCrosshairVisibilityDrawProfileTest::RunTest(const FString& Parameters)
{
	FHelsincyCrosshairProfile Profile;
	Profile.VisualsConfig.Opacity = 0.8f;
	Profile.HitMarkerConfig.bEnabled = true;
	Profile.HitMarkerConfig.CrosshairVisibilityWhileActive = EHelsincyHitMarkerCrosshairVisibilityPolicy::ScaleAlpha;
	Profile.HitMarkerConfig.CrosshairAlphaScaleWhileHitMarkerActive = 0.25f;

	const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult Visibility =
		HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(
			Profile.HitMarkerConfig,
			true);

	const FHelsincyCrosshairProfile DrawProfile =
		HelsincySingleHitMarkerRenderCore::MakeBaseCrosshairDrawProfile(Profile, Visibility);

	TestTrue(TEXT("ScaleAlpha should keep the base crosshair drawable."), Visibility.ShouldDrawBaseCrosshair());
	TestTrue(
		TEXT("ScaleAlpha should be applied through VisualsConfig.Opacity for geometric renderers."),
		FMath::IsNearlyEqual(DrawProfile.VisualsConfig.Opacity, 0.2f));
	TestTrue(
		TEXT("MakeBaseCrosshairDrawProfile should not mutate the source profile."),
		FMath::IsNearlyEqual(Profile.VisualsConfig.Opacity, 0.8f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyHitMarkerCrosshairVisibilityLegacyAlphaTest,
	"HelsincyCrosshair.HitMarker.CrosshairVisibility.LegacySingleInstanceFieldsMapToRuntimeVisibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyHitMarkerCrosshairVisibilityLegacyAlphaTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.CrosshairVisibilityWhileActive = EHelsincyHitMarkerCrosshairVisibilityPolicy::Hide;
	Config.CrosshairAlphaScaleWhileHitMarkerActive = 0.10f;
	Config.bApplyHitMarkerVisibilityPolicyToCenterDot = true;
	Config.bUseSingleInstanceVisualSeparation = true;
	Config.SingleInstanceCrosshairAlphaScale = 0.45f;
	Config.bHideCenterDotWhenSingleInstanceActive = false;
	Config.SingleInstanceCenterDotAlphaScale = 0.35f;

	const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult Visibility =
		HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(Config, true);

	TestTrue(TEXT("Edited legacy crosshair alpha field should keep the base crosshair drawable."), Visibility.ShouldDrawBaseCrosshair());
	TestTrue(
		TEXT("Edited legacy crosshair alpha field should map to runtime base alpha scale."),
		FMath::IsNearlyEqual(Visibility.BaseCrosshairAlphaScale, 0.45f));
	TestTrue(TEXT("Edited legacy center-dot hide field should keep the center dot drawable."), Visibility.ShouldDrawCenterDot());
	TestTrue(
		TEXT("Edited legacy center-dot alpha field should map to runtime center-dot alpha scale."),
		FMath::IsNearlyEqual(Visibility.CenterDotAlphaScale, 0.35f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyHitMarkerCrosshairVisibilityLegacySeparationDisabledTest,
	"HelsincyCrosshair.HitMarker.CrosshairVisibility.LegacySingleInstanceSeparationDisabledKeepsVisible",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyHitMarkerCrosshairVisibilityLegacySeparationDisabledTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.CrosshairVisibilityWhileActive = EHelsincyHitMarkerCrosshairVisibilityPolicy::Hide;
	Config.CrosshairAlphaScaleWhileHitMarkerActive = 0.10f;
	Config.bApplyHitMarkerVisibilityPolicyToCenterDot = true;
	Config.bUseSingleInstanceVisualSeparation = false;

	const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult Visibility =
		HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(Config, true);

	TestTrue(TEXT("Disabling legacy visual separation should keep the base crosshair visible when new fields are untouched."), Visibility.ShouldDrawBaseCrosshair());
	TestTrue(TEXT("Disabling legacy visual separation should keep the center dot visible when new fields are untouched."), Visibility.ShouldDrawCenterDot());
	TestTrue(
		TEXT("Disabling legacy visual separation should leave base crosshair alpha unchanged."),
		FMath::IsNearlyEqual(Visibility.BaseCrosshairAlphaScale, 1.0f));
	TestTrue(
		TEXT("Disabling legacy visual separation should leave center-dot alpha unchanged."),
		FMath::IsNearlyEqual(Visibility.CenterDotAlphaScale, 1.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyHitMarkerCrosshairVisibilityNewPolicyWinsTest,
	"HelsincyCrosshair.HitMarker.CrosshairVisibility.NewPolicyWinsOverLegacySingleInstanceFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyHitMarkerCrosshairVisibilityNewPolicyWinsTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.CrosshairVisibilityWhileActive = EHelsincyHitMarkerCrosshairVisibilityPolicy::ScaleAlpha;
	Config.CrosshairAlphaScaleWhileHitMarkerActive = 0.25f;
	Config.bApplyHitMarkerVisibilityPolicyToCenterDot = true;
	Config.bUseSingleInstanceVisualSeparation = true;
	Config.SingleInstanceCrosshairAlphaScale = 0.75f;
	Config.bHideCenterDotWhenSingleInstanceActive = false;
	Config.SingleInstanceCenterDotAlphaScale = 0.60f;

	const HelsincySingleHitMarkerRenderCore::FHitMarkerCrosshairVisibilityResult Visibility =
		HelsincySingleHitMarkerRenderCore::ResolveCrosshairVisibilityWhileHitMarkerActive(Config, true);

	TestTrue(
		TEXT("Explicit new ScaleAlpha policy should win over edited legacy base alpha field."),
		FMath::IsNearlyEqual(Visibility.BaseCrosshairAlphaScale, 0.25f));
	TestTrue(
		TEXT("Explicit new ScaleAlpha policy should win over edited legacy center-dot alpha field."),
		FMath::IsNearlyEqual(Visibility.CenterDotAlphaScale, 0.25f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
