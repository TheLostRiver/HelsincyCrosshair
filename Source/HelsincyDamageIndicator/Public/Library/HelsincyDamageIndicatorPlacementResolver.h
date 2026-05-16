// Copyright , Helsincy Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DataTypes/HelsincyDamageIndicatorTypes.h"

struct FHelsincy_ResolvedDamageIndicatorPlacement
{
	FVector2D Origin = FVector2D::ZeroVector;
	FVector2D Position = FVector2D::ZeroVector;
	FVector2D Direction = FVector2D(0.0f, -1.0f);
	float RotationAngle = 0.0f;
	bool bShouldDraw = false;
};

namespace HelsincyDamageIndicatorPlacement
{
	FORCEINLINE FVector2D AngleToScreenDirection(const float Angle)
	{
		const float RadAngle = FMath::DegreesToRadians(Angle);
		FVector2D Direction(FMath::Sin(RadAngle), -FMath::Cos(RadAngle));
		if (Direction.SizeSquared() <= KINDA_SMALL_NUMBER)
		{
			Direction = FVector2D(0.0f, -1.0f);
		}
		else
		{
			Direction.Normalize();
		}
		return Direction;
	}

	FORCEINLINE float ResolveAxisIntersectionTime(const float Origin, const float Direction, const float MinValue, const float MaxValue)
	{
		if (Direction > KINDA_SMALL_NUMBER)
		{
			return (MaxValue - Origin) / Direction;
		}
		if (Direction < -KINDA_SMALL_NUMBER)
		{
			return (MinValue - Origin) / Direction;
		}
		return BIG_NUMBER;
	}

	FORCEINLINE FVector2D ClampToRect(const FVector2D& Value, const FVector2D& MinValue, const FVector2D& MaxValue)
	{
		return FVector2D(
			FMath::Clamp(Value.X, MinValue.X, MaxValue.X),
			FMath::Clamp(Value.Y, MinValue.Y, MaxValue.Y)
		);
	}

	FORCEINLINE bool ResolveRayCircleIntersectionTime(
		const FVector2D& Origin,
		const FVector2D& Direction,
		const FVector2D& CircleCenter,
		const float Radius,
		const bool bNearLeft,
		const bool bNearTop,
		float& OutTime)
	{
		const float A = Direction.SizeSquared();
		if (A <= KINDA_SMALL_NUMBER)
		{
			return false;
		}

		const FVector2D Delta = Origin - CircleCenter;
		const float B = 2.0f * FVector2D::DotProduct(Delta, Direction);
		const float C = Delta.SizeSquared() - Radius * Radius;
		const float Discriminant = B * B - 4.0f * A * C;
		if (Discriminant < 0.0f)
		{
			return false;
		}

		const float SqrtDiscriminant = FMath::Sqrt(Discriminant);
		const float InvDenominator = 1.0f / (2.0f * A);
		const float TimeA = (-B - SqrtDiscriminant) * InvDenominator;
		const float TimeB = (-B + SqrtDiscriminant) * InvDenominator;

		OutTime = BIG_NUMBER;
		const float Tolerance = 0.5f;
		const auto TryAcceptTime = [&](const float CandidateTime)
		{
			if (CandidateTime <= KINDA_SMALL_NUMBER)
			{
				return;
			}

			const FVector2D Candidate = Origin + Direction * CandidateTime;
			const bool bInCornerX = bNearLeft
				? Candidate.X <= CircleCenter.X + Tolerance
				: Candidate.X >= CircleCenter.X - Tolerance;
			const bool bInCornerY = bNearTop
				? Candidate.Y <= CircleCenter.Y + Tolerance
				: Candidate.Y >= CircleCenter.Y - Tolerance;

			if (bInCornerX && bInCornerY && CandidateTime < OutTime)
			{
				OutTime = CandidateTime;
			}
		};

		TryAcceptTime(TimeA);
		TryAcceptTime(TimeB);

		return OutTime < BIG_NUMBER;
	}

