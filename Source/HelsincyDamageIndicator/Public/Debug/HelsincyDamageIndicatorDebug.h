// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace HelsincyDamageIndicatorDebug
{
#if !UE_BUILD_SHIPPING
	HELSINCYDAMAGEINDICATOR_API bool IsEnabled();
	HELSINCYDAMAGEINDICATOR_API bool IsTextEnabled();
	HELSINCYDAMAGEINDICATOR_API bool IsGeometryEnabled();
	HELSINCYDAMAGEINDICATOR_API bool IsVerboseLogEnabled();
#else
	FORCEINLINE bool IsEnabled()          { return false; }
	FORCEINLINE bool IsTextEnabled()      { return false; }
	FORCEINLINE bool IsGeometryEnabled()  { return false; }
	FORCEINLINE bool IsVerboseLogEnabled(){ return false; }
#endif
}
