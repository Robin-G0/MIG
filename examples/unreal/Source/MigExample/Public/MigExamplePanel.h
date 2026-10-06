#pragma once
#include "Blueprint/UserWidget.h"

#include "MigExamplePanel.generated.h"

class UMigInputComponent;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class MIGEXAMPLE_API UMigExamplePanel : public UUserWidget {
    GENERATED_BODY()
public:
    UPROPERTY()
    TObjectPtr<UMigInputComponent> Input;
    void Refresh();

protected:
    TSharedRef<SWidget> RebuildWidget() override;

private:
    UFUNCTION()
    void ImportProfile();
    UPROPERTY()
    TObjectPtr<UEditableTextBox> Path;
    UPROPERTY()
    TObjectPtr<UTextBlock> Status;
};
