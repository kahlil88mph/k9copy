// Copyright: GPL-2.0-or-later, see COPYING.
#include "kpagedialog.h"
#include "k9i18n.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

// ---------------------------------------------------------------- KPageWidget

KPageWidget::KPageWidget(QWidget *parent)
    : QWidget(parent), m_list(new QListWidget(this)), m_header(new QLabel(this)), m_stack(new QStackedWidget(this))
{
    m_list->setViewMode(QListView::IconMode);
    m_list->setFlow(QListView::TopToBottom);
    m_list->setMovement(QListView::Static);
    m_list->setIconSize(QSize(32, 32));
    m_list->setSpacing(4);
    m_list->setMaximumWidth(130);
    m_list->setWordWrap(true);

    QFont f = m_header->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.2);
    m_header->setFont(f);

    auto *right = new QVBoxLayout;
    right->addWidget(m_header);
    right->addWidget(m_stack, 1);
    auto *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_list);
    l->addLayout(right, 1);

    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row < 0 || row >= m_items.size())
            return;
        KPageWidgetItem *before = currentPage();
        m_stack->setCurrentIndex(row);
        updateHeader();
        if (before != m_items[row])
            Q_EMIT currentPageChanged(m_items[row], before);
    });
}

void KPageWidget::setFaceType(FaceType f)
{
    m_list->setVisible(f != Plain);
}

KPageWidgetItem *KPageWidget::addPage(QWidget *widget, const QString &name)
{
    auto *item = new KPageWidgetItem(widget, name);
    addPage(item);
    return item;
}

void KPageWidget::addPage(KPageWidgetItem *item)
{
    item->setParent(this);
    m_items << item;
    m_stack->addWidget(item->widget());
    auto *li = new QListWidgetItem(item->icon(), item->name(), m_list);
    li->setTextAlignment(Qt::AlignHCenter);
    connect(item, &KPageWidgetItem::changed, this, [this, item, li] {
        li->setIcon(item->icon());
        li->setText(item->name());
        if (item == currentPage())
            updateHeader();
    });
    if (m_items.size() == 1)
        m_list->setCurrentRow(0);
}

KPageWidgetItem *KPageWidget::currentPage() const
{
    const int i = m_stack->currentIndex();
    return (i >= 0 && i < m_items.size()) ? m_items[i] : nullptr;
}

void KPageWidget::setCurrentPage(KPageWidgetItem *item)
{
    const int i = m_items.indexOf(item);
    if (i >= 0)
        m_list->setCurrentRow(i);
}

void KPageWidget::updateHeader()
{
    KPageWidgetItem *p = currentPage();
    m_header->setText(p ? p->header() : QString());
}

// ---------------------------------------------------------------- KPageDialog

KPageDialog::KPageDialog(QWidget *parent) : QDialog(parent), m_pages(new KPageWidget(this))
{
    init();
}

KPageDialog::KPageDialog(KPageWidget *widget, QWidget *parent) : QDialog(parent), m_pages(widget)
{
    m_pages->setParent(this);
    init();
}

void KPageDialog::init()
{
    m_buttons = new QDialogButtonBox(this);
    auto *l = new QVBoxLayout(this);
    l->addWidget(m_pages, 1);
    l->addWidget(m_buttons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// ---------------------------------------------------------------- KAssistantDialog

KAssistantDialog::KAssistantDialog(QWidget *parent) : KPageDialog(parent) { setupButtons(); }

KAssistantDialog::KAssistantDialog(KPageWidget *widget, QWidget *parent) : KPageDialog(widget, parent)
{
    setupButtons();
}

void KAssistantDialog::setupButtons()
{
    setFaceType(Plain);
    m_back = m_buttons->addButton(i18n("< &Back"), QDialogButtonBox::ActionRole);
    m_next = m_buttons->addButton(i18n("&Next >"), QDialogButtonBox::ActionRole);
    m_finish = m_buttons->addButton(i18n("&Finish"), QDialogButtonBox::AcceptRole);
    m_buttons->addButton(QDialogButtonBox::Cancel);
    m_back->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_next->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    m_finish->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    connect(m_back, &QPushButton::clicked, this, [this] { back(); });
    connect(m_next, &QPushButton::clicked, this, [this] { next(); });
    connect(m_pages, &KPageWidget::currentPageChanged, this, [this] { updateButtons(); });
}

int KAssistantDialog::step(int from, int dir) const
{
    const auto &pages = m_pages->pages();
    for (int i = from + dir; i >= 0 && i < pages.size(); i += dir)
        if (isAppropriate(pages[i]))
            return i;
    return -1;
}

void KAssistantDialog::back()
{
    const int i = step(m_pages->pages().indexOf(currentPage()), -1);
    if (i >= 0)
        setCurrentPage(m_pages->pages()[i]);
}

void KAssistantDialog::next()
{
    const int i = step(m_pages->pages().indexOf(currentPage()), +1);
    if (i >= 0)
        setCurrentPage(m_pages->pages()[i]);
}

void KAssistantDialog::setAppropriate(KPageWidgetItem *page, bool appropriate)
{
    m_inappropriate.removeAll(page);
    if (!appropriate)
        m_inappropriate << page;
    updateButtons();
}

void KAssistantDialog::setValid(KPageWidgetItem *page, bool valid)
{
    m_invalid.removeAll(page);
    if (!valid)
        m_invalid << page;
    updateButtons();
}

void KAssistantDialog::showEvent(QShowEvent *e)
{
    updateButtons();
    KPageDialog::showEvent(e);
}

void KAssistantDialog::updateButtons()
{
    const int cur = m_pages->pages().indexOf(currentPage());
    const bool valid = currentPage() && isValid(currentPage());
    const bool hasNext = step(cur, +1) >= 0;
    m_back->setEnabled(step(cur, -1) >= 0);
    m_next->setEnabled(hasNext && valid);
    m_finish->setEnabled(!hasNext && valid);
    m_next->setDefault(hasNext);
    m_finish->setDefault(!hasNext);
}
