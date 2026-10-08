#pragma once
#include "../apps/import_review.hpp"
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace mig::linux_ui {
inline bool review_import(QWidget* parent, const Configuration& config) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Review imported movement mappings");
    dialog.resize(720, 540);
    QVBoxLayout layout(&dialog);
    QTextEdit summary;
    summary.setReadOnly(true);
    summary.setPlainText(QString::fromStdString(ui::configuration_review(config)));
    layout.addWidget(&summary);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons.button(QDialogButtonBox::Ok)->setText("Import reviewed profile");
    buttons.button(QDialogButtonBox::Cancel)->setDefault(true);
    layout.addWidget(&buttons);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    return dialog.exec() == QDialog::Accepted;
}
} // namespace mig::linux_ui
