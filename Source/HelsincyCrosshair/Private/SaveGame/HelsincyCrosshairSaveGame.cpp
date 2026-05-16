// Copyright 2024-2025, Helsincy Games. All Rights Reserved.

#include "SaveGame/HelsincyCrosshairSaveGame.h"

UHelsincyCrosshairSaveGame::UHelsincyCrosshairSaveGame()
	: SaveVersion(CurrentSaveVersion)
	, LastActivePresetName(NAME_None)
{
}

void UHelsincyCrosshairSaveGame::MigrateData()
{
	// 递增式迁移: 逐版本升级，不跳过中间步骤
	// Incremental migration: upgrade version by version, don't skip intermediate steps
	while (SaveVersion < CurrentSaveVersion)
	{
		switch (SaveVersion)
		{
		case 0:
			// Version 0 → 1: 初始格式，无需转换 | Initial format, no conversion needed
			break;
		// 未来新增版本迁移 | Future version migrations:
		// case 1:
		//     /* Version 1 → 2 迁移逻辑 | Migration logic */
		//     break;
		default:
			break;
		}
		++SaveVersion;
	}
}
