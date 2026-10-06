#pragma once
#include "MigInputComponent.h"

#include "MigRaisedHandsComponent.generated.h"

UCLASS(ClassGroup = Input, meta = (BlueprintSpawnableComponent))
class MIGEXAMPLE_API UMigRaisedHandsComponent : public UMigInputComponent {
    GENERATED_BODY()
public:
    UMigRaisedHandsComponent() {
        RaisedHands = true;
    }
};
