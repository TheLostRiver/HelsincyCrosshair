// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"
#include "HelsincyDamageIndicatorSaveGame.generated.h"

/**
 * 伤害指示器预设持久化存档
 * Damage indicator preset save data for persistence
 * 将预设库保存到磁盘，实现跨关卡/跨会话持久化
 * Saves preset library to disk for cross-level/cross-session persistence
 */
UCLASS()
class HELSINCYDAMAGEINDICATOR_API UHelsincyDamageIndicatorSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UHelsincyDamageIndicatorSaveGame();

	// 数据版本号 — 用于未来数据迁移 | Data version number — for future data migration
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 SaveVersion;

	// 预设库: 名称 → 完整配置 | Preset library: name → full configuration
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	TMap<FName, FHelsincy_DamageIndicatorProfile> SavedPresets;

	// 最后使用的预设名称 (恢复时自动应用) | Last used preset name (auto-applied on restore)
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FName LastActivePresetName;

	// 当前存档版本 | Current save version
	static constexpr int32 CurrentSaveVersion = 1;

	// 数据迁移 (Version < CurrentSaveVersion 时调用)
	// Data migration (called when Version < CurrentSaveVersion)
	void MigrateData();
};
