// Copyright: GPL-2.0-or-later, see COPYING.
#include "kiconloader.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>

#ifndef K9_DATADIR
#define K9_DATADIR "/usr/share/k9copy"
#endif

static QStringList privateIconDirs()
{
    // Installed location, then the source tree next to a build dir (dev runs).
    return {QStringLiteral(K9_DATADIR "/icons"),
            QCoreApplication::applicationDirPath() + QStringLiteral("/../src/icons")};
}

QIcon k9Icon(const QString &name)
{
    static QHash<QString, QIcon> cache;
    auto it = cache.constFind(name);
    if (it != cache.constEnd())
        return *it;

    QIcon icon = QIcon::fromTheme(name);
    if (icon.isNull()) {
        for (const QString &dir : privateIconDirs()) {
            for (int size : {16, 22, 32, 48}) {
                const QString f = QStringLiteral("%1/%2-actions-%3.png").arg(dir).arg(size).arg(name);
                if (QFileInfo::exists(f))
                    icon.addFile(f, QSize(size, size));
            }
            if (!icon.isNull())
                break;
        }
    }
    cache.insert(name, icon);
    return icon;
}
