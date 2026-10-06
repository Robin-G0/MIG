#include "MigInputComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IPluginManager.h"
#include "MigExamplePanel.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UMigInputComponent::BeginPlay() {
    Super::BeginPlay();
    Tracker = mig_create("{\"schema_version\":2,\"tracking\":{\"hands\":true},\"inputs\":[]}");
    if (!Tracker) {
        UE_LOG(LogTemp, Error, TEXT("MIG: %s"), UTF8_TO_TCHAR(mig_last_error()));
        return;
    }
    if (RaisedHands) {
        const auto Plugin = IPluginManager::Get().FindPlugin(TEXT("MigExample"));
        if (Plugin.IsValid()) {
            ImportProfile(FPaths::Combine(Plugin->GetContentDir(), TEXT("raised-hands.json")));
        }
    } else if (!ProfilePath.IsEmpty()) {
        ImportProfile(ProfilePath);
    }
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) {
        Panel = CreateWidget<UMigExamplePanel>(Controller);
        if (Panel) {
            Panel->Input = this;
            Panel->AddToViewport();
        }
    }
}

bool UMigInputComponent::ImportProfile(const FString& Path) {
    FString Json;
    if (!Tracker || !FFileHelper::LoadFileToString(Json, *Path)) {
        SetDisplayAction(FString::Printf(TEXT("Cannot open MIG profile %s"), *Path));
        UE_LOG(LogTemp, Error, TEXT("%s"), *DisplayAction);
        return false;
    }
    if (mig_load(Tracker, TCHAR_TO_UTF8(*Json)) < 0) {
        SetDisplayAction(UTF8_TO_TCHAR(mig_last_error()));
        UE_LOG(LogTemp, Error, TEXT("%s"), *DisplayAction);
        return false;
    }
    ProfilePath = Path;
    SetDisplayAction(TEXT("Profile imported. Recalibrating."));
    return true;
}
void UMigInputComponent::SubmitFrame(const mig_packet& Packet) {
    if (!Tracker) {
        return;
    }
    const int Count = mig_update(Tracker, &Packet);
    if (Count < 0) {
        UE_LOG(LogTemp, Error, TEXT("MIG: %s"), UTF8_TO_TCHAR(mig_last_error()));
        return;
    }
    TArray<TPair<FString, FString>> Actions;
    for (int Index = 0; Index < Count; ++Index) {
        Actions.Emplace(UTF8_TO_TCHAR(mig_event_action(Tracker, Index)),
                        UTF8_TO_TCHAR(mig_event_id(Tracker, Index)));
    }
    FString Status;
    for (const auto& Action : Actions) {
        Status += FString::Printf(TEXT("%s (input %s)\n"), *Action.Key, *Action.Value);
    }
    if (!Status.IsEmpty()) {
        SetDisplayAction(Status);
    }
    for (const auto& Action : Actions) {
        OnMotion.Broadcast(Action.Key, Action.Value);
    }
}
void UMigInputComponent::SetDisplayAction(const FString& Action) {
    DisplayAction = Action;
    if (Panel) {
        Panel->Refresh();
    }
}

void UMigInputComponent::EndPlay(const EEndPlayReason::Type Reason) {
    if (Panel) {
        Panel->RemoveFromParent();
        Panel = nullptr;
    }
    mig_destroy(Tracker);
    Tracker = nullptr;
    Super::EndPlay(Reason);
}
