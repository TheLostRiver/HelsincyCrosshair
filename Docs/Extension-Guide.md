# HelsincyCrosshair 扩展开发指南

## 1. 扩展思路总览

HelsincyCrosshair 的扩展重点不在修改 HUD，而在补充三类能力：

1. 新的准心形状渲染器
2. 新的伤害指示器渲染器
3. 新的目标识别输入来源

如果你能保持这三层边界，插件会比直接修改主绘制逻辑更容易维护。

## 2. 自定义准心渲染器

### 2.1 C++ 扩展方式

自定义准心渲染器需要：

1. 继承 UHelsincyShapeRenderer。
2. 在构造函数或默认对象中设置 AssociatedTag。
3. 重写 Draw。
4. 让 UHelsincyCrosshairManagerSubsystem 注册该类。

基础骨架示例：

```cpp
UCLASS()
class UMyCrosshairRenderer : public UHelsincyShapeRenderer
{
    GENERATED_BODY()

public:
    UMyCrosshairRenderer()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Crosshair.Shape.MyShape"));
    }

    virtual void Draw(
        UCanvas* Canvas,
        const FHelsincyCrosshairProfile& Profile,
        FVector2D Spread,
        FVector2D Center,
        FLinearColor CurrentColor,
        float DeltaTime,
        float Scale) override;
};
```

### 2.2 蓝图扩展方式

UHelsincyShapeRenderer 提供 BlueprintImplementableEvent：

- ReceiveDraw

理论上你可以：

1. 创建一个继承自 UHelsincyShapeRenderer 的蓝图类。
2. 配置 AssociatedTag。
3. 在 ReceiveDraw 中完成绘制。
4. 将该类加入 UHelsincyCrosshairSettings.BlueprintRenderers。

不过要注意，蓝图绘制路径天然比纯 C++ 方案更偏向灵活性，而不是极致性能。

### 2.3 注册约束

Subsystem 注册时会检查：

- Tag 是否有效
- Tag 是否属于 Crosshair.Shape 子树
- 是否重复注册

因此不要复用内置形状 Tag 去覆盖现有渲染器，除非你明确修改注册策略。

## 3. 自定义伤害方向指示器

### 3.1 C++ 扩展方式

继承 UHelsincyIndicatorRenderer，并重写 DrawPointer。

基础骨架：

```cpp
UCLASS()
class UMyIndicatorRenderer : public UHelsincyIndicatorRenderer
{
    GENERATED_BODY()

public:
    UMyIndicatorRenderer()
    {
        AssociatedTag = FGameplayTag::RequestGameplayTag(FName("Indicator.Style.MyPointer"));
    }

    virtual void DrawPointer(
        UCanvas* Canvas,
        const FHelsincy_DamageIndicatorProfile& Profile,
        FVector2D Center,
        float Angle,
        float Alpha,
        float Scale) override;
};
```

然后调用 RegisterDamageIndicatorRenderer 完成注册。

### 3.2 蓝图扩展方式

UHelsincyIndicatorRenderer 提供 BlueprintImplementableEvent：

- ReceiveDrawPointer

按设计，它支持通过 `UHelsincyDamageIndicatorSettings.BlueprintIndicatorRenderers` 自动加载注册。

在 **Project Settings → HelsincyDamageIndicatorSystem** 中将蓝图类添加到 Blueprint Renderers 列表即可完成注册。

## 4. 自定义目标识别

### 4.1 阵营识别扩展

如果你的项目已经有阵营系统，最稳妥的方式是让目标实现 GenericTeamAgentInterface。插件会直接读取敌友态度，无需再造一层自定义接口。

### 4.2 交互对象识别扩展

如果你的目标是可拾取物、终端、门、机关等，不适合用阵营表示，则实现 IHelsincyTargetInterface：

```cpp
virtual FGameplayTag GetCrosshairInteractionTag_Implementation() const override;
```

返回值示例：

- Target.Loot
- Target.Loot.Gold
- Target.Interactable
- Target.Terminal

组件会在 InteractionColorMap 中先精确查找，再沿父 Tag 向上回溯匹配。

## 5. 自定义配置资产流

如果你希望做更复杂的准心管理，推荐扩展点不是去修改 CurrentProfile 字段，而是围绕 DataAsset 做包装：

1. 每种武器维护独立 DataAsset。
2. 每种瞄准状态维护独立 DataAsset。
3. 切换状态时统一调用 LoadFromDataAsset。

好处是：

- 调参交给策划更方便
- 运行时切换逻辑更清晰
- 更适合和武器系统对接

## 6. 扩展时的设计建议

### 6.1 不要直接改 HUD 解决一切问题

HUD 只应该做调度和组合绘制，不应该承载大量武器逻辑、状态逻辑或业务判断。

### 6.2 不要把动态逻辑写进 DataAsset

DataAsset 适合作为模板配置，不适合作为状态容器。运行时变化应该由组件维护。

### 6.3 Tag 命名要成体系

建议遵循：

- Crosshair.Shape.ProjectName.ShapeName
- Indicator.Style.ProjectName.StyleName
- Target.Interactable.TypeName

这样后续做父级匹配和批量管理更容易。

## 7. 推荐扩展路线

如果你要把插件继续做成项目级能力，推荐按下面顺序扩展：

1. 修正蓝图伤害指示器注册问题。
2. ~~补齐 DotOnly 内置渲染器。~~ ✅ 已完成
3. 为武器系统增加统一准心切换适配层。
4. ~~增加 SaveGame 级预设持久化。~~ ✅ 已完成（SavePresetsToDisk / LoadPresetsFromDisk + 自动保存属性）
5. 为每个形状补一套示例 DataAsset。

## 8. 结论

HelsincyCrosshair 的扩展策略本质上是“Tag 选路 + 渲染器实现 + 组件供数”。只要保持这个边界，你可以很自然地把插件从一个基础准心模块演进成一套完整的射击 HUD 表现系统。