// Small KColorButton replacement (keeps the class name for the .ui files).
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9_KCOLORBUTTON_H
#define K9_KCOLORBUTTON_H

#include <QColor>
#include <QPushButton>

class KColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KColorButton(QWidget *parent = nullptr);
    QColor color() const { return m_color; }
    void setColor(const QColor &c);

Q_SIGNALS:
    void changed(const QColor &);

private Q_SLOTS:
    void choose();

private:
    QColor m_color = Qt::black;
};

#endif
