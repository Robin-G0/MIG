#include "canvas.hpp"
#include "../apps/authoring.hpp"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
namespace mig::linux_ui {
Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(500, 420);
}
QRectF Canvas::area() const {
    const auto size = std::min(width(), height()) - 32;
    return {(width() - size) / 2.0, (height() - size) / 2.0, double(size), double(size)};
}
const Grid& Canvas::basis() const {
    return motion && motion->space == CoordinateSpace::Calibrated ? snapshot.reference_grid
                                                                  : snapshot.grid;
}
QPointF Canvas::project(Vec2 local) const {
    const auto rectangle = area();
    if (full_grid || !basis().valid) {
        return {
            rectangle.right() - (local.x - Grid::min_cell) / Grid::cell_count * rectangle.width(),
            rectangle.top() + (local.y - Grid::min_cell) / Grid::cell_count * rectangle.height()};
    }
    const auto point = basis().metric(local);
    const auto image = snapshot.image.size().scaled(size(), Qt::KeepAspectRatio);
    return {(width() - image.width()) / 2.0 + (1 - point.x / snapshot.body.aspect) * image.width(),
            (height() - image.height()) / 2.0 + point.y * image.height()};
}
void Canvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), palette().base());
    if (!full_grid && !snapshot.image.isNull()) {
        const auto image = snapshot.image.mirrored(true, false).scaled(size(), Qt::KeepAspectRatio);
        painter.drawImage(QPoint((width() - image.width()) / 2, (height() - image.height()) / 2),
                          image);
    }
    if (show_grid || (read_only && motion)) {
        painter.setPen(QColor(110, 115, 125, 130));
        if (show_grid) {
            for (int index = Grid::min_cell; index <= Grid::max_cell; ++index) {
                painter.drawLine(project({float(index), Grid::min_cell}),
                                 project({float(index), Grid::max_cell}));
                painter.drawLine(project({Grid::min_cell, float(index)}),
                                 project({Grid::max_cell, float(index)}));
            }
        }
        if (motion) {
            const auto& progress =
                ui::displayed_progress(snapshot.progress, snapshot.mirrored_progress);
            std::size_t constraint_index = 0;
            const auto cells = [&](const auto& constraints) {
                for (const auto& cell : constraints) {
                    static const QColor colors[]{QColor("#36c98e"), QColor("#ed6372"),
                                                 QColor("#efc650"), QColor("#b886ed")};
                    const auto status = constraint_index < progress.constraints.size()
                                            ? progress.constraints[constraint_index]
                                            : ConstraintStatus::Missing;
                    ++constraint_index;
                    const bool reacting = read_only && (status == ConstraintStatus::Validated ||
                                                        status == ConstraintStatus::Triggered ||
                                                        status == ConstraintStatus::Holding);
                    const bool invalid = read_only && (status == ConstraintStatus::Forbidden ||
                                                       status == ConstraintStatus::FingerInvalid ||
                                                       status == ConstraintStatus::HandMissing ||
                                                       status == ConstraintStatus::SignMismatch ||
                                                       status == ConstraintStatus::LandmarkLost);
                    auto color = invalid    ? QColor("#ed6372")
                                 : reacting ? QColor("#ffffff")
                                            : colors[int(cell.type)];
                    painter.setPen(QPen(color, 2));
                    color.setAlpha(cell.landmark == landmark ? 90 : 25);
                    painter.setBrush(color);
                    auto region = cell.cell;
                    if (read_only && progress.mirrored) {
                        region.x = 9 - region.x - region.width;
                    }
                    const QPolygonF polygon{
                        project({float(region.x), float(region.y)}),
                        project({float(region.x + region.width), float(region.y)}),
                        project({float(region.x + region.width), float(region.y + region.height)}),
                        project({float(region.x), float(region.y + region.height)})};
                    painter.drawPolygon(polygon);
                    painter.drawText(polygon.boundingRect(), Qt::AlignCenter,
                                     cell.order ? QString::number(cell.order) : QString());
                }
            };
            cells(motion->constraints);
            for (const auto& step : motion->steps) {
                cells(step.constraints);
            }
        }
    }
    if (show_dots && basis().valid) {
        painter.setBrush(QColor("#42dba3"));
        painter.setPen(Qt::NoPen);
        for (const auto& point : snapshot.body.points) {
            if (point.confidence >= .6f) {
                painter.drawEllipse(project(basis().local({point.position.x * snapshot.body.aspect,
                                                           point.position.y})),
                                    4, 4);
            }
        }
    }
    if (show_hands && (read_only || show_dots) && basis().valid) {
#ifdef MIG_NATIVE_HANDS
        painter.setPen(QPen(QColor("#efc650"), 2));
        for (std::size_t index = 0; index < snapshot.hands.count; ++index) {
            const auto& hand = snapshot.hands.hands[index];
            for (int base : {1, 5, 9, 13, 17}) {
                auto previous = hand.points[0];
                for (int joint = base; joint < base + 4; ++joint) {
                    const auto point = hand.points[joint];
                    painter.drawLine(
                        project(basis().local({previous.x * snapshot.body.aspect, previous.y})),
                        project(basis().local({point.x * snapshot.body.aspect, point.y})));
                    previous = point;
                }
            }
        }
#endif
    }
}
#ifndef MIG_CONTROLLER
void Canvas::paint_cell(QPointF point) {
    if (read_only || !motion || !full_grid) {
        return;
    }
    auto previous = *motion;
    try {
        edit_cell(point);
    } catch (const std::exception& error) {
        *motion = std::move(previous);
        update();
        if (failed) {
            failed(error.what());
        }
    }
}
void Canvas::edit_cell(QPointF point) {
    if (read_only || !motion || !full_grid) {
        return;
    }
    const auto rectangle = area();
    const int x = int(std::floor(Grid::min_cell + (rectangle.right() - point.x()) /
                                                      rectangle.width() * Grid::cell_count));
    const int y = int(std::floor(Grid::min_cell + (point.y() - rectangle.top()) /
                                                      rectangle.height() * Grid::cell_count));
    if (x < Grid::min_cell || x >= Grid::max_cell || y < Grid::min_cell || y >= Grid::max_cell) {
        return;
    }
    if (motion->steps.empty()) {
        motion->steps.push_back({"step1", StepMode::Ordered});
    }
    auto& cells = motion->steps[0].constraints;
    if (erase) {
        std::erase_if(cells, [&](const auto& cell) {
            return cell.landmark == landmark && cell.cell.x == x && cell.cell.y == y;
        });
    } else {
        SpatialConstraint brush;
        brush.id = "cell" + std::to_string(cells.size() + 1);
        unsigned serial = unsigned(cells.size() + 1);
        while (std::any_of(cells.begin(), cells.end(),
                           [&](const auto& cell) { return cell.id == brush.id; })) {
            brush.id = "cell" + std::to_string(++serial);
        }
        brush.landmark = landmark;
        brush.type = type;
        if (!order) {
            for (const auto& cell : cells) {
                if (cell.landmark == landmark && cell.type == type && cell.cell.x == x &&
                    cell.cell.y == y) {
                    brush.order = cell.order;
                    break;
                }
            }
        }
        if (is_trigger(type) && !order) {
            for (auto& cell : cells) {
                if (!brush.order && cell.landmark == landmark && cell.type == type) {
                    brush.order = cell.order;
                    break;
                }
            }
        }
        if (type == ConstraintType::Interaction) {
            interaction.hand = landmark == 16 ? HandSide::Right : HandSide::Left;
            brush.interaction = interaction;
        }
        brush.order = order ? order : brush.order;
        if (!brush.order) {
            for (const auto& cell : cells) {
                if (cell.landmark == landmark && cell.type != ConstraintType::Forbidden &&
                    (type != ConstraintType::Required || !is_trigger(cell.type))) {
                    brush.order = std::max(brush.order, cell.order);
                }
            }
            ++brush.order;
            if (type == ConstraintType::Required && !order) {
                for (auto& cell : cells) {
                    if (cell.landmark == landmark && is_trigger(cell.type) &&
                        cell.order >= brush.order) {
                        ++cell.order;
                    }
                }
            }
        }
        if (type == ConstraintType::Forbidden) {
            brush.order = 0;
        }
        ui::pencil(cells, brush, x, y, true);
    }
    update();
    if (changed) {
        changed();
    }
}
#endif
void Canvas::mousePressEvent(QMouseEvent* event) {
#ifndef MIG_CONTROLLER
    if (event->button() == Qt::LeftButton) {
        paint_cell(event->position());
    }
#else
    QWidget::mousePressEvent(event);
#endif
}
void Canvas::mouseMoveEvent(QMouseEvent* event) {
#ifndef MIG_CONTROLLER
    if (event->buttons() & Qt::LeftButton) {
        paint_cell(event->position());
    }
#else
    QWidget::mouseMoveEvent(event);
#endif
}
} // namespace mig::linux_ui
