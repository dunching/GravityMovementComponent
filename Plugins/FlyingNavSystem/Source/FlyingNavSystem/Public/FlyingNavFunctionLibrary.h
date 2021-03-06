// Copyright Ben Sutherland 2022. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlyingNavigationData.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "FlyingNavFunctionLibrary.generated.h"

#ifndef ENABLE_DRAW_DEBUG
#define ENABLE_DRAW_DEBUG  !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#endif

// Blueprint Type version of ENavigationQueryResult::Type
UENUM(BlueprintType)
enum class EPathfindingResult: uint8
{
	Invalid,
	Error,
	Fail,
	Success,
	RecastError,
	Null
};

UCLASS(BlueprintType)
class UCatmullRomSpline : public UObject
{
	GENERATED_BODY()

public:
	UCatmullRomSpline(): bValid(false) {}

	/*
	* Fills spline with Path Points
	* @param PathPoints: Array of points to use as the spline control points.
	* @returns: Whether the resultant spline is valid
	*/
	UFUNCTION(BlueprintCallable, Category = "Spline")
	bool GenerateSpline(const TArray<FVector>& PathPoints);
	
	/*
	* Samples spline at a given T value 
	* @param T: Parameter to evaluate. In the range [0,1], corresponding to PathPoints[1] -> PathPoints[n-2] (Second to Second Last).
	*/
	UFUNCTION(BlueprintPure, Category = "Spline")
	FVector SampleSplineByParameter(const float T) const;

	/*
	* Finds the spline parameter for a given distance along the spline (approximation)
	* @param Distance: <= 0 gives T=0, >= arclength gives T=1. In Unreal units.
	*/
	UFUNCTION(BlueprintPure, Category = "Spline")
	float FindParameterForDistance(float Distance) const;

	/*
	* Samples spline at a given distance value
	* @param Distance: <= 0 gives first valid point, >= arclength gives last valid point. In Unreal units.
	*/
	UFUNCTION(BlueprintPure, Category = "Spline")
	FVector SampleSplineByDistance(const float Distance) const
	{
		return SampleSplineByParameter(FindParameterForDistance(Distance));
	}
	
	/*
	* Samples spline at equidistant intervals
	* @param SampleLength: distance between samples along path in Unreal units.
	*/
	UFUNCTION(BlueprintPure, Category = "Spline")
	TArray<FVector> EquidistantSamples(const float SampleLength = 100.f) const;

	/*
	* Returns the approximate length of the curve (pre-generated by GenerateSpline).
	*/
	UFUNCTION(BlueprintPure, Category = "Spline")
	float GetArcLength() const
	{
		if (!ensureMsgf(bValid, TEXT("Curve is not valid.")))
		{
			return 0.f;
		}

		return DistanceLUT.Last();
	}
private:
	void FillLUT();
	
	TArray<FVector> PValues;
	TArray<float> TValues;
	TArray<float> DistanceLUT;
	float MaxT;
	float TScale;
	float LUTParameterScale;
	float PathSegmentsLength;

	bool bValid;
};

/**
 * Allows rebuilding flying nav data from Blueprints, but can also be called from C++
 */
