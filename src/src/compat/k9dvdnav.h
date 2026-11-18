// dvdnav_open() wrapper: libdvdnav >= 6 logs to stdout by default, which
// corrupts the MPEG stream k9play writes there. Route its messages to stderr.
// Copyright: GPL-2.0-or-later, see COPYING.
#ifndef K9DVDNAV_H
#define K9DVDNAV_H
#include <dvdnav/dvdnav.h>
#include <cstdarg>
#include <cstdio>

inline dvdnav_status_t k9dvdnav_open(dvdnav_t **dest, const char *path)
{
    static const dvdnav_logger_cb logger = {
        [](void *, dvdnav_logger_level_t level, const char *fmt, va_list args) {
            if (level == DVDNAV_LOGGER_LEVEL_DEBUG)
                return;
            std::fputs("libdvdnav: ", stderr);
            std::vfprintf(stderr, fmt, args);
            std::fputc('\n', stderr);
        }};
    return dvdnav_open2(dest, nullptr, &logger, path);
}
#endif
