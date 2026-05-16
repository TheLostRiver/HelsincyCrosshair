#pragma once

#include "CoreMinimal.h"
#include "DataTypes/HelsincyCrosshairTypes.h"

namespace HelsincyHitMarkerShakeMath
{
	constexpr float TwoPi = 6.28318530718f;

	struct FResolvedImpactMotion
	{
		FVector2D TranslationOffset = FVector2D::ZeroVector;
		float RotationDegrees = 0.0f;
		float ScaleMultiplier = 1.0f;
		float ArmLengthMultiplier = 1.0f;
		float MotionAlpha = 0.0f;
	};

	inline uint32 HashSeed(int32 Seed, int32 Salt = 0)
	{
		uint32 X = static_cast<uint32>(Seed) ^ (static_cast<uint32>(Salt) * 0x9E3779B9u);
		X ^= X >> 16;
		X *= 0x7FEB352Du;
		X ^= X >> 15;
		X *= 0x846CA68Bu;
		X ^= X >> 16;
		return X;
	}

	inline float HashUnit(int32 Seed, int32 Salt = 0)
	{
		return static_cast<float>(HashSeed(Seed, Salt) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
	}

	inline float SignFromSeed(int32 Seed, int32 Salt = 0)
	{
		return HashUnit(Seed, Salt) < 0.5f ? -1.0f : 1.0f;
	}

	inline float PhaseFromSeed(int32 Seed, int32 Salt = 0)
	{
		return HashUnit(Seed, Salt) * TwoPi;
	}

	inline FVector2D DirectionFromSeed(int32 Seed, int32 Salt = 0)
	{
		const float Angle = PhaseFromSeed(Seed, Salt);
		return FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
	}

	inline FVector2D RotateDirection(const FVector2D& Direction, float Degrees)
	{
		const float Radians = FMath::DegreesToRadians(Degrees);
		const float C = FMath::Cos(Radians);
		const float S = FMath::Sin(Radians);
		return FVector2D(
			Direction.X * C - Direction.Y * S,
			Direction.X * S + Direction.Y * C
		);
	}

	inline float GetPriorityShakeMultiplier(const FHelsincy_HitMarkerProfile& Config, EHitMarkerPriority Priority)
	{
		switch (Priority)
		{
		case EHitMarkerPriority::High_Priority_Kill:
			return FMath::Max(Config.KillShakeMultiplier, 0.0f);
		case EHitMarkerPriority::Medium_Priority_Head:
			return FMath::Max(Config.HeadshotShakeMultiplier, 0.0f);
		default:
			return 1.0f;
		}
	}

	inline float ResolveDampedShakeScalar(float Age, float Frequency, float Damping, float Phase)
	{
		const float SafeAge = FMath::Max(Age, 0.0f);
		const float Envelope = FMath::Exp(-FMath::Max(Damping, 0.0f) * SafeAge);
		return FMath::Sin((FMath::Max(Frequency, 0.0f) * SafeAge) + Phase) * Envelope;
	}

	inline FVector2D ResolveDampedShakeOffset(const FVector2D& Direction, float Age, float Amplitude, float Frequency, float Damping, float Phase)
	{
		if (FMath::Abs(Amplitude) <= KINDA_SMALL_NUMBER)
		{
			return FVector2D::ZeroVector;
		}

		FVector2D SafeDirection = Direction;
		if (SafeDirection.IsNearlyZero())
		{
			SafeDirection = FVector2D(1.0f, 0.0f);
		}
		else
		{
			SafeDirection.Normalize();
		}

		return SafeDirection * (Amplitude * ResolveDampedShakeScalar(Age, Frequency, Damping, Phase));
	}

	inline float ResolveTwistCounterTwistCurve(float NormalizedAge)
	{
		const float T = FMath::Clamp(NormalizedAge, 0.0f, 1.0f);
		if (T < 0.16f)
		{
			return FMath::InterpEaseOut(0.0f, 1.0f, T / 0.16f, 2.0f);
		}
		if (T < 0.42f)
		{
			return FMath::Lerp(1.0f, -0.55f, (T - 0.16f) / 0.26f);
		}
		if (T < 0.78f)
		{
			return FMath::Lerp(-0.55f, 0.12f, (T - 0.42f) / 0.36f);
		}
		return FMath::Lerp(0.12f, 0.0f, (T - 0.78f) / 0.22f);
	}

	inline float ResolveFastPunchCurve(float Age, float Duration)
	{
		const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
		const float SafeAge = FMath::Max(Age, 0.0f);
		const float Attack = 1.0f - FMath::Exp(-42.0f * SafeAge);
		const float Damping = FMath::Exp(-5.6f * (SafeAge / SafeDuration));
		return FMath::Clamp(Attack * Damping, 0.0f, 1.0f);
	}

	inline FResolvedImpactMotion ResolveImpactMotion(
		const FHelsincy_HitMarkerProfile& Config,
		const FVector2D& Direction,
		float Age,
		float Energy,
		float DamageScale,
		float MotionSign,
		float Phase,
		float Scale)
	{
		FResolvedImpactMotion Result;
		const float SafeEnergy = FMath::Clamp(Energy, 0.0f, FMath::Max(Config.MaxImpactMotionEnergy, 0.25f));
		const float SafeDamage = FMath::Clamp(DamageScale, 0.0f, 1.0f);
		const float DamageBoost = 1.0f + (SafeDamage * FMath::Clamp(Config.DamageToImpactScale, 0.0f, 1.0f));
		const float EffectiveEnergy = SafeEnergy * DamageBoost;
		const float SafeDuration = FMath::Max(Config.ImpactMotionDuration, KINDA_SMALL_NUMBER);
		const float NormalizedAge = FMath::Clamp(Age / SafeDuration, 0.0f, 1.0f);
		const float Twist = Config.bUseImpactMotion ? ResolveTwistCounterTwistCurve(NormalizedAge) : 0.0f;
		const float Punch = Config.bUseImpactMotion ? ResolveFastPunchCurve(Age, SafeDuration) : 0.0f;

		Result.MotionAlpha = FMath::Max(FMath::Abs(Twist), Punch);
		Result.RotationDegrees = MotionSign * Config.ImpactRotationDegrees * EffectiveEnergy * Twist;
		Result.ScaleMultiplier = 1.0f + (Config.ImpactScalePunch * EffectiveEnergy * Punch);
		Result.ArmLengthMultiplier = 1.0f + (Config.ImpactArmLengthPunch * EffectiveEnergy * Punch);
		Result.TranslationOffset = ResolveDampedShakeOffset(
			Direction,
			Age,
			Config.ShakeIntensity * Scale * EffectiveEnergy * Config.ImpactTranslationWeight,
			Config.ShakeFrequency,
			Config.ShakeDamping,
			Phase);
		return Result;
	}

	inline void ResolveNormalShakeValues(int32 Seed, float Age, float Frequency, float Damping, float OutValues[4])
	{
		for (int32 i = 0; i < 4; ++i)
		{
			const float ArmFrequency = FMath::Max(Frequency, 0.0f) * (0.92f + HashUnit(Seed, 20 + i) * 0.16f);
			OutValues[i] = ResolveDampedShakeScalar(Age, ArmFrequency, Damping, PhaseFromSeed(Seed, 10 + i));
		}
	}
}
