// Copyright , Helsincy Games. All Rights Reserved.

#include "SaveGame/HelsincyDamageIndicatorSaveGame.h"

UHelsincyDamageIndicatorSaveGame::UHelsincyDamageIndicatorSaveGame()
	: SaveVersion(CurrentSaveVersion)
{
}

void UHelsincyDamageIndicatorSaveGame::MigrateData()
{
	// 未来版本增量迁移逻辑
	// Future incremental migration logic
	// if (SaveVersion < 2) { ... migrate v1 → v2 ... }
	SaveVersion = CurrentSaveVersion;
}
