#pragma once
#include "capture.hpp"
#include <QWidget>
#include <functional>
namespace mig::linux_ui {
class Canvas : public QWidget {
public:
    explicit Canvas(QWidget* parent = nullptr);
    Snapshot snapshot;
    Motion* motion{};
    int landmark{15}, order{};
    ConstraintType type{ConstraintType::Required};
    Interaction interaction;
    bool erase{}, show_grid{true}, show_dots{true}, full_grid{true};
    bool show_hands{true}, read_only{};
    std::function<void()> changed;
    std::function<void(const std::string&)> failed;

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    void paint_cell(QPointF point);
    void edit_cell(QPointF point);
    QPointF project(Vec2 local) const;
    const Grid& basis() const;
    QRectF area() const;
};
} // namespace mig::linux_ui
