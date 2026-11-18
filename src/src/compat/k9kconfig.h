// Thin KConfig/KConfigGroup look-alikes on top of QSettings (INI format).
// The file stays at ~/.config/K9Copy, so settings written by the KDE builds
// are picked up. Only the API surface k9copy uses is provided.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9KCONFIG_H
#define K9KCONFIG_H

#include <QColor>
#include <QFont>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <memory>

class KConfigGroup
{
public:
    KConfigGroup() = default;
    KConfigGroup(QSettings *s, const QString &name) : m_s(s), m_name(name) {}

    QString name() const { return m_name; }

    QString readEntry(const QString &key, const char *def = "") const
    {
        return readEntry(key, QString::fromUtf8(def));
    }
    QString readEntry(const QString &key, const QString &def) const
    {
        QVariant v = raw(key);
        if (!v.isValid())
            return def;
        // INI values containing commas come back as lists.
        if (v.typeId() == QMetaType::QStringList)
            return v.toStringList().join(QLatin1Char(','));
        return v.toString();
    }
    QStringList readEntry(const QString &key, const QStringList &def) const
    {
        QVariant v = raw(key);
        if (!v.isValid())
            return def;
        if (v.typeId() == QMetaType::QStringList)
            return v.toStringList();
        QString s = v.toString();
        return s.isEmpty() ? QStringList() : s.split(QLatin1Char(','));
    }
    QColor readEntry(const QString &key, const QColor &def) const
    {
        QStringList l = readEntry(key, QStringList());
        if (l.size() >= 3)
            return QColor(l[0].trimmed().toInt(), l[1].trimmed().toInt(), l[2].trimmed().toInt(),
                          l.size() > 3 ? l[3].trimmed().toInt() : 255);
        if (l.size() == 1 && QColor::isValidColorName(l[0]))
            return QColor::fromString(l[0]);
        return def;
    }
    QFont readEntry(const QString &key, const QFont &def) const
    {
        QFont f;
        QString s = readEntry(key, QString());
        return (!s.isEmpty() && f.fromString(s)) ? f : def;
    }
    bool readEntry(const QString &key, bool def) const
    {
        QVariant v = raw(key);
        return v.isValid() ? (v.toString() == QLatin1String("true") || v.toInt() != 0) : def;
    }
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, T> readEntry(const QString &key, T def) const
    {
        QVariant v = raw(key);
        bool ok = false;
        T r = v.isValid() ? v.value<T>() : def;
        if (v.isValid()) {
            v.toString().toDouble(&ok);
            if (!ok)
                r = def;
        }
        return r;
    }

    QList<bool> readEntry(const QString &key, const QList<bool> &def) const
    {
        QStringList l = readEntry(key, QStringList());
        if (l.isEmpty())
            return def;
        QList<bool> r;
        for (const QString &e : l)
            r << (e.trimmed() == QLatin1String("true") || e.trimmed() == QLatin1String("1"));
        return r;
    }
    void writeEntry(const QString &key, const QList<bool> &v)
    {
        QStringList l;
        for (bool b : v)
            l << (b ? QStringLiteral("true") : QStringLiteral("false"));
        set(key, l);
    }

    void writeEntry(const QString &key, const QString &v) { set(key, v); }
    void writeEntry(const QString &key, const char *v) { set(key, QString::fromUtf8(v)); }
    void writeEntry(const QString &key, const QStringList &v) { set(key, v); }
    void writeEntry(const QString &key, bool v) { set(key, v ? QStringLiteral("true") : QStringLiteral("false")); }
    void writeEntry(const QString &key, const QColor &c)
    {
        set(key, QStringList{QString::number(c.red()), QString::number(c.green()), QString::number(c.blue())});
    }
    void writeEntry(const QString &key, const QFont &f) { set(key, f.toString()); }
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>> writeEntry(const QString &key, T v) { set(key, QVariant::fromValue(v)); }

private:
    QVariant raw(const QString &key) const
    {
        return m_s ? m_s->value(m_name + QLatin1Char('/') + key) : QVariant();
    }
    void set(const QString &key, const QVariant &v)
    {
        if (m_s)
            m_s->setValue(m_name + QLatin1Char('/') + key, v);
    }
    QSettings *m_s = nullptr;
    QString m_name;
};

class KConfig
{
public:
    explicit KConfig(const QString &name)
        : m_s(std::make_unique<QSettings>(
              QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + name,
              QSettings::IniFormat))
    {
    }
    KConfigGroup group(const QString &name) { return KConfigGroup(m_s.get(), name); }
    bool hasGroup(const QString &name) const { return m_s->childGroups().contains(name); }
    void sync() { m_s->sync(); }

private:
    std::unique_ptr<QSettings> m_s;
};

#endif
