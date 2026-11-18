// Minimal KPageWidget / KPageDialog / KAssistantDialog replacements built on
// QListWidget + QStackedWidget + QDialogButtonBox. Only what k9copy uses.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9_KPAGEDIALOG_H
#define K9_KPAGEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;
class QStackedWidget;

class KPageWidgetItem : public QObject
{
    Q_OBJECT
public:
    KPageWidgetItem(QWidget *widget, const QString &name = QString())
        : m_widget(widget), m_name(name), m_header(name) {}
    QWidget *widget() const { return m_widget; }
    QString name() const { return m_name; }
    void setName(const QString &n) { m_name = n; Q_EMIT changed(); }
    QString header() const { return m_header; }
    void setHeader(const QString &h) { m_header = h; Q_EMIT changed(); }
    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon &i) { m_icon = i; Q_EMIT changed(); }
Q_SIGNALS:
    void changed();
private:
    QPointer<QWidget> m_widget;
    QString m_name, m_header;
    QIcon m_icon;
};

class KPageWidget : public QWidget
{
    Q_OBJECT
public:
    enum FaceType { Auto, Plain, List, Tree, Tabbed };
    explicit KPageWidget(QWidget *parent = nullptr);

    void setFaceType(FaceType f);
    KPageWidgetItem *addPage(QWidget *widget, const QString &name);
    void addPage(KPageWidgetItem *item);
    KPageWidgetItem *currentPage() const;
    void setCurrentPage(KPageWidgetItem *item);
    const QList<KPageWidgetItem *> &pages() const { return m_items; }

Q_SIGNALS:
    void currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before);

private:
    void updateHeader();
    QListWidget *m_list;
    QLabel *m_header;
    QStackedWidget *m_stack;
    QList<KPageWidgetItem *> m_items;
};

class KPageDialog : public QDialog
{
    Q_OBJECT
public:
    enum FaceType { Auto = KPageWidget::Auto, Plain = KPageWidget::Plain, List = KPageWidget::List,
                    Tree = KPageWidget::Tree, Tabbed = KPageWidget::Tabbed };
    explicit KPageDialog(QWidget *parent = nullptr);
    explicit KPageDialog(KPageWidget *widget, QWidget *parent = nullptr);

    void setFaceType(FaceType f) { m_pages->setFaceType(KPageWidget::FaceType(f)); }
    void setStandardButtons(QDialogButtonBox::StandardButtons b) { m_buttons->setStandardButtons(b); }
    QDialogButtonBox *buttonBox() const { return m_buttons; }
    QPushButton *button(QDialogButtonBox::StandardButton b) const { return m_buttons->button(b); }
    KPageWidgetItem *addPage(QWidget *widget, const QString &name) { return m_pages->addPage(widget, name); }
    void addPage(KPageWidgetItem *item) { m_pages->addPage(item); }
    KPageWidgetItem *currentPage() const { return m_pages->currentPage(); }
    void setCurrentPage(KPageWidgetItem *item) { m_pages->setCurrentPage(item); }
    KPageWidget *pageWidget() const { return m_pages; }

private:
    void init();
protected:
    KPageWidget *m_pages;
    QDialogButtonBox *m_buttons;
};

class KAssistantDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit KAssistantDialog(QWidget *parent = nullptr);
    explicit KAssistantDialog(KPageWidget *widget, QWidget *parent = nullptr);

    void setAppropriate(KPageWidgetItem *page, bool appropriate);
    bool isAppropriate(KPageWidgetItem *page) const { return !m_inappropriate.contains(page); }
    void setValid(KPageWidgetItem *page, bool valid);
    bool isValid(KPageWidgetItem *page) const { return !m_invalid.contains(page); }

public Q_SLOTS:
    virtual void back();
    virtual void next();

protected:
    void showEvent(QShowEvent *e) override;

private:
    void setupButtons();
    void updateButtons();
    int step(int from, int dir) const;
    QPushButton *m_back = nullptr;
    QPushButton *m_next = nullptr;
    QPushButton *m_finish = nullptr;
    QList<KPageWidgetItem *> m_inappropriate;
    QList<KPageWidgetItem *> m_invalid;
};

#endif
