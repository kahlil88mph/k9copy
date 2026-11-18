// Stand-in for KGuiItem/KStandardGuiItem: standard button text + theme icon.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9GUIITEM_H
#define K9GUIITEM_H
#include "k9i18n.h"
#include <QAbstractButton>
#include <QIcon>

struct KGuiItem {
    QString text;
    QString iconName;
    static void assign(QAbstractButton *b, const KGuiItem &item)
    {
        b->setText(item.text);
        b->setIcon(QIcon::fromTheme(item.iconName));
    }
};

namespace KStandardGuiItem {
inline KGuiItem ok()     { return {i18n("&OK"), QStringLiteral("dialog-ok")}; }
inline KGuiItem cancel() { return {i18n("&Cancel"), QStringLiteral("dialog-cancel")}; }
inline KGuiItem stop()   { return {i18n("&Stop"), QStringLiteral("process-stop")}; }
inline KGuiItem add()    { return {i18n("Add"), QStringLiteral("list-add")}; }
inline KGuiItem remove() { return {i18n("Remove"), QStringLiteral("list-remove")}; }
inline KGuiItem reset()  { return {i18n("Reset"), QStringLiteral("edit-undo")}; }
inline KGuiItem close()  { return {i18n("&Close"), QStringLiteral("window-close")}; }
}
#endif
