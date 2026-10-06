#include "MIGTrackerComponent.h"

bool UMIGTrackerComponent::ImportJson(const FString& Json) {
    if (mig_abi_version() != 1 || mig_packet_size() != sizeof(mig_packet)) {
        return false;
    }
    for (int32 Index = 0; Index < Json.Len(); ++Index) {
        if (Json[Index] == 0) {
            return false;
        }
    }
    const FTCHARToUTF8 Utf8(*Json);
    if (Tracker) {
        return mig_load(Tracker, Utf8.Get()) == 0;
    }
    Tracker = mig_create(Utf8.Get());
    return Tracker != nullptr;
}

bool UMIGTrackerComponent::SubmitFrame(const mig_packet& Packet) {
    if (!Tracker) {
        return false;
    }
    const int Count = mig_update(Tracker, &Packet);
    if (Count < 0) {
        return false;
    }
    TArray<TPair<FString, FString>> Events;
    Events.Reserve(Count);
    for (int Index = 0; Index < Count; ++Index) {
        Events.Emplace(UTF8_TO_TCHAR(mig_event_action(Tracker, Index)),
                       UTF8_TO_TCHAR(mig_event_id(Tracker, Index)));
    }
    for (const auto& Event : Events) {
        OnAction.Broadcast(Event.Key, Event.Value);
    }
    return true;
}

bool UMIGTrackerComponent::IsActive(int32 InputIndex) const {
    return Tracker && InputIndex >= 0 && mig_active(Tracker, InputIndex) != 0;
}

void UMIGTrackerComponent::Reset(bool Recalibrate) {
    if (Tracker) {
        mig_reset(Tracker, Recalibrate);
    }
}

void UMIGTrackerComponent::Close() {
    if (Tracker) {
        mig_destroy(Tracker);
        Tracker = nullptr;
    }
}

void UMIGTrackerComponent::EndPlay(const EEndPlayReason::Type Reason) {
    Close();
    Super::EndPlay(Reason);
}

void UMIGTrackerComponent::BeginDestroy() {
    Close();
    Super::BeginDestroy();
}
