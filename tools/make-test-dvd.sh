#!/bin/sh
# Builds a small synthetic DVD (VIDEO_TS) for testing k9copy without a disc:
# 20 s interlaced NTSC 16:9 title, two AC-3 tracks (en, fr), one DVD subtitle
# (en), two chapters.  Needs ffmpeg, ImageMagick (magick), dvdauthor/spumux.
#
#   tools/make-test-dvd.sh /tmp/k9test
#   k9copy --batch --input /tmp/k9test/DVD --backup-all \
#          --prefered-audio-lang en --prefered-sub-lang en --output /tmp/k9test/backup
#   k9copy --batch --input /tmp/k9test/DVD --backup-all \
#          --prefered-audio-lang en --prefered-sub-lang en \
#          --encode /tmp/k9test/out.mp4 --video-codec x264 --audio-codec AAC --size 8
set -eu
dir=${1:?usage: $0 <output-dir>}
mkdir -p "$dir"
cd "$dir"

ffmpeg -hide_banner -loglevel error -y \
    -f lavfi -i testsrc2=size=720x480:rate=30000/1001 \
    -f lavfi -i sine=440:sample_rate=48000 \
    -f lavfi -i sine=880:sample_rate=48000 \
    -t 20 -map 0 -map 1 -map 2 -vf setfield=tff \
    -target ntsc-dvd -aspect 16:9 -c:a ac3 -b:a 192k -flags +ilme+ildct title.mpg

magick -size 720x480 xc:none -fill white -stroke black -strokewidth 2 \
    -font DejaVu-Sans-Bold -pointsize 40 -gravity south -annotate +0+40 \
    'K9COPY SUBTITLE TEST' -colors 3 -type PaletteAlpha PNG8:sub.png

cat > spumux.xml <<'EOF'
<subpictures format="NTSC"><stream>
  <spu start="00:00:02.00" end="00:00:15.00" image="sub.png" force="no"/>
</stream></subpictures>
EOF
spumux -s 0 spumux.xml < title.mpg > title-sub.mpg 2>/dev/null

cat > dvd.xml <<'EOF'
<dvdauthor dest="DVD">
  <vmgm/>
  <titleset>
    <titles>
      <video format="ntsc" aspect="16:9" widescreen="nopanscan"/>
      <audio lang="en"/><audio lang="fr"/>
      <subpicture lang="en"/>
      <pgc><vob file="title-sub.mpg" chapters="0,00:00:10.000"/></pgc>
    </titles>
  </titleset>
</dvdauthor>
EOF
rm -rf DVD
VIDEO_FORMAT=NTSC dvdauthor -x dvd.xml >/dev/null 2>&1
echo "test DVD: $dir/DVD"
