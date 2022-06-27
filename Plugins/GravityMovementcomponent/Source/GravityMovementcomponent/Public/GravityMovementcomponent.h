// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <GameFramework/CharacterMovementComponent.h>

#include "GravityMovementcomponent.generated.h"

UCLASS()
class GRAVITYMOVEMENTCOMPONENT_API UGravityMovementcomponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
public:

	UGravityMovementcomponent(const FObjectInitializer& ObjectInitializer);

	void TickComponentBase(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) ;

	virtual void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	virtual bool IsValidLandingSpot(
		const FVector& CapsuleLocation,
		const FHitResult& Hit
	) const override;

	virtual void PhysWalking(float deltaTime, int32 Iterations)override;

	virtual void PhysFalling(float deltaTime, int32 Iterations)override;

	virtual void CalcVelocity(
		float DeltaTime,
		float Friction,
		bool bFluid, 
		float BrakingDeceleration
	)override;

	virtual bool ShouldLimitAirControl(
		float DeltaTime, 
		const FVector& FallAcceleration
	) const override;

	virtual FVector GetFallingLateralAcceleration(
		float DeltaTime
	)override;

	virtual bool DoJump(bool bReplayingMoves)override;

	virtual void StartFalling(
		int32 Iterations, 
		float remainingTime, 
		float timeTick,
		const FVector& Delta, 
		const FVector& subLoc
	)override;

	virtual void TwoWallAdjust(
		FVector& Delta, 
		const FHitResult& Hit,
		const FVector& OldHitNormal
	) const override;

	virtual bool ShouldCheckForValidLandingSpot(
		float DeltaTime, 
		const FVector& Delta,
		const FHitResult& Hit
	) const override;

	virtual float BoostAirControl(
		float DeltaTime,
		float TickAirControl,
		const FVector& FallAcceleration
	)override;

	virtual void MoveAlongFloor(
		const FVector& InVelocity,
		float DeltaSeconds,
		FStepDownResult* OutStepDownResult
	)override;

	virtual FVector ComputeGroundMovementDelta(
		const FVector& Delta,
		const FHitResult& RampHit,
		const bool bHitFromLineTrace
	) const override;

	virtual FVector ConstrainInputAcceleration(
		const FVector& InputAcceleration
	) const override;

	virtual bool IsWalkable(const FHitResult& Hit) const override;

	virtual float SlideAlongSurface(
		const FVector& Delta, 
		float Time,
		const FVector& Normal, 
		FHitResult& Hit,
		bool bHandleImpact = false
	)override;

	virtual bool StepUp(
		const FVector& GravDir,
		const FVector& Delta, 
		const FHitResult& Hit, 
		FStepDownResult* OutStepDownResult = NULL
	)override;

	virtual void HandleImpact(
		const FHitResult& Hit,
		float TimeSlice = 0.f, 
		const FVector& MoveDelta = FVector::ZeroVector
	) override;

	virtual void FindFloor(
		const FVector& CapsuleLocation, 
		FFindFloorResult& OutFloorResult,
		bool bCanUseCachedLocation, 
		const FHitResult* DownwardSweepResult = NULL
	) const override;

	virtual void ComputeFloorDist(
		const FVector& CapsuleLocation, 
		float LineDistance, 
		float SweepDistance, 
		FFindFloorResult& OutFloorResult,
		float SweepRadius,
		const FHitResult* DownwardSweepResult = NULL
	) const override;

	virtual void MaintainHorizontalGroundVelocity()override;

	virtual bool IsWithinEdgeTolerance(
		const FVector& CapsuleLocation, 
		const FVector& TestImpactPoint,
		const float CapsuleRadius
	) const override;

	virtual void AdjustFloorHeight()override;

	virtual bool ComputePerchResult(
		const float TestRadius, 
		const FHitResult& InHit,
		const float InMaxFloorDist, 
		FFindFloorResult& OutPerchFloorResult
	) const override;

	virtual bool ShouldComputePerchResult(
		const FHitResult& InHit, 
		bool bCheckRadius = true
	) const override;

	virtual bool FloorSweepTest(
		struct FHitResult& OutHit,
		const FVector& Start,
		const FVector& End,
		ECollisionChannel TraceChannel,
		const struct FCollisionShape& CollisionShape,
		const struct FCollisionQueryParams& Params,
		const struct FCollisionResponseParams& ResponseParam
	) const override;

	FQuat CapsuleQuat = FQuat::Identity;

	FVector GravityOrigin = FVector::ZeroVector;

	FVector GravityDirection = FVector::ZeroVector;

	FVector PreStepUpLocation = FVector::ZeroVector;
};

