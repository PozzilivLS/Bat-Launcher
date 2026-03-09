#pragma once

#include <QWidget>

class RotatableWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

 public:
  RotatableWidget(QWidget *parent = nullptr);
  ~RotatableWidget();

  qreal rotation() const { return rotation_; };
  void setRotation(qreal angle);

 protected:
  void paintEvent(QPaintEvent *event) override;

 private:
  qreal rotation_ = 0;
};
