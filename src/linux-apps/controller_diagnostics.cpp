#include "controller_window.hpp"
#include <QAction>
#include <QApplication>
#include <QMouseEvent>
#include <QTemporaryDir>
#include <mig/format/configuration.hpp>

namespace mig::linux_ui {
bool ControllerWindow::ui_test() {
    QTemporaryDir temporary;
    if (!temporary.isValid()) {
        return false;
    }
    profiles_ = controller::Profiles(temporary.path().toStdString() + "/profiles");
    Configuration sample;
    Motion motion;
    motion.id = motion.name = motion.action = "test";
    SpatialConstraint trigger;
    trigger.id = "trigger";
    trigger.type = ConstraintType::Trigger;
    trigger.landmark = 15;
    trigger.cell = {4, 3, 1, 1};
    motion.steps.push_back({"step1", StepMode::Visited});
    motion.steps[0].constraints.push_back(trigger);
    sample.motions.push_back(motion);
    const auto source = temporary.path().toStdString() + "/test.json";
    save_configuration(sample, source);
    load(source);
    load(source);
    activate_profile(0);
    if (profiles_.selected() != 0 || bindings_->count() != 1 ||
        !bindings_->item(0)->text().contains("test")) {
        return false;
    }
    verification_action_->setChecked(true);
    if (!verification_ || !camera_action_->isChecked() || !canvas_->read_only ||
        canvas_->motion != &config_.motions[0]) {
        return false;
    }
    QApplication::processEvents();
    grab().save(QString::fromStdString(runtime_ + "/controller-verification.png"));
    const auto before = config_;
    const QPointF center(canvas_->width() / 2.0, canvas_->height() / 2.0);
    QMouseEvent click(QEvent::MouseButtonPress, center, center, Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    QApplication::sendEvent(canvas_, &click);
    if (config_ != before) {
        return false;
    }
    set_compact(true);
    if (!compact_ || capture_.preview_enabled || panel_->isVisible()) {
        return false;
    }
    QApplication::processEvents();
    grab().save(QString::fromStdString(runtime_ + "/controller-compact.png"));
    set_compact(false);
    verification_action_->setChecked(false);
    camera_action_->setChecked(false);
    apply_theme(false);
    QApplication::processEvents();
    grab().save(QString::fromStdString(runtime_ + "/controller-bindings.png"));
    controller::Profiles restored(temporary.path().toStdString() + "/profiles");
    restored.restore();
    return !verification_ && !capture_.preview_enabled && restored.selected() == 0 &&
           restored.load(0) == sample;
}
} // namespace mig::linux_ui
