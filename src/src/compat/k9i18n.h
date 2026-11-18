// Minimal replacement for KI18n: gettext lookup in the "k9copy" domain with
// KDE-style %1..%n argument substitution. Existing .po catalogs keep working.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9I18N_H
#define K9I18N_H

#include <QString>
#include <type_traits>

namespace k9i18n {
void init();                       // bind the text domain; call once from main()
QString lookup(const char *text);  // translated text, or the original

inline QString toArg(const QString &s) { return s; }
inline QString toArg(const char *s) { return QString::fromUtf8(s); }
inline QString toArg(QChar c) { return QString(c); }
template<typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, QString> toArg(T v) { return QString::number(v); }
}

inline QString i18n(const char *text) { return k9i18n::lookup(text); }

template<typename... Args>
inline QString i18n(const char *text, const Args &...args)
{
    QString s = k9i18n::lookup(text);
    ((s = s.arg(k9i18n::toArg(args))), ...);
    return s;
}

// Used by uic-generated code (uic -tr tr2i18n).
inline QString tr2i18n(const char *text, const char * = nullptr)
{
    return (text && *text) ? k9i18n::lookup(text) : QString();
}

#define I18N_NOOP(x) x

#endif
