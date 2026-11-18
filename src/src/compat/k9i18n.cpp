// Copyright: GPL-2.0-or-later, see COPYING.
#include "k9i18n.h"
#include <libintl.h>

#ifndef K9_LOCALEDIR
#define K9_LOCALEDIR "/usr/share/locale"
#endif

static const char *const kDomain = "k9copy";

void k9i18n::init()
{
    bindtextdomain(kDomain, K9_LOCALEDIR);
    bind_textdomain_codeset(kDomain, "UTF-8");
}

QString k9i18n::lookup(const char *text)
{
    return QString::fromUtf8(dgettext(kDomain, text));
}
