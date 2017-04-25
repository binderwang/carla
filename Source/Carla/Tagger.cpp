// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "Tagger.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"

enum class Label : uint8
{
  None         =   0u,
  Buildings    =   1u,
  Fences       =   2u,
  Other        =   3u,
  Pedestrians  =   4u,
  Poles        =   5u,
  RoadLines    =   6u,
  Roads        =   7u,
  Sidewalks    =   8u,
  Vegetation   =   9u,
  Vehicles     =  10u,
  Walls        =  11u,
};

static Label GetLabel(const FString &str) {
  if (str == "Buildings")            return Label::Buildings;
  else if (str == "Fences")          return Label::Fences;
  else if (str == "Pedestrians")     return Label::Pedestrians;
  else if (str == "Pole")            return Label::Poles;
  else if (str == "Props")           return Label::Other;
  else if (str == "Road")            return Label::Roads;
  else if (str == "RoadLines")       return Label::RoadLines;
  else if (str == "SideWalk")        return Label::Sidewalks;
  else if (str == "Vegetation")      return Label::Vegetation;
  else if (str == "Vehicles")        return Label::Vehicles;
  else if (str == "Walls")           return Label::Walls;
  else                               return Label::None;
}

template <typename T>
static auto cast(T label)
{
  return static_cast<typename std::underlying_type<T>::type>(label);
}

ATagger::ATagger()
{
  PrimaryActorTick.bCanEverTick = false;
}

#if WITH_EDITOR
void ATagger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bTriggerTagObjects) {
      TagObjects();
    }
  }
  bTriggerTagObjects = false;
}
#endif // WITH_EDITOR

static void setStencilValue(UPrimitiveComponent *comp, const Label &label) {
  if (label != Label::None)
  {
    comp->SetRenderCustomDepth(true);
    comp->SetCustomDepthStencilValue(cast(label));
  }
}

void ATagger::TagObjects()
{
  for (TActorIterator<AActor> it(GetWorld()); it; ++it) {

#ifdef CARLA_TAGGER_EXTRA_LOG
    UE_LOG(LogCarla, Warning, TEXT("Actor: %s"), *it->GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

    /// get UStaticMeshComponents
    TArray<UStaticMeshComponent*> staticComponents;
    it->GetComponents<UStaticMeshComponent>(staticComponents);

    for (auto& meshIt : staticComponents)
    {

#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Warning, TEXT("  + StaticMeshComponent: %s"), *meshIt->GetName());
#endif // CARLA_TAGGER_EXTRA_LOG

      FString Path = meshIt->GetStaticMesh()->GetPathName();
      TArray<FString> stringArray;
      Path.ParseIntoArray(stringArray, TEXT("/"), false);
      /*for (int32 i = 0; i < stringArray.Num(); i++) {
      UE_LOG(LogCarla, Warning, TEXT("  -\"%s\""), *stringArray[i]);
      }*/
      if (stringArray.Num() > 3)
      {
        Label lab = GetLabel(stringArray[3]);

#ifdef CARLA_TAGGER_EXTRA_LOG
        UE_LOG(LogCarla, Warning, TEXT("    - Label: \"%s\""), *stringArray[3]);
#endif // CARLA_TAGGER_EXTRA_LOG

        setStencilValue(meshIt, lab);
      }
    }

    /// get USkeletalMeshComponents
    TArray<USkeletalMeshComponent*> skeletalComponents;
    it->GetComponents<USkeletalMeshComponent>(skeletalComponents);

    for (auto& meshIt : skeletalComponents)
    {
#ifdef CARLA_TAGGER_EXTRA_LOG
      UE_LOG(LogCarla, Warning, TEXT("  + SkeletalMeshComponent: %s"), *meshIt->GetName());
#endif // CARLA_TAGGER_EXTRA_LOG
      FString Path = meshIt->GetPhysicsAsset()->GetPathName();
      TArray<FString> stringArray;
      Path.ParseIntoArray(stringArray, TEXT("/"), false);
      if (stringArray.Num() > 3)
      {
        Label lab = GetLabel(stringArray[3]);

#ifdef CARLA_TAGGER_EXTRA_LOG
        UE_LOG(LogCarla, Warning, TEXT("    - Label: \"%s\""), *stringArray[3]);
#endif // CARLA_TAGGER_EXTRA_LOG

        setStencilValue(meshIt, lab);
      }
    }
  }
}