UCLASS()
class FLYINGNAVSYSTEM_API UFlyingNavFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static EPathfindingResult ConvertToPathfindingResult(const ENavigationQueryResult::Type Result);

	// Rebuild all Flying Navigation Data agents, inline C++ version.
	// To build a specific navigation data, use Get Flying Navigation Data and call Rebuild Navigation Data on it. 
	// WARNING: Using a small detail size and hi-res level can cause a performance hit
	static void RebuildAllFlyingNavigation(UWorld* World);

	// Rebuild all Flying Navigation Data agents, blocking this thread.
	// To build a specific navigation data, use Get Flying Navigation Data and call Rebuild Navigation Data on it. 
	// WARNING: Using a small detail size and hi-res level can cause a performance hit
	UFUNCTION(BlueprintCallable, Category="Flying Navigation", meta=(WorldContext="WorldContextObject"))
	static void RebuildAllFlyingNavigation(const UObject* WorldContextObject);

	static void DrawNavPath(
		UWorld* World, 
		FNavPathSharedPtr NavPath,
		const FColor PathColor = FColor::Red,
		const FVector& PathOffset = FVector::ZeroVector,
		const bool bPersistent = true);

	// Draw the navigation path returned by the FindPathToLocationSynchronously function
	UFUNCTION(BlueprintCallable, Category="Flying Navigation", meta=(WorldContext="WorldContextObject", DevelopmentOnly))
	static void DrawNavPath(
		const UObject* WorldContextObject,
		UNavigationPath* NavPath,
		const FLinearColor PathColor = FLinearColor::Red,
		const FVector PathOffset = FVector::ZeroVector,
		const bool bPersistent = true);

	// Updates a NavigationPath to a new set of path points
	UFUNCTION(BlueprintCallable, Category="Flying Navigation", meta=(WorldContext="WorldContextObject"))
	static UNavigationPath* SetNavigationPathPoints(const UObject* WorldContextObject, UNavigationPath* NavPath, const TArray<FVector>& PathPoints);
	
	// Get the FlyingNavigationData actor for a given Pawn.
    // Will return nullptr if the Pawn's movement component specifies a different Preferred Nav Data.
	UFUNCTION(BlueprintPure, Category="Flying Navigation")
	static AFlyingNavigationData* GetFlyingNavigationData(const APawn* NavAgent);

	// TODO: Async pathfinding in Behaviour Trees and AI Move To

	// USE THIS FUNCTION INSTEAD OF UNavigationSystemV1::FindPathAsync FOR FLYING NAV SYSTEM ASYNC QUERIES
	static uint32 FindPathAsync(
		UNavigationSystemV1* NavSys,
		const FNavAgentProperties& AgentProperties,
		FPathFindingQuery& Query,
		const FNavPathQueryDelegate& ResultDelegate,
		EPathFindingMode::Type Mode);

	// Helper function. Goal actor is passed to async delegate for observing, PathStart and PathEnd are used for pathfinding
	static UNavigationPath* FindPathToLocationAsynchronously(
		const UObject* WorldContextObject,
		const FLatentActionInfo& LatentInfo,
		const FVector& PathStart,
		const FVector& PathEnd,
		AActor* const PathfindingContext,
		AActor* const GoalActor,
		const float TetherDistance,
		const TSubclassOf<UNavigationQueryFilter> FilterClass);
	
	/*
	 * Finds path on separate thread. 
	 * @param Pathfinding Context could be one of following: NavigationData (like FlyingNavigationData actor), Pawn or Controller.
	 * @param bAllowPartialPaths: Find a path despite the goal not being accessible - WARNING can be slow
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation", meta = (WorldContext="WorldContextObject", Latent, LatentInfo = "LatentInfo"))
	static UNavigationPath* FindPathToLocationAsynchronously(
		UObject* WorldContextObject,
		const FLatentActionInfo LatentInfo,
        const FVector& PathStart,
        const FVector& PathEnd,
        AActor* PathfindingContext = nullptr,
        TSubclassOf<UNavigationQueryFilter> FilterClass = nullptr)
	{
		return FindPathToLocationAsynchronously(WorldContextObject,LatentInfo,PathStart,PathEnd,PathfindingContext,nullptr, 0.f, FilterClass);
	}

	/*
	 * Finds path on a separate thread.
	 * Main advantage over FindPathToLocationAsynchronously is that the resulting path will automatically get updated if
	 * the goal actor moves more than TetherDistance away from last path node. Updates are also asynchronous.
	 * @param Pathfinding Context could be one of following: NavigationData (like FlyingNavigationData actor), Pawn or Controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation", meta = (WorldContext="WorldContextObject", Latent, LatentInfo = "LatentInfo"))
	static UNavigationPath* FindPathToActorAsynchronously(
		UObject* WorldContextObject,
		const FLatentActionInfo LatentInfo,
		const FVector& PathStart,
		AActor* GoalActor,
		float TetherDistance = 50.f,
		AActor* PathfindingContext = nullptr,
		TSubclassOf<UNavigationQueryFilter> FilterClass = nullptr);

	// Takes a path generated by Flying Navigation System and returns if it was invalid, an error, fail or success:
	// Invalid: If start or end point is blocked or out of bounds, or pathfinding failed to start (check log)
	// Error: Algorithm got stuck in infinite loop
	// Fail: If start and end points are not connected and partial paths are not enabled
	// Null: If the path was null (check log for further details). Check the default agent is valid in project settings.
	// RecastError: Path was not generated by the flying nav system (check recast agents are not interfering)
	// Success: Path is valid
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static EPathfindingResult GetPathfindingResult(UNavigationPath* Path);

	// Checks whether a given position is in free space within the given agent's octree (and can therefore be used for pathfinding queries).
    UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
    static bool IsPositionAValidEndpoint(const APawn* NavAgent, const FVector& Position, const bool bAllowBlocked = false);

	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static void RequestMove(UNavigationPath* PathToFollow, class AAIController* Controller);

	UFUNCTION(BlueprintPure, Category = "AI|Navigation")
	static FVector GetActorFeetLocation(const APawn* Pawn);

	UFUNCTION(BlueprintPure, Category = "AI|Navigation")
	static FVector GetActorFeetOffset(const APawn* Pawn);
	
	/*
	* Constructs a centripetal Catmull-Rom Spline from path points. Returns nullptr if an error
	*/
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation")
	static UCatmullRomSpline* MakeCatmullRomSpline(const TArray<FVector>& PathPoints)
	{
		UCatmullRomSpline* CRSpline = NewObject<UCatmullRomSpline>();
		if (CRSpline->GenerateSpline(PathPoints))
		{
			return CRSpline;
		}
		return nullptr;
	}
	
	/*
	* Smooths navigation path by constructing a centripetal Catmull-Rom Spline and sampling points.
	* If Path is invalid, returns Path unmodified;
	* @param SampleLength: distance between samples along path in Unreal units.
	*/
	UFUNCTION(BlueprintCallable, Category = "AI|Navigation", meta = (WorldContext="WorldContextObject"))
	static UNavigationPath* SmoothPath(UObject* WorldContextObject, UNavigationPath* Path, const float SampleLength = 100.f);
};
