// Minimal KXmlGuiWindow replacement: a QMainWindow whose menus and toolbars
// are built from the original kxmlgui .rc file (embedded as :/k9copyui.rc).
// Shortcut and toolbar editors are not provided.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9_KXMLGUIWINDOW_H
#define K9_KXMLGUIWINDOW_H

#include <QAction>
#include <QHash>
#include <QMainWindow>
#include <QPointer>

class QDomElement;
class QMenu;

class KActionCollection : public QObject
{
    Q_OBJECT
public:
    explicit KActionCollection(QObject *parent) : QObject(parent) {}
    QAction *addAction(const QString &name, QAction *action)
    {
        if (action) {
            action->setObjectName(name);
            m_actions.insert(name, action);
        }
        return action;
    }
    QAction *action(const QString &name) const { return m_actions.value(name); }
private:
    QHash<QString, QPointer<QAction>> m_actions;
};

using KToggleAction = QAction; // callers call setCheckable(true)

namespace KStandardAction {
QAction *open(const QObject *recv, const char *slot, QObject *collection);
QAction *quit(const QObject *recv, const char *slot, QObject *collection);
QAction *preferences(const QObject *recv, const char *slot, QObject *collection);
// No shortcut/toolbar editors in this build: these return nullptr.
inline QAction *keyBindings(const QObject *, const char *, QObject *) { return nullptr; }
inline QAction *configureToolbars(const QObject *, const char *, QObject *) { return nullptr; }
}

class KXmlGuiWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit KXmlGuiWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    KActionCollection *actionCollection() const { return m_collection; }
    void setupGUI() { createGUI(nullptr); }
    void createGUI(const char *xmlfile);
    void setAutoSaveSettings();
    void setStandardToolBarMenuEnabled(bool on) { m_toolbarMenu = on; }
    void createStandardStatusBarAction() { m_statusbarMenu = true; }
    virtual bool queryClose() { return true; }
    void saveAutoSaveSettings();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void buildMenu(QMenu *menu, const QDomElement &e);
    KActionCollection *m_collection;
    bool m_toolbarMenu = false;
    bool m_statusbarMenu = false;
    bool m_autoSave = false;
    bool m_restored = false;
};

#endif
