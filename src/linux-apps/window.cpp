#include "window.hpp"
#include "../apps/authoring.hpp"
#include "theme.hpp"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QVBoxLayout>
#include <mig/format/configuration.hpp>
namespace mig::linux_ui {
Window::Window(std::string runtime, unsigned camera)
    : runtime_(std::move(runtime)), camera_(camera) {
    setWindowTitle(QString("Motion Input Grid — %1").arg(MIG_APP_NAME));
    resize(1200, 850);
    canvas_ = new Canvas(this);
    canvas_->failed = [this](const std::string& message) {
        report(QString::fromStdString(message));
    };
    setCentralWidget(canvas_);
    log_dock_ = new QDockWidget("Logs", this);
    logs_ = new QPlainTextEdit(log_dock_);
    logs_->setReadOnly(true);
    logs_->setMaximumBlockCount(256);
    log_dock_->setWidget(logs_);
    addDockWidget(Qt::BottomDockWidgetArea, log_dock_);
    log_dock_->hide();
    build_sidebar();
    build_menus();
    apply_theme(true);
    connect(&timer_, &QTimer::timeout, this, [this] { tick(); });
    timer_.start(20);
    report("Draw in Full grid, edit settings, then save your profile. Camera starts explicitly.");
}
Window::~Window() {
    stop();
}
void Window::report(const QString& message) {
    statusBar()->showMessage(message);
    logs_->appendPlainText(message);
}
void Window::apply_theme(bool dark) {
    dark_ = dark;
    apply_widget_theme(*this, dark);
}
void Window::build_menus() {
    auto* file = menuBar()->addMenu("File");
    file->addAction("Open", this, [this] {
        const auto path = QFileDialog::getOpenFileName(this, "Open profile", {}, "JSON (*.json)");
        if (!path.isEmpty()) {
            load(path.toStdString());
        }
    });
    file->addAction("Save", this, [this] {
        stop();
        const auto path = QFileDialog::getSaveFileName(this, "Save profile", {}, "JSON (*.json)");
        if (path.isEmpty()) {
            return;
        }
        try {
            save_configuration(config_, path.toStdString());
            report("Profile saved.");
        } catch (const std::exception& error) {
            report(error.what());
        }
    });
    file->addAction("Pro: edit complete profile", this, [this] { edit_json(); });
    auto* view = menuBar()->addMenu("View");
    const auto toggle = [&](const char* name, bool& value) {
        auto* action = view->addAction(name);
        action->setCheckable(true);
        action->setChecked(value);
        connect(action, &QAction::toggled, this, [this, &value](bool checked) {
            value = checked;
            canvas_->update();
        });
    };
    toggle("Grid", canvas_->show_grid);
    toggle("Detection dots", canvas_->show_dots);
    toggle("Full grid / camera body view", canvas_->full_grid);
    view->addAction(log_dock_->toggleViewAction());
    auto* theme = view->addAction("Dark mode");
    theme->setCheckable(true);
    theme->setChecked(true);
    connect(theme, &QAction::toggled, this, [this](bool checked) { apply_theme(checked); });
}
void Window::build_sidebar() {
    auto* dock = new QDockWidget("Inputs and drawing", this);
    auto* panel = new QWidget(dock);
    auto* layout = new QVBoxLayout(panel);
    const auto button = [&](const char* name, auto callback) {
        auto* control = new QPushButton(name, panel);
        layout->addWidget(control);
        connect(control, &QPushButton::clicked, this, callback);
    };
    button("Start camera", [this] { start(); });
    button("Stop / release keys", [this] { stop(); });
    button("Recalibrate", [this] {
        if (running_) {
            start();
        }
    });
    keyboard_switch_ = new QCheckBox("Keyboard output (X11)", panel);
    keyboard_switch_->setEnabled(keyboard_.available());
    keyboard_switch_->setToolTip(
        "Opt-in XTest output. Requires X11; native Wayland is unsupported.");
    layout->addWidget(keyboard_switch_);
    connect(keyboard_switch_, &QCheckBox::toggled, this, [this](bool enabled) {
        if (!enabled) {
            output_.cancel([this](int key, bool release) { return keyboard_.key(key, release); },
                           [](char16_t, bool) { return true; });
        }
    });
    inputs_ = new QListWidget(panel);
    layout->addWidget(inputs_);
    connect(inputs_, &QListWidget::currentRowChanged, this, [this](int row) { select_input(row); });
    button("Add input", [this] {
        stop();
        if (config_.motions.size() >= 64) {
            report("Maximum 64 inputs.");
            return;
        }
        Motion input;
        unsigned number = 1;
        do {
            input.id = "input" + std::to_string(number++);
        } while (std::any_of(config_.motions.begin(), config_.motions.end(),
                             [&](const auto& item) { return item.id == input.id; }));
        input.name = input.action = input.id;
        input.steps.push_back({"step1", StepMode::Ordered});
        config_.motions.push_back(input);
        refresh_inputs();
        inputs_->setCurrentRow(int(config_.motions.size() - 1));
    });
    button("Input settings / bindings", [this] { edit_settings(); });
    button("Delete input", [this] {
        stop();
        const int row = inputs_->currentRow();
        if (row >= 0) {
            config_.motions.erase(config_.motions.begin() + row);
            refresh_inputs();
        }
    });
    auto* body = new QComboBox(panel);
    for (const auto& part : landmarks) {
        body->addItem(QString::fromUtf8(part.name.data(), int(part.name.size())), part.index);
    }
    body->setCurrentIndex(body->findData(15));
    layout->addWidget(body);
    connect(body, &QComboBox::currentIndexChanged, this, [this, body] {
        canvas_->landmark = body->currentData().toInt();
        canvas_->selection.clear();
        canvas_->update();
    });
    button("Save layer as selected body part", [this] {
        stop();
        if (!canvas_->motion) {
            return;
        }
        const auto layers = ui::layers(*canvas_->motion);
        QStringList names;
        for (const auto& layer : layers) {
            names.push_back(QString::fromStdString(std::string(landmark_name(layer.landmark))));
        }
        bool accepted;
        const auto source = QInputDialog::getItem(this, "Save layer", "Existing layer to reassign",
                                                  names, 0, false, &accepted);
        if (!accepted) {
            return;
        }
        try {
            ui::reassign_layer(*canvas_->motion, landmark_index(source.toStdString()),
                               canvas_->landmark);
            canvas_->update();
        } catch (const std::exception& error) {
            report(error.what());
        }
    });
    const char* labels[]{"Required (green)", "Forbidden (red)", "Trigger (yellow)",
                         "Interaction (purple)"};
    for (int type = 0; type < 4; ++type) {
        button(labels[type], [this, type] {
            stop();
            canvas_->type = ConstraintType(type);
            canvas_->erase = false;
            canvas_->select = false;
            canvas_->selection.clear();
        });
    }
    button("Select: drag / Ctrl to add", [this] {
        stop();
        canvas_->select = true;
        canvas_->erase = false;
    });
    button("Clear all finger rules", [this] {
        stop();
        if (canvas_->motion) {
            const int index = inputs_->currentRow();
            ui::clear_fingers(*canvas_->motion);
            refresh_inputs();
            inputs_->setCurrentRow(index);
        }
    });
    button("Eraser", [this] {
        stop();
        canvas_->erase = true;
        canvas_->select = false;
        canvas_->selection.clear();
    });
    auto* order = new QSpinBox(panel);
    order->setRange(0, 1024);
    order->setSpecialValueText("Order: automatic");
    layout->addWidget(order);
    connect(order, &QSpinBox::valueChanged, this, [this](int value) { canvas_->order = value; });
    auto* gesture = new QComboBox(panel);
    for (const auto name : gesture_names) {
        gesture->addItem(QString::fromUtf8(name.data(), int(name.size())));
    }
    gesture->setCurrentIndex(int(Gesture::V));
    layout->addWidget(gesture);
    connect(gesture, &QComboBox::currentIndexChanged, this,
            [this](int value) { canvas_->interaction.gesture = Gesture(value); });
    auto* hold = new QSpinBox(panel);
    hold->setRange(0, maximum_interaction_hold_ms);
    hold->setSuffix(" ms hold");
    layout->addWidget(hold);
    connect(hold, &QSpinBox::valueChanged, this,
            [this](int value) { canvas_->interaction.hold_ms = value; });
    canvas_->changed = [this] {
        if (running_) {
            stop();
        }
        config_.track_hands |= canvas_->type == ConstraintType::Interaction;
        canvas_->interaction.hand = canvas_->landmark == 16 ? HandSide::Right : HandSide::Left;
    };
    dock->setWidget(panel);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}
void Window::refresh_inputs() {
    canvas_->motion = nullptr;
    inputs_->clear();
    for (const auto& input : config_.motions) {
        inputs_->addItem(QString::fromStdString(input.name + "\n" + ui::binding_summary(input)));
    }
    if (!config_.motions.empty()) {
        inputs_->setCurrentRow(0);
    }
    canvas_->update();
}
void Window::select_input(int index) {
    canvas_->selection.clear();
    canvas_->motion =
        index >= 0 && index < int(config_.motions.size()) ? &config_.motions[index] : nullptr;
    canvas_->update();
}
void Window::load(const std::string& path) {
    try {
        auto proposed = load_configuration(path);
        stop();
        config_ = std::move(proposed);
        refresh_inputs();
        report("Profile loaded.");
    } catch (const std::exception& error) {
        report(error.what());
    }
}
void Window::edit_settings() {
    stop();
    if (!canvas_->motion) {
        return;
    }
    auto draft = *canvas_->motion;
    QDialog dialog(this);
    dialog.setWindowTitle("Input settings");
    QFormLayout form(&dialog);
    QLineEdit name(QString::fromStdString(draft.name)),
        action(QString::fromStdString(draft.action));
    QLineEdit keys(QString::fromStdString(ui::binding_label(draft.keyboard)));
    QCheckBox mirror;
    mirror.setChecked(draft.mirror);
    QComboBox mode;
    for (const auto item : action_mode_names) {
        mode.addItem(QString::fromUtf8(item.data(), int(item.size())));
    }
    mode.setCurrentIndex(int(draft.action_mode));
    QSpinBox repeat;
    repeat.setRange(20, 60000);
    repeat.setValue(draft.repeat_interval_ms);
    QSpinBox cooldown;
    cooldown.setRange(0, 10000);
    cooldown.setValue(int(draft.cooldown_ms));
    form.addRow("Name", &name);
    form.addRow("Logical action", &action);
    form.addRow("Keys: Ctrl + C _ \"text\"", &keys);
    form.addRow("Mirror", &mirror);
    form.addRow("Action mode", &mode);
    form.addRow("Repeat interval ms", &repeat);
    form.addRow("Cooldown ms", &cooldown);
    QDialogButtonBox buttons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        try {
            draft.name = name.text().toStdString();
            draft.action = action.text().toStdString();
            draft.keyboard = ui::parse_binding(keys.text().toStdString());
            draft.mirror = mirror.isChecked();
            draft.action_mode = ActionMode(mode.currentIndex());
            draft.repeat_interval_ms = repeat.value();
            draft.cooldown_ms = cooldown.value();
            Configuration proposal = config_;
            proposal.motions[inputs_->currentRow()] = draft;
            validate(proposal);
            config_ = std::move(proposal);
            dialog.accept();
        } catch (const std::exception& error) {
            QMessageBox::warning(&dialog, "Invalid settings", error.what());
        }
    });
    dialog.exec();
    refresh_inputs();
}
void Window::edit_json() {
    stop();
    QDialog dialog(this);
    dialog.resize(850, 700);
    dialog.setWindowTitle("Pro profile editor: all schema-v2 fields");
    QVBoxLayout layout(&dialog);
    QPlainTextEdit json(QString::fromStdString(serialize_configuration(config_)));
    layout.addWidget(&json);
    QDialogButtonBox buttons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout.addWidget(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&] {
        try {
            auto proposed = parse_configuration(json.toPlainText().toStdString());
            config_ = std::move(proposed);
            dialog.accept();
        } catch (const std::exception& error) {
            QMessageBox::warning(&dialog, "Invalid profile", error.what());
        }
    });
    dialog.exec();
    refresh_inputs();
}
void Window::start() {
    stop();
    try {
        validate(config_);
        capture_.start(config_, runtime_, camera_);
        running_ = true;
        report("Starting camera; keep shoulders visible for calibration.");
    } catch (const std::exception& error) {
        report(error.what());
    }
}
void Window::stop() {
    capture_.stop();
    running_ = false;
    active_.fill(false);
    output_.cancel([this](int key, bool release) { return keyboard_.key(key, release); },
                   [](char16_t, bool) { return true; });
    canvas_->snapshot = {};
    canvas_->update();
}
void Window::tick() {
    const auto now = native::now_ms();
    Snapshot snapshot;
    if (capture_.take(snapshot)) {
        if (!snapshot.error.empty()) {
            stop();
            report(QString::fromStdString(snapshot.error));
            return;
        }
        last_frame_ = snapshot.body.timestamp_ms;
        active_ = snapshot.active;
        if (snapshot.paused) {
            output_.cancel([this](int key, bool release) { return keyboard_.key(key, release); },
                           [](char16_t, bool) { return true; });
        }
        for (const auto& event : snapshot.events) {
            if (now - event.timestamp_ms > 250) {
                continue;
            }
            const auto input = event.motion;
            flashed_[input] = now + 900;
            report(QString::fromStdString("Action: " + config_.motions[input].action));
            if (keyboard_switch_->isChecked()) {
                output_.trigger(input, config_.motions[input], now);
            }
        }
        canvas_->snapshot = std::move(snapshot);
        canvas_->update();
    }
    const auto send = [this](int key, bool release) { return keyboard_.key(key, release); };
    if (keyboard_switch_->isChecked() && running_ && now - last_frame_ <= 250) {
        if (!output_.advance(config_.motions, active_, now, send,
                             [this](char16_t character, bool release) {
                                 return release || keyboard_.text(character);
                             })) {
            keyboard_switch_->setChecked(false);
            report("Keyboard output failed: unsupported key/text in the current X11 layout.");
        }
    } else {
        output_.cancel(send, [](char16_t, bool) { return true; });
        active_.fill(false);
    }
    for (int index = 0; index < inputs_->count(); ++index) {
        const QBrush background =
            flashed_[index] > now || active_[index] ? QColor("#267a54") : Qt::transparent;
        auto* item = inputs_->item(index);
        if (item->background() != background) {
            item->setBackground(background);
        }
    }
}
bool Window::ui_test() {
    Motion input;
    input.id = input.name = input.action = "test";
    input.steps.push_back({"step1", StepMode::Ordered});
    config_.motions.push_back(input);
    refresh_inputs();
    canvas_->type = ConstraintType::Trigger;
    const QPointF center(canvas_->width() / 2.0, canvas_->height() / 2.0);
    QMouseEvent click(QEvent::MouseButtonPress, center, center, Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    QApplication::sendEvent(canvas_, &click);
    if (config_.motions[0].steps[0].constraints.size() != 1) {
        return false;
    }
    ui::reassign_layer(config_.motions[0], 15, 16);
    if (config_.motions[0].steps[0].constraints[0].landmark != 16) {
        return false;
    }
    canvas_->landmark = 16;
    canvas_->type = ConstraintType::Required;
    const QPointF next = center + QPointF(40, 0);
    QMouseEvent required(QEvent::MouseButtonPress, next, next, Qt::LeftButton, Qt::LeftButton,
                         Qt::NoModifier);
    QApplication::sendEvent(canvas_, &required);
    QApplication::sendEvent(canvas_, &required);
    const auto& cells = config_.motions[0].steps[0].constraints;
    if (cells.size() != 2 || cells[0].type != ConstraintType::Required || cells[0].order != 1 ||
        cells[1].type != ConstraintType::Trigger || cells[1].order != 2) {
        return false;
    }
    apply_theme(false);
    apply_theme(true);
    log_dock_->show();
    log_dock_->hide();
    const auto json = serialize_configuration(config_);
    return parse_configuration(json) == config_ && canvas_->motion && inputs_->count() == 1;
}
} // namespace mig::linux_ui
