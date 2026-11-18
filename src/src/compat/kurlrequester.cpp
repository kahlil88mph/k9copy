// Copyright: GPL-2.0-or-later, see COPYING.
#include "kurlrequester.h"
#include "k9i18n.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QToolButton>

KUrlRequester::KUrlRequester(QWidget *parent)
    : QWidget(parent), m_edit(new QLineEdit(this)), m_button(new QToolButton(this))
{
    auto *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(2);
    l->addWidget(m_edit);
    l->addWidget(m_button);
    m_button->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    m_button->setToolTip(i18n("Browse..."));
    setFocusProxy(m_edit);
    connect(m_edit, &QLineEdit::textChanged, this, &KUrlRequester::textChanged);
    connect(m_edit, &QLineEdit::returnPressed, this, &KUrlRequester::returnPressed);
    connect(m_button, &QToolButton::clicked, this, &KUrlRequester::browse);
}

void KUrlRequester::setFilter(const QString &filter)
{
    QStringList parts;
    for (const QString &line : filter.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const int bar = line.indexOf(QLatin1Char('|'));
        const QString patterns = (bar < 0 ? line : line.left(bar)).trimmed();
        QString label = bar < 0 ? patterns : line.mid(bar + 1).trimmed();
        // Labels in k9copy already end in "(*.ext)"; add patterns if not.
        if (!label.contains(QLatin1Char('(')))
            label += QStringLiteral(" (%1)").arg(patterns);
        parts << label;
    }
    m_qtFilter = parts.join(QStringLiteral(";;"));
}

QUrl KUrlRequester::url() const
{
    const QString t = m_edit->text();
    return t.isEmpty() ? QUrl() : QUrl::fromUserInput(t, QString(), QUrl::AssumeLocalFile);
}

void KUrlRequester::setUrl(const QUrl &url)
{
    m_edit->setText(url.isLocalFile() ? url.toLocalFile() : url.toString());
}

void KUrlRequester::browse()
{
    const QString start = m_edit->text();
    QString result;
    if (m_mode & KFile::Directory) {
        result = QFileDialog::getExistingDirectory(this, QString(), start);
    } else if (m_acceptModeSet ? m_acceptMode == QFileDialog::AcceptOpen : bool(m_mode & KFile::ExistingOnly)) {
        result = QFileDialog::getOpenFileName(this, QString(), start, m_qtFilter);
    } else {
        result = QFileDialog::getSaveFileName(this, QString(), start, m_qtFilter);
    }
    if (result.isEmpty())
        return;
    m_edit->setText(result);
    Q_EMIT urlSelected(QUrl::fromLocalFile(result));
}
