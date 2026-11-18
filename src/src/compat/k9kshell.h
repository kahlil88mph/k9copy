// KShell::quoteArg replacement (POSIX sh quoting).
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9KSHELL_H
#define K9KSHELL_H
#include <QString>
#include <QStringList>
#include <QProcess>

namespace KShell {
inline QString quoteArg(const QString &arg)
{
    if (arg.isEmpty())
        return QStringLiteral("''");
    bool safe = true;
    for (QChar c : arg) {
        if (!(c.isLetterOrNumber() || QStringLiteral("_-./=:,+@%").contains(c))) {
            safe = false;
            break;
        }
    }
    if (safe)
        return arg;
    QString q = arg;
    q.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + q + QLatin1Char('\'');
}
inline QStringList splitArgs(const QString &cmd) { return QProcess::splitCommand(cmd); }
}
#endif
