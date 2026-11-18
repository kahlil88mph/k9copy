// Copyright: GPL-2.0-or-later, see COPYING.
#include "kxmlguiwindow.h"
#include "k9i18n.h"
#include "config.h"

#include <QApplication>
#include <QDomDocument>
#include <QEvent>
#include <QFile>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>

static QAction *stdAction(const QString &name, const QString &text, const char *icon, QKeySequence::StandardKey key,
                          const QObject *recv, const char *slot, QObject *collection)
{
    auto *a = new QAction(QIcon::fromTheme(QLatin1String(icon)), text, collection);
    a->setShortcut(key);
    if (recv && slot)
        QObject::connect(a, SIGNAL(triggered(bool)), recv, slot);
    if (auto *c = qobject_cast<KActionCollection *>(collection))
        c->addAction(name, a);
    return a;
}

QAction *KStandardAction::open(const QObject *r, const char *s, QObject *c)
{
    return stdAction(QStringLiteral("file_open"), i18n("&Open..."), "document-open", QKeySequence::Open, r, s, c);
}

QAction *KStandardAction::quit(const QObject *r, const char *s, QObject *c)
{
    return stdAction(QStringLiteral("file_quit"), i18n("&Quit"), "application-exit", QKeySequence::Quit, r, s, c);
}

QAction *KStandardAction::preferences(const QObject *r, const char *s, QObject *c)
{
    QAction *a = stdAction(QStringLiteral("options_configure"), i18n("&Configure K9Copy..."), "configure",
                           QKeySequence::Preferences, r, s, c);
    if (auto *col = qobject_cast<KActionCollection *>(c))
        col->addAction(QStringLiteral("settings_preferences"), a); // name used by k9copyui.rc
    a->setObjectName(QStringLiteral("options_configure"));
    return a;
}

KXmlGuiWindow::KXmlGuiWindow(QWidget *parent, Qt::WindowFlags f)
    : QMainWindow(parent, f), m_collection(new KActionCollection(this))
{
}

static QString menuTitle(const QDomElement &e)
{
    const QDomElement t = e.firstChildElement(QStringLiteral("text"));
    if (!t.isNull())
        return k9i18n::lookup(t.text().toUtf8().constData());
    const QString name = e.attribute(QStringLiteral("name"));
    if (name == QLatin1String("file")) return i18n("&File");
    if (name == QLatin1String("settings")) return i18n("&Settings");
    if (name == QLatin1String("help")) return i18n("&Help");
    return name;
}

void KXmlGuiWindow::buildMenu(QMenu *menu, const QDomElement &parent)
{
    for (QDomElement e = parent.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString tag = e.tagName();
        if (tag == QLatin1String("Action")) {
            if (QAction *a = m_collection->action(e.attribute(QStringLiteral("name"))))
                menu->addAction(a);
        } else if (tag == QLatin1String("Separator")) {
            menu->addSeparator();
        } else if (tag == QLatin1String("Menu")) {
            buildMenu(menu->addMenu(menuTitle(e)), e);
        }
    }
}

