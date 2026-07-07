#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Engine/Texture2D.h"
#include "Library/HelsincySingleHitMarkerRenderCore.h"
#include "Library/HelsincySingleHitMarkerSpriteSupport.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteFallbackWhenCoreResourceMissingTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteFallsBackWhenCoreResourceMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteFallbackWhenCoreResourceMissingTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.SingleInstanceCoreTexture = NewObject<UTexture2D>();

	const HelsincySingleHitMarkerSpriteSupport::FResolvedSingleHitMarkerSpriteAssets ResolvedAssets =
		HelsincySingleHitMarkerSpriteSupport::ResolveSpriteAssets(Config);

	TestTrue(
		TEXT("SpriteDualLayer should fall back to geometry when the core texture has no render resource yet."),
		ResolvedAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::LegacyGeometry);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteCenterUsesSingleStableShakeSourceTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteCenterUsesSingleStableShakeSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteCenterUsesSingleStableShakeSourceTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.SingleInstanceSpriteMotionMode = EHelsincySingleHitMarkerSpriteMotionMode::WholeSpriteShake;
	Config.bUseImpactMotion = false;
	Config.NormalShakeIntensity = 8.0f;
	Config.ShakeIntensity = 8.0f;
	Config.ShakeFrequency = 34.0f;
	Config.ShakeDamping = 0.0f;

	FHelsincy_SingleHitMarkerState MarkerState;
	MarkerState.bActive = true;
	MarkerState.bVisible = true;
	MarkerState.TimeRemaining = 0.2f;
	MarkerState.TotalDuration = 0.25f;
	MarkerState.Opacity = 1.0f;
	MarkerState.ShakeEnergy = 1.0f;
	MarkerState.ShakeAge = 0.05f;
	MarkerState.ShakePhase = 0.0f;
	MarkerState.ShakeDirection = FVector2D(1.0f, 0.0f);

	HelsincySingleHitMarkerRenderCore::FSharedHitMarkerRenderState State;
	HelsincySingleHitMarkerRenderCore::CalculateSingleHitMarkerState(
		Config,
		MarkerState,
		FVector2D(100.0f, 100.0f),
		1.0f,
		State);

	const FVector2D UnshakenCenter = State.BasePosition + State.CalculatedGlobalOffset;
	const FVector2D SpriteCenter =
		HelsincySingleHitMarkerRenderCore::CalculateSingleHitMarkerSpriteCenter(State);

	TestFalse(
		TEXT("The shared hitmarker state should still provide one directional shake source for sprite motion."),
		UnshakenCenter.Equals(State.BasePosition, KINDA_SMALL_NUMBER));
	TestTrue(
		TEXT("Whole-sprite motion should temper the procedural shake amplitude for thin sprite artwork."),
		State.CalculatedGlobalOffset.Size() <= Config.ShakeIntensity * 0.36f);
	TestTrue(
		TEXT("SpriteDualLayer should not add a second high-frequency sprite-only offset on top of the shared shake."),
		SpriteCenter.Equals(UnshakenCenter, KINDA_SMALL_NUMBER));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteNormalShakeIsTemperedTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteNormalShakeIsTempered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteNormalShakeIsTemperedTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.SingleInstanceSpriteMotionMode = EHelsincySingleHitMarkerSpriteMotionMode::PerArmQuadrantShake;
	Config.bUseImpactMotion = true;
	Config.NormalShakeIntensity = 10.0f;

	FHelsincy_SingleHitMarkerState MarkerState;
	MarkerState.bActive = true;
	MarkerState.bVisible = true;
	MarkerState.Opacity = 1.0f;
	MarkerState.ShakeEnergy = 1.0f;
	MarkerState.AccentStrength = 1.0f;

	HelsincySingleHitMarkerRenderCore::FSharedHitMarkerRenderState State;
	HelsincySingleHitMarkerRenderCore::CalculateSingleHitMarkerState(
		Config,
		MarkerState,
		FVector2D(100.0f, 100.0f),
		1.0f,
		State);

	TestTrue(
		TEXT("SpriteDualLayer should temper per-arm normal shake so thin sprite arms do not shimmer frame-to-frame."),
		State.CurrentNormalShakeIntensity < Config.NormalShakeIntensity);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteMotionModeDefaultsToWholeSpriteTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteMotionModeDefaultsToWholeSprite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteMotionModeDefaultsToWholeSpriteTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;

	TestTrue(
		TEXT("SpriteDualLayer should default to whole-sprite motion because the bundled core/glow textures are complete X sprites."),
		Config.SingleInstanceSpriteMotionMode == EHelsincySingleHitMarkerSpriteMotionMode::WholeSpriteShake);

	Config.SingleInstanceSpriteMotionMode = EHelsincySingleHitMarkerSpriteMotionMode::PerArmQuadrantShake;
	TestTrue(
		TEXT("SpriteDualLayer should also expose the per-arm normal shake algorithm."),
		Config.SingleInstanceSpriteMotionMode == EHelsincySingleHitMarkerSpriteMotionMode::PerArmQuadrantShake);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerWholeSpriteUsesOneCompleteTextureTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.WholeSpriteUsesOneCompleteTexture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerWholeSpriteUsesOneCompleteTextureTest::RunTest(const FString& Parameters)
{
	HelsincySingleHitMarkerRenderCore::FSharedHitMarkerRenderState State;
	State.BasePosition = FVector2D(100.0f, 100.0f);
	State.CalculatedGlobalOffset = FVector2D(2.0f, -3.0f);
	State.InnerOffset = 8.0f;
	State.Length = 40.0f;
	State.CurrentNormalShakeIntensity = 10.0f;
	State.NormalShakeValues[0] = 1.0f;
	State.NormalShakeValues[1] = -1.0f;
	State.NormalShakeValues[2] = 0.5f;
	State.NormalShakeValues[3] = -0.5f;

	const HelsincySingleHitMarkerRenderCore::FSingleHitMarkerSpriteDrawSpec SpriteSpec =
		HelsincySingleHitMarkerRenderCore::CalculateSingleHitMarkerWholeSpriteSpec(State, 1.0f);

	TestTrue(
		TEXT("Whole-sprite motion should draw exactly one full texture centered on the shared sprite center."),
		SpriteSpec.Center.Equals(FVector2D(102.0f, 97.0f), KINDA_SMALL_NUMBER));
	TestTrue(
		TEXT("Whole-sprite motion should size one complete X texture from the hitmarker envelope."),
		SpriteSpec.Size.Equals(FVector2D(96.0f, 96.0f), KINDA_SMALL_NUMBER));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerPerArmUsesFourIndependentArmSpritesTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.PerArmUsesFourIndependentArmSprites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerPerArmUsesFourIndependentArmSpritesTest::RunTest(const FString& Parameters)
{
	HelsincySingleHitMarkerRenderCore::FSharedHitMarkerRenderState State;
	State.BasePosition = FVector2D(100.0f, 100.0f);
	State.CalculatedGlobalOffset = FVector2D::ZeroVector;
	State.InnerOffset = 8.0f;
	State.Length = 40.0f;
	State.CurrentNormalShakeIntensity = 10.0f;
	State.NormalShakeValues[0] = 1.0f;
	State.NormalShakeValues[1] = -1.0f;
	State.NormalShakeValues[2] = 0.5f;
	State.NormalShakeValues[3] = -0.5f;

	TArray<HelsincySingleHitMarkerRenderCore::FSingleHitMarkerSpriteDrawSpec> ArmSpecs;
	HelsincySingleHitMarkerRenderCore::CalculateSingleHitMarkerPerArmSpriteSpecs(
		State,
		FVector2D(128.0f, 32.0f),
		1.0f,
		ArmSpecs);

	TestEqual(TEXT("Per-arm mode should draw four arm sprites."), ArmSpecs.Num(), 4);
	TestTrue(
		TEXT("Per-arm mode should preserve the single-arm texture aspect ratio."),
		ArmSpecs[0].Size.Equals(FVector2D(40.0f, 10.0f), KINDA_SMALL_NUMBER));
	TestFalse(
		TEXT("Per-arm normal shake should move each arm independently instead of moving one complete X texture."),
		ArmSpecs[0].Center.Equals(ArmSpecs[1].Center, KINDA_SMALL_NUMBER));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerPerArmDoesNotUseWholeSpriteTexturesTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.PerArmDoesNotUseWholeSpriteTextures",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerPerArmDoesNotUseWholeSpriteTexturesTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.SingleInstanceSpriteMotionMode = EHelsincySingleHitMarkerSpriteMotionMode::PerArmQuadrantShake;
	Config.SingleInstanceCoreTexture = NewObject<UTexture2D>();
	Config.SingleInstanceGlowTexture = NewObject<UTexture2D>();
	Config.SingleInstanceArmTexture = nullptr;
	Config.SingleInstanceArmGlowTexture = nullptr;

	const HelsincySingleHitMarkerSpriteSupport::FResolvedSingleHitMarkerSpriteAssets ResolvedAssets =
		HelsincySingleHitMarkerSpriteSupport::ResolveSpriteAssets(Config);

	TestTrue(
		TEXT("Per-arm sprite mode must not reuse full-X core/glow textures as arm textures; missing arm art falls back safely."),
		ResolvedAssets.Mode == HelsincySingleHitMarkerSpriteSupport::EResolvedSingleHitMarkerSpriteMode::LegacyGeometry);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteFallbackIgnoresLegacyCustomTextureTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteFallbackIgnoresLegacyCustomTexture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteFallbackIgnoresLegacyCustomTextureTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.bEnabled = true;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.CustomTexture = NewObject<UTexture2D>();

	TestFalse(
		TEXT("SpriteDualLayer fallback must not use the legacy CustomTexture path because it draws the same image once per arm."),
		HelsincySingleHitMarkerRenderCore::ShouldAllowCustomTextureFallbackForSingleInstance(Config));

	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::LegacyGeometry;
	TestTrue(
		TEXT("LegacyGeometry keeps the existing custom texture fallback behavior."),
		HelsincySingleHitMarkerRenderCore::ShouldAllowCustomTextureFallbackForSingleInstance(Config));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteUsesCanvasYawRotationTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteUsesCanvasYawRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteUsesCanvasYawRotationTest::RunTest(const FString& Parameters)
{
	const FRotator Rotation = HelsincySingleHitMarkerRenderCore::MakeCanvasTileRotation(12.0f);

	TestTrue(
		TEXT("Canvas tile sprites should rotate around the screen-space Z axis via Yaw."),
		FMath::IsNearlyEqual(Rotation.Yaw, 12.0f));
	TestTrue(
		TEXT("Canvas tile sprites must not use Roll, which tilts the 2D quad out of the canvas plane."),
		FMath::IsNearlyZero(Rotation.Roll));
	TestTrue(
		TEXT("Canvas tile sprites must keep Pitch at zero."),
		FMath::IsNearlyZero(Rotation.Pitch));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteImpactDecayIsNotClampedToGeometryMinimumTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteImpactDecayIsNotClampedToGeometryMinimum",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteImpactDecayIsNotClampedToGeometryMinimumTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.Mode = EHitMarkerMode::SingleInstance;
	Config.SingleInstanceImpactDecaySpeed = 8.0f;

	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::LegacyGeometry;
	TestTrue(
		TEXT("LegacyGeometry keeps the snappy minimum impact decay."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveImpactMotionDecaySpeed(Config), 18.0f));

	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	TestTrue(
		TEXT("SpriteDualLayer should preserve impact motion longer than the geometry minimum."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveImpactMotionDecaySpeed(Config), 8.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteDisplayDurationUsesPerceptibleMinimumTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteDisplayDurationUsesPerceptibleMinimum",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteDisplayDurationUsesPerceptibleMinimumTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.Duration = 0.10f;
	Config.SingleInstanceSpriteMinDisplayDuration = 0.35f;

	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::LegacyGeometry;
	TestTrue(
		TEXT("LegacyGeometry should keep the shared hitmarker duration unchanged."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveDisplayDuration(Config), 0.10f));

	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	TestTrue(
		TEXT("SpriteDualLayer should enforce its data-driven minimum display duration to avoid flashing."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveDisplayDuration(Config), 0.35f));

	Config.Duration = 0.50f;
	TestTrue(
		TEXT("SpriteDualLayer should honor a shared duration that is already longer than the sprite minimum."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveDisplayDuration(Config), 0.50f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincySingleHitMarkerSpriteDisplayDurationFallsBackWhenSavedMinimumIsInvalidTest,
	"HelsincyCrosshair.HitMarker.SingleInstance.SpriteDisplayDurationFallsBackWhenSavedMinimumIsInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincySingleHitMarkerSpriteDisplayDurationFallsBackWhenSavedMinimumIsInvalidTest::RunTest(const FString& Parameters)
{
	FHelsincy_HitMarkerProfile Config;
	Config.Duration = 0.10f;
	Config.SingleInstanceRenderMode = EHelsincySingleHitMarkerRenderMode::SpriteDualLayer;
	Config.SingleInstanceSpriteMinDisplayDuration = 0.0f;

	TestTrue(
		TEXT("SpriteDualLayer should fall back to a perceptible default when an old saved asset has a zero sprite minimum."),
		FMath::IsNearlyEqual(FHelsincy_SingleHitMarkerState::ResolveDisplayDuration(Config), 0.35f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
