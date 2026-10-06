#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include <mig/c/api.h>

#include "MIGTrackerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMIGAction, const FString&, Action, const FString&,
                                             InputId);

UCLASS(ClassGroup = Input, meta = (BlueprintSpawnableComponent))
class MIGRUNTIME_API UMIGTrackerComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "MIG")
    FMIGAction OnAction;
    UFUNCTION(BlueprintCallable, Category = "MIG")
    bool ImportJson(const FString& Json);
    UFUNCTION(BlueprintCallable, Category = "MIG")
    bool IsActive(int32 InputIndex) const;
    UFUNCTION(BlueprintCallable, Category = "MIG")
    void Reset(bool Recalibrate = false);
    UFUNCTION(BlueprintCallable, Category = "MIG")
    void Close();
    bool SubmitFrame(const mig_packet& Packet);

protected:
    void EndPlay(const EEndPlayReason::Type Reason) override;
    void BeginDestroy() override;

private:
    mig_tracker* Tracker{};
};
