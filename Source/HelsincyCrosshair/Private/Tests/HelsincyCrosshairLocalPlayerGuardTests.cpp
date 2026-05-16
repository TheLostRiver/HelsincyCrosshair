#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Components/HelsincyCrosshairComponent.h"

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

#endif // WITH_DEV_AUTOMATION_TESTS
