#include "theme.hpp"

namespace mig::linux_ui {
void apply_widget_theme(QWidget& window, bool dark) {
    const auto text = dark ? "#eeeeee" : "#222222";
    const auto surface = dark ? "#252526" : "#f3f3f3";
    const auto base = dark ? "#1e1e1e" : "#ffffff";
    const auto thumb = dark ? "#5a5d5e" : "#b7b7b7";
    const auto hover = dark ? "#787c80" : "#92979c";
    QPalette colors;
    colors.setColor(QPalette::Window, QColor(surface));
    colors.setColor(QPalette::Base, QColor(base));
    colors.setColor(QPalette::Text, QColor(text));
    colors.setColor(QPalette::WindowText, QColor(text));
    colors.setColor(QPalette::ButtonText, QColor(text));
    colors.setColor(QPalette::Button, QColor(surface));
    colors.setColor(QPalette::Highlight, QColor("#267acb"));
    window.setPalette(colors);
    window.setStyleSheet(
        QString("QWidget { color: %1; background-color: %2; }"
                "QListWidget, QComboBox, QPlainTextEdit { background-color: %3; }"
                "QPushButton, QComboBox, QSpinBox { border: 1px solid #737373;"
                "border-radius: 6px; padding: 7px; }"
                "QPushButton:hover { border-color: #3794ff; }"
                "QListWidget::item { color: %1; border: 1px solid #737373;"
                "padding: 10px; margin: 3px; }"
                "QListWidget::item:selected { color: %1; border-color: #3794ff; }"
                "QScrollBar { background: %2; border: none; }"
                "QScrollBar:vertical { width: 12px; margin: 2px 0; }"
                "QScrollBar:horizontal { height: 12px; margin: 0 2px; }"
                "QScrollBar::handle { background: %4; border-radius: 4px; }"
                "QScrollBar::handle:vertical { min-height: 24px; }"
                "QScrollBar::handle:horizontal { min-width: 24px; }"
                "QScrollBar::handle:hover { background: %5; }"
                "QScrollBar::handle:pressed { background: #3794ff; }"
                "QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }"
                "QScrollBar::add-page, QScrollBar::sub-page { background: none; }"
                "QAbstractScrollArea::corner { background: %2; border: none; }")
            .arg(text, surface, base, thumb, hover));
}
} // namespace mig::linux_ui
