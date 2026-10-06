#include "MigExamplePanel.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "MigInputComponent.h"

TSharedRef<SWidget> UMigExamplePanel::RebuildWidget() {
    auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    auto* Border = WidgetTree->ConstructWidget<UBorder>();
    Border->SetPadding(FMargin(12));
    Border->SetBrushColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.9f));
    auto* Slot = Canvas->AddChildToCanvas(Border);
    Slot->SetPosition(FVector2D(12, 12));
    Slot->SetAutoSize(true);
    auto* Stack = WidgetTree->ConstructWidget<UVerticalBox>();
    Border->SetContent(Stack);
    if (Input && !Input->IsRaisedHands()) {
        Path = WidgetTree->ConstructWidget<UEditableTextBox>();
        Path->SetHintText(FText::FromString(TEXT("Path to a JSON profile")));
        Stack->AddChildToVerticalBox(Path);
        auto* Button = WidgetTree->ConstructWidget<UButton>();
        auto* Label = WidgetTree->ConstructWidget<UTextBlock>();
        Label->SetText(FText::FromString(TEXT("Import JSON profile")));
        Button->SetContent(Label);
        Button->OnClicked.AddDynamic(this, &UMigExamplePanel::ImportProfile);
        Stack->AddChildToVerticalBox(Button);
    }
    Status = WidgetTree->ConstructWidget<UTextBlock>();
    Status->SetWrapTextAt(720);
    Stack->AddChildToVerticalBox(Status);
    Refresh();
    return Super::RebuildWidget();
}

void UMigExamplePanel::Refresh() {
    if (Input && Status) {
        Status->SetText(FText::FromString(Input->DisplayAction));
    }
}

void UMigExamplePanel::ImportProfile() {
    if (Input && Path) {
        Input->ImportProfile(Path->GetText().ToString());
    }
}
