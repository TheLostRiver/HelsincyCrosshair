#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Components/HelsincyDamageIndicatorComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyDamageIndicatorPendingControllerSurvivesLegacyTimeoutTest,
	"HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyDamageIndicatorPendingControllerSurvivesLegacyTimeoutTest::RunTest(const FString& Parameters)
{
	UHelsincyDamageIndicatorComponent* Component = NewObject<UHelsincyDamageIndicatorComponent>();
	TestNotNull(TEXT("DamageIndicator component should be created."), Component);

	Component->Debug_SetPendingLocalPlayerGuardForAutomation(60);

	Component->Debug_TickPendingLocalPlayerGuardForAutomation();

	TestTrue(
		TEXT("A missing Controller is still recoverable after the old 60-frame threshold."),
		Component->IsWaitingForLocalPlayerController());
	TestTrue(
		TEXT("The retry frame count should keep advancing for diagnostics instead of becoming terminal."),
		Component->GetPendingLocalPlayerGuardFrameCount() > 60);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyDamageIndicatorLocalPlayerGuardReactivatesAfterNonLocalOwnerTest,
	"HelsincyCrosshair.DamageIndicator.LocalPlayerGuard.ReactivatesAfterNonLocalOwner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyDamageIndicatorLocalPlayerGuardReactivatesAfterNonLocalOwnerTest::RunTest(const FString& Parameters)
{
	UHelsincyDamageIndicatorComponent* Component = NewObject<UHelsincyDamageIndicatorComponent>();
	TestNotNull(TEXT("DamageIndicator component should be created."), Component);

	Component->Debug_ActivateForLocalPlayerGuardForAutomation();
	TestTrue(TEXT("The initial local-owner activation should mark the guard as passed."), Component->Debug_IsOwnerCheckPassedForAutomation());
	TestTrue(TEXT("The initial local-owner activation should activate the component."), Component->IsActive());
	TestTrue(TEXT("The initial local-owner activation should enable ticking."), Component->IsComponentTickEnabled());
	TestTrue(TEXT("The initial local-owner activation should run one-time initialization."), Component->Debug_IsLocalPlayerInitializationCompleteForAutomation());

	Component->Debug_DeactivateForLocalPlayerGuardForAutomation();
	TestFalse(TEXT("A non-local owner should clear the current owner-check pass state."), Component->Debug_IsOwnerCheckPassedForAutomation());
	TestFalse(TEXT("A non-local owner should deactivate the component."), Component->IsActive());
	TestFalse(TEXT("A non-local owner should disable ticking."), Component->IsComponentTickEnabled());

	Component->Debug_ActivateForLocalPlayerGuardForAutomation();
	TestTrue(TEXT("Returning to a local owner should mark the guard as passed again."), Component->Debug_IsOwnerCheckPassedForAutomation());
	TestTrue(TEXT("Returning to a local owner should reactivate the component."), Component->IsActive());
	TestTrue(TEXT("Returning to a local owner should re-enable ticking."), Component->IsComponentTickEnabled());
	TestTrue(TEXT("Returning to a local owner should preserve completed one-time initialization."), Component->Debug_IsLocalPlayerInitializationCompleteForAutomation());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
