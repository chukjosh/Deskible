#ifndef MODERNSWITCH_H
#define MODERNSWITCH_H

#include <QAbstractButton>
#include <QPainter>
#include <QVariantAnimation>

class ModernSwitch : public QAbstractButton {
    Q_OBJECT
public:
    explicit ModernSwitch(QWidget *parent = nullptr) : QAbstractButton(parent) {
        setCheckable(true);
        setFixedSize(44, 22);
        m_thumbPos = isChecked() ? 24 : 2;
    }
    
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        
        QRect r = rect().adjusted(1, 1, -1, -1);
        QColor trackCol = isChecked() ? QColor("#5555FF") : QColor(100, 100, 100, 100);
        
        p.setPen(Qt::NoPen);
        p.setBrush(trackCol);
        p.drawRoundedRect(r, 11, 11);
        
        p.setBrush(Qt::white);
        p.drawEllipse(m_thumbPos, 3, 16, 16);
    }

    void nextCheckState() override {
        QAbstractButton::nextCheckState();
        animateThumb(isChecked());
    }

protected:
    void animateThumb(bool checked) {
        QVariantAnimation *anim = new QVariantAnimation(this);
        anim->setDuration(200);
        anim->setStartValue(m_thumbPos);
        anim->setEndValue(checked ? 24 : 2);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QVariantAnimation::valueChanged, [this](const QVariant &v){
            m_thumbPos = v.toInt();
            update();
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    int thumbPos() const { return m_thumbPos; }
    void setThumbPos(int p) { m_thumbPos = p; update(); }

private:
    int m_thumbPos;
};

#endif // MODERNSWITCH_H
