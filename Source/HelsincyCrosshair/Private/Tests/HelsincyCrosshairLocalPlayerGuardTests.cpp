#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Components/HelsincyCrosshairComponent.h"
#include "WorldCollision.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyCrosshairPendingControllerSurvivesLegacyTimeoutTest,
	"HelsincyCrosshair.LocalPlayerGuard.PendingControllerSurvivesLegacyTimeout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyCrosshairPendingControllerSurvivesLegacyTimeoutTest::RunTest(const FString& Parameters)
{
	UHelsincyCrosshairComponent* Component = NewObject<UHelsincyCrosshairComponent>();
	TestNotNull(TEXT("Crosshair component should be created."), Component);

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
	FHelsincyCrosshairLocalPlayerGuardReactivatesAfterNonLocalOwnerTest,
	"HelsincyCrosshair.LocalPlayerGuard.ReactivatesAfterNonLocalOwner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyCrosshairLocalPlayerGuardReactivatesAfterNonLocalOwnerTest::RunTest(const FString& Parameters)
{
	UHelsincyCrosshairComponent* Component = NewObject<UHelsincyCrosshairComponent>();
	TestNotNull(TEXT("Crosshair component should be created."), Component);

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHelsincyCrosshairAsyncTraceIgnoresStaleCallbackTest,
	"HelsincyCrosshair.TargetDetection.AsyncTraceIgnoresStaleCallback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHelsincyCrosshairAsyncTraceIgnoresStaleCallbackTest::RunTest(const FString& Parameters)
{
	UHelsincyCrosshairComponent* Component = NewObject<UHelsincyCrosshairComponent>();
	TestNotNull(TEXT("Crosshair component should be created."), Component);

	const FTraceHandle CurrentHandle(11, 1);
	const FTraceHandle StaleHandle(10, 1);
	FTraceDatum EmptyDatum;

	Component->Debug_SetAsyncTraceHandleForAutomation(CurrentHandle);
	Component->Debug_SetTargetAttitudeForAutomation(ETeamAttitude::Friendly);

	Component->Debug_InvokeTraceCompletedForAutomation(StaleHandle, EmptyDatum);

	TestTrue(TEXT("A stale callback should leave the active async trace handle intact."), Component->Debug_HasActiveAsyncTraceForAutomation());
	TestTrue(
		TEXT("A stale callback should not clear or overwrite the current target attitude."),
		Component->GetCurrentTargetAttitude() == ETeamAttitude::Friendly);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
