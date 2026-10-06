#pragma once
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include <mig/c/api.h>

#include "MigInputComponent.generated.h"

class UMigExamplePanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMigAction, const FString&, Action, const FString&,
                                             InputId);
UCLASS(ClassGroup = Input, meta = (BlueprintSpawnableComponent))
class MIGEXAMPLE_API UMigInputComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "MIG")
    FString ProfilePath;
    UPROPERTY(BlueprintAssignable, Category = "MIG")
    FMigAction OnMotion;
    UFUNCTION(BlueprintCallable, Category = "MIG")
    bool ImportProfile(const FString& Path);
    UPROPERTY(BlueprintReadOnly, Category = "MIG")
    FString DisplayAction = TEXT("Import a profile, then keep shoulders visible.");
    bool IsRaisedHands() const {
        return RaisedHands;
    }
    // Submit anatomical packets from your estimator on the game thread.
    void SubmitFrame(const mig_packet& Packet);

protected:
    bool RaisedHands{};
    void BeginPlay() override;
    void EndPlay(const EEndPlayReason::Type Reason) override;

private:
    void SetDisplayAction(const FString& Action);
    mig_tracker* Tracker{};
    UPROPERTY()
    TObjectPtr<UMigExamplePanel> Panel;
};