void KXmlGuiWindow::createGUI(const char *)
{
    QFile f(QStringLiteral(":/k9copyui.rc"));
    QDomDocument doc;
    if (!f.open(QIODevice::ReadOnly) || !doc.setContent(&f)) {
        qWarning("k9copy: cannot load embedded k9copyui.rc");
        return;
    }
    const QDomElement root = doc.documentElement();

    menuBar()->clear();
    for (QToolBar *tb : findChildren<QToolBar *>(Qt::FindDirectChildrenOnly)) {
        removeToolBar(tb);
        tb->deleteLater();
    }

    QMenu *settingsMenu = nullptr;
    const QDomElement mb = root.firstChildElement(QStringLiteral("MenuBar"));
    for (QDomElement m = mb.firstChildElement(QStringLiteral("Menu")); !m.isNull();
         m = m.nextSiblingElement(QStringLiteral("Menu"))) {
        QMenu *menu = menuBar()->addMenu(menuTitle(m));
        buildMenu(menu, m);
        if (m.attribute(QStringLiteral("name")) == QLatin1String("settings"))
            settingsMenu = menu;
        // Drop menus whose actions don't exist in this window.
        if (menu->actions().isEmpty() && m.attribute(QStringLiteral("name")) != QLatin1String("settings"))
            menuBar()->removeAction(menu->menuAction());
    }

    QList<QToolBar *> toolbars;
    for (QDomElement t = root.firstChildElement(QStringLiteral("ToolBar")); !t.isNull();
         t = t.nextSiblingElement(QStringLiteral("ToolBar"))) {
        const QDomElement text = t.firstChildElement(QStringLiteral("text"));
        auto *tb = addToolBar(text.isNull() ? i18n("Main Toolbar")
                                            : k9i18n::lookup(text.text().toUtf8().constData()));
        tb->setObjectName(t.attribute(QStringLiteral("name")));
        tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        for (QDomElement e = t.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == QLatin1String("Action")) {
                if (QAction *a = m_collection->action(e.attribute(QStringLiteral("name"))))
                    tb->addAction(a);
            } else if (e.tagName() == QLatin1String("Separator")) {
                tb->addSeparator();
            }
        }
        toolbars << tb;
    }

    if (!settingsMenu)
        settingsMenu = menuBar()->addMenu(i18n("&Settings"));
    if (m_toolbarMenu || m_statusbarMenu)
        settingsMenu->addSeparator();
    if (m_toolbarMenu)
        for (QToolBar *tb : toolbars)
            settingsMenu->addAction(tb->toggleViewAction());
    if (m_statusbarMenu) {
        QAction *sb = settingsMenu->addAction(i18n("Show St&atusbar"));
        sb->setCheckable(true);
        sb->setChecked(true);
        connect(sb, &QAction::toggled, statusBar(), &QStatusBar::setVisible);
    }

    QMenu *help = menuBar()->addMenu(i18n("&Help"));
    help->addAction(QIcon::fromTheme(QStringLiteral("k9copy")), i18n("&About K9Copy"), this, [this] {
        QMessageBox::about(this, i18n("About K9Copy"),
                           i18n("<h3>K9Copy %1</h3><p>DVD backup tool.</p>"
                                "<p>Originally by Jean-Michel Petit; k9copy-reloaded by Pauline123.<br>"
                                "Qt 6 port without KDE Frameworks.</p><p>License: GPL v2 or later</p>",
                                QStringLiteral(VERSION)));
    });
    help->addAction(i18n("About &Qt"), qApp, &QApplication::aboutQt);

    if (m_autoSave && !m_restored) {
        QSettings s;
        restoreGeometry(s.value(objectName() + QStringLiteral("/geometry")).toByteArray());
        restoreState(s.value(objectName() + QStringLiteral("/state")).toByteArray());
        m_restored = true;
    }
}

void KXmlGuiWindow::setAutoSaveSettings()
{
    if (m_autoSave)
        return;
    m_autoSave = true;
    if (objectName().isEmpty())
        setObjectName(QString::fromLatin1(metaObject()->className()));
    installEventFilter(this);
    QSettings s;
    restoreGeometry(s.value(objectName() + QStringLiteral("/geometry")).toByteArray());
    restoreState(s.value(objectName() + QStringLiteral("/state")).toByteArray());
    m_restored = true;
}

void KXmlGuiWindow::saveAutoSaveSettings()
{
    QSettings s;
    s.setValue(objectName() + QStringLiteral("/geometry"), saveGeometry());
    s.setValue(objectName() + QStringLiteral("/state"), saveState());
}

bool KXmlGuiWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o == this && e->type() == QEvent::Close && m_autoSave)
        saveAutoSaveSettings();
    return QMainWindow::eventFilter(o, e);
}
