# K9Copy (Qt 6 revival)

K9Copy is a DVD backup tool. It copies a video DVD to a folder or ISO image,
shrinking it to fit a single-layer disc, and can also encode titles to
MKV/MP4/AVI/WebM files.

This is a port of [k9copy-reloaded](https://sourceforge.net/projects/k9copy-reloaded/) 3.0.3
(itself a continuation of Jean-Michel Petit's original K9Copy) to current Linux:

- **Plain Qt 6**: no KDE Frameworks, Phonon or xine. The KDE APIs the code used are
  replaced by thin shims in `src/src/compat/`.
- **Preview** through libmpv, plus the built-in libmpeg2 viewer.
- **Encoding** through ffmpeg instead of mencoder. Presets: H.264, HEVC, AV1, VP9,
  Xvid, MPEG-4, MPEG-2 or copy for video; AAC, MP3, Opus, Vorbis, AC-3, MP2, FLAC or
  copy for audio. DVD subtitles can be burned in, or kept as soft subtitles in MKV.
- **Drive detection** through libudev, and privileged commands through pkexec.
- Many fixes to the backup and encode pipeline.

## Dependencies

| Kind | Packages |
|---|---|
| Build | cmake, a C++17 compiler, gettext |
| Runtime libraries | Qt 6 (Base: Widgets, Xml, DBus, OpenGL), libmpv, libudev, libdvdread, libdvdnav, libmpeg2, FFmpeg |
| Runtime tools | ffmpeg, dvdauthor, dvd+rw-tools (growisofs) |
| Optional | libdvdcss (encrypted discs), mkvtoolnix (MKV with soft subtitles), cdrtools or cdrkit (ISO creation), polkit |

## Building

```sh
cmake -S src -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

On Arch Linux, `packaging/arch/PKGBUILD` builds a package.

## Command line

Besides the GUI, `k9copy --batch` runs headless:

```sh
# Back up a whole DVD to a folder or an ISO
k9copy --batch --input /dev/sr0 --backup-all \
       --prefered-audio-lang en --prefered-sub-lang en --output ~/backup.iso

# Encode the main title to a file of about 700 MB
k9copy --batch --input /dev/sr0 --backup-all \
       --prefered-audio-lang en --prefered-sub-lang en \
       --encode ~/movie.mkv --video-codec x264 --audio-codec AAC --size 700
```

`k9copy --help` lists all the options.

## Testing without a disc

`tools/make-test-dvd.sh DIR` builds a small synthetic `VIDEO_TS` with two audio
tracks, one subtitle and two chapters. It needs ffmpeg, ImageMagick and dvdauthor.

## Status

The backup and encode paths have been checked on synthetic DVDs. Physical discs
and the GUI wizard flows have not yet been tested much, so bug reports are welcome.

## License

GPL-2.0-or-later. See `src/COPYING`.