	FORCEINLINE FVector2D ApplyCornerPadding(
		const FVector2D& Position,
		const FVector2D& Origin,
		const FVector2D& Direction,
		const FVector2D& SafeMin,
		const FVector2D& SafeMax,
		const float CornerPadding)
	{
		const float SafeWidth = SafeMax.X - SafeMin.X;
		const float SafeHeight = SafeMax.Y - SafeMin.Y;
		const float EffectivePadding = FMath::Min(CornerPadding, FMath::Min(SafeWidth, SafeHeight) * 0.5f);
		if (EffectivePadding <= KINDA_SMALL_NUMBER)
		{
			return Position;
		}

		const bool bNearLeft = Position.X <= SafeMin.X + EffectivePadding;
		const bool bNearRight = Position.X >= SafeMax.X - EffectivePadding;
		const bool bNearTop = Position.Y <= SafeMin.Y + EffectivePadding;
		const bool bNearBottom = Position.Y >= SafeMax.Y - EffectivePadding;

		if ((bNearLeft || bNearRight) && (bNearTop || bNearBottom))
		{
			const FVector2D ArcCenter(
				bNearLeft ? SafeMin.X + EffectivePadding : SafeMax.X - EffectivePadding,
				bNearTop ? SafeMin.Y + EffectivePadding : SafeMax.Y - EffectivePadding);

			float ArcTime = 0.0f;
			if (ResolveRayCircleIntersectionTime(Origin, Direction, ArcCenter, EffectivePadding, bNearLeft, bNearTop, ArcTime))
			{
				const FVector2D ArcPosition = Origin + Direction * ArcTime;
				return ClampToRect(ArcPosition, SafeMin, SafeMax);
			}
		}

		return ClampToRect(Position, SafeMin, SafeMax);
	}

	FORCEINLINE FHelsincy_ResolvedDamageIndicatorPlacement ResolveDamageIndicatorPlacement(
		const FHelsincy_DamageIndicatorProfile& Profile,
		const FVector2D& CanvasSize,
		const FVector2D& Center,
		const float Angle,
		const float Scale,
		const FVector2D& PointerSize)
	{
		FHelsincy_ResolvedDamageIndicatorPlacement Result;
		Result.Origin = Center;
		Result.Direction = AngleToScreenDirection(Angle);
		Result.RotationAngle = Angle;

		const FVector2D HalfPointer = FVector2D(
			FMath::Max(0.0f, PointerSize.X) * 0.5f,
			FMath::Max(0.0f, PointerSize.Y) * 0.5f
		);

		if (Profile.PlacementMode == EHelsincyDamageIndicatorPlacementMode::RadialCircle)
		{
			const float PointerExtent = FMath::Max(HalfPointer.X, HalfPointer.Y);
			float Radius = FMath::Max(0.0f, Profile.Radius * Scale);
			const float Offset = PointerExtent + FMath::Max(0.0f, Profile.PointerDistanceOffset * Scale);
			Radius = Profile.bPointerOutsideCircle ? Radius + Offset : FMath::Max(0.0f, Radius - Offset);

			Result.Position = Center + Result.Direction * Radius;
			Result.bShouldDraw = true;
			return Result;
		}

		const float Margin = FMath::Max(0.0f, Profile.EdgeMargin * Scale);
		const FVector2D SafeMin = FVector2D(Margin, Margin) + HalfPointer;
		const FVector2D SafeMax = CanvasSize - FVector2D(Margin, Margin) - HalfPointer;

		if (SafeMax.X <= SafeMin.X || SafeMax.Y <= SafeMin.Y)
		{
			Result.bShouldDraw = false;
			return Result;
		}

		const FVector2D ClampedCenter = ClampToRect(Center, SafeMin, SafeMax);
		const float TimeX = ResolveAxisIntersectionTime(ClampedCenter.X, Result.Direction.X, SafeMin.X, SafeMax.X);
		const float TimeY = ResolveAxisIntersectionTime(ClampedCenter.Y, Result.Direction.Y, SafeMin.Y, SafeMax.Y);
		const float Time = FMath::Min(TimeX, TimeY);

		if (Time <= 0.0f || Time >= BIG_NUMBER)
		{
			Result.bShouldDraw = false;
			return Result;
		}

		FVector2D Position = ClampedCenter + Result.Direction * Time;
		Position = ClampToRect(Position, SafeMin, SafeMax);
		Position = ApplyCornerPadding(Position, ClampedCenter, Result.Direction, SafeMin, SafeMax, FMath::Max(0.0f, Profile.EdgeCornerPadding * Scale));

		Result.Position = Position;
		Result.bShouldDraw = true;
		return Result;
	}
}
