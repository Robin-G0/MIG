#include "MigInputComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/IPluginManager.h"
#include "MigExamplePanel.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UMigInputComponent::UMigInputComponent() {
    PrimaryComponentTick.bCanEverTick = true;
}

void UMigInputComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!UseSyntheticDemo || !RaisedHands || !Tracker || DemoSequence >= 90) {
        return;
    }
    DemoTime += DeltaTime;
    if (DemoTime < 0.02f) {
        return;
    }
    DemoTime = 0;
    // This fixture demonstrates provider wiring without a camera. Disable it
    // before submitting real unmirrored packets from your pose provider.
    mig_packet Packet{};
    Packet.sequence = ++DemoSequence;
    Packet.timestamp_ms = DemoSequence * 20;
    Packet.aspect = 1;
    Packet.body[11 * 8] = 0.65f;
    Packet.body[12 * 8] = 0.35f;
    for (int Joint : {11, 12, 15, 16}) {
        Packet.body[Joint * 8 + 3] = 1;
    }
    Packet.body[11 * 8 + 1] = Packet.body[12 * 8 + 1] = 0.45f;
    const float Row = DemoSequence < 60 ? 5.5f : FMath::Max(1.5f, 5.5f - (DemoSequence - 59) / 5.f);
    Packet.body[15 * 8] = 0.62f;
    Packet.body[16 * 8] = 0.38f;
    Packet.body[15 * 8 + 1] = Packet.body[16 * 8 + 1] = 0.45f + (Row - 3.5f) * 0.06f;
    SubmitFrame(Packet);
}

void UMigInputComponent::BeginPlay() {
    Super::BeginPlay();
    // One handle per component owns recognition/calibration; BeginPlay creates
    // it once, SubmitFrame updates it, and EndPlay destroys it on the game thread.
    Tracker = mig_create("{\"schema_version\":2,\"tracking\":{\"hands\":true},\"inputs\":[]}");
    DemoSequence = 0;
    DemoTime = 0;
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
    // Packet carries fresh landmarks, aspect and monotonic time. The result is
    // the number of logical actions, not injected keyboard shortcuts.
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
