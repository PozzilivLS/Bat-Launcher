#include "rotatablewidget.h"

#include <QPainter>

RotatableWidget::RotatableWidget(QWidget *parent)
    : QWidget(parent)
{}

RotatableWidget::~RotatableWidget() {}

void RotatableWidget::setRotation(qreal angle) {
  if (rotation_ == angle) {
    return;
  }

  rotation_ = angle;
  update();
}

void RotatableWidget::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  p.translate(width() / 2, height() / 2);
  p.rotate(rotation_);
  p.translate(-width() / 2, -height() / 2);

  QPixmap pmap(":/BatActivator/Images/bat.png");
  p.drawPixmap((width() - pmap.width()) / 2, (height() - pmap.height()) / 2,
               pmap);

  QWidget::paintEvent(event);
}

