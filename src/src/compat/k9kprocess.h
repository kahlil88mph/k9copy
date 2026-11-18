// KProcess look-alike on QProcess: program()/operator<</setShellCommand.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9KPROCESS_H
#define K9KPROCESS_H

#include <QProcess>
#include <QStringList>

class KProcess : public QProcess
{
public:
    using OutputChannelMode = QProcess::ProcessChannelMode;

    explicit KProcess(QObject *parent = nullptr) : QProcess(parent) {}

    void setOutputChannelMode(OutputChannelMode m) { setProcessChannelMode(m); }

    void setShellCommand(const QString &cmd)
    {
        QProcess::setProgram(QStringLiteral("/bin/sh"));
        setArguments({QStringLiteral("-c"), cmd});
    }

    void setProgram(const QString &exe, const QStringList &args = QStringList())
    {
        QProcess::setProgram(exe);
        setArguments(args);
    }
    void setProgram(const QStringList &argv)
    {
        QProcess::setProgram(argv.value(0));
        setArguments(argv.mid(1));
    }

    // Program followed by its arguments, as KProcess::program() returned.
    QStringList program() const
    {
        QString p = QProcess::program();
        if (p.isEmpty())
            return arguments();
        return QStringList(p) + arguments();
    }

    void clearProgram()
    {
        QProcess::setProgram(QString());
        setArguments(QStringList());
    }

    KProcess &operator<<(const QString &arg)
    {
        if (QProcess::program().isEmpty())
            QProcess::setProgram(arg);
        else
            setArguments(arguments() << arg);
        return *this;
    }
    KProcess &operator<<(const char *arg) { return *this << QString::fromUtf8(arg); }
    KProcess &operator<<(const QStringList &args)
    {
        for (const QString &a : args)
            *this << a;
        return *this;
    }

    void start() { QProcess::start(); }
    int execute(int msecs = -1)
    {
        QProcess::start();
        waitForFinished(msecs);
        return exitCode();
    }
};

#endif
