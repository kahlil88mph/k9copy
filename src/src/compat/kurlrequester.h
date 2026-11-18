// Small KUrlRequester replacement: line edit + browse button.
// Keeps the class name so the existing .ui files load unchanged.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9_KURLREQUESTER_H
#define K9_KURLREQUESTER_H

#include <QFileDialog>
#include <QFlags>
#include <QLineEdit>
#include <QUrl>
#include <QWidget>

class QToolButton;

namespace KFile {
enum Mode { File = 1, Directory = 2, Files = 4, ExistingOnly = 8, LocalOnly = 16 };
Q_DECLARE_FLAGS(Modes, Mode)
}
Q_DECLARE_OPERATORS_FOR_FLAGS(KFile::Modes)

class KUrlRequester : public QWidget
{
    Q_OBJECT
public:
    explicit KUrlRequester(QWidget *parent = nullptr);

    void setMode(KFile::Modes mode) { m_mode = mode; }
    // Replaces fileDialog()->setAcceptMode(): forces an open or save dialog.
    void setAcceptMode(QFileDialog::AcceptMode m) { m_acceptMode = m; m_acceptModeSet = true; }
    KFile::Modes mode() const { return m_mode; }
    // KDE filter syntax: "*.iso|Iso Image\n*.mpg|Mpeg" (one pattern|label per line)
    void setFilter(const QString &filter);

    QUrl url() const;
    void setUrl(const QUrl &url);
    QString text() const { return m_edit->text(); }
    void setText(const QString &t) { m_edit->setText(t); }
    QLineEdit *lineEdit() const { return m_edit; }

Q_SIGNALS:
    void textChanged(const QString &);
    void returnPressed();
    void urlSelected(const QUrl &);

private Q_SLOTS:
    void browse();

private:
    QLineEdit *m_edit;
    QToolButton *m_button;
    KFile::Modes m_mode = KFile::File;
    QString m_qtFilter;
    QFileDialog::AcceptMode m_acceptMode = QFileDialog::AcceptOpen;
    bool m_acceptModeSet = false;
};

#endif
