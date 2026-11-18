// Icon lookup without KIconThemes: freedesktop theme first, then the
// k9copy private icons (icons/NN-actions-<name>.png).
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9_KICONLOADER_H
#define K9_KICONLOADER_H

#include <QIcon>
#include <QPixmap>
#include <QString>

QIcon k9Icon(const QString &name);

inline QPixmap SmallIcon(const QString &name) { return k9Icon(name).pixmap(16, 16); }
inline QPixmap BarIcon(const QString &name) { return k9Icon(name).pixmap(22, 22); }
inline QPixmap DesktopIcon(const QString &name) { return k9Icon(name).pixmap(32, 32); }

class KIconLoader
{
public:
    enum Group { Desktop, Toolbar, MainToolbar, Small, Panel, Dialog, User };
    static KIconLoader *global()
    {
        static KIconLoader loader;
        return &loader;
    }
    QIcon loadIconSet(const QString &name, Group = Toolbar, int = 0) const { return k9Icon(name); }
    QPixmap loadIcon(const QString &name, Group g = Toolbar, int size = 0) const
    {
        if (size <= 0)
            size = (g == Desktop) ? 32 : (g == Small ? 16 : 22);
        return k9Icon(name).pixmap(size, size);
    }
};

#endif
