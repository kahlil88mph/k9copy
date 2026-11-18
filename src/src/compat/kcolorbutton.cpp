// Copyright: GPL-2.0-or-later, see COPYING.
#include "kcolorbutton.h"
#include <QColorDialog>
#include <QPixmap>

KColorButton::KColorButton(QWidget *parent) : QPushButton(parent)
{
    setColor(m_color);
    connect(this, &QPushButton::clicked, this, &KColorButton::choose);
}

void KColorButton::setColor(const QColor &c)
{
    const bool differs = c != m_color;
    m_color = c;
    QPixmap swatch(32, 16);
    swatch.fill(c);
    setIcon(QIcon(swatch));
    setIconSize(swatch.size());
    if (differs)
        Q_EMIT changed(c);
}

void KColorButton::choose()
{
    QColor c = QColorDialog::getColor(m_color, this);
    if (c.isValid())
        setColor(c);
}
