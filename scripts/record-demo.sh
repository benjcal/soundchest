#!/usr/bin/env bash
# Records the scripted UI demo and renders assets/media/soundchest-demo.{gif,webp}.
#
# The app runs on a private Xvfb display with QT_SCALE_FACTOR=2, so the UI is
# rendered at 2x and captured natively at 2560x1600 for crisp text on HiDPI
# screens. xdotool drives the mouse, ffmpeg captures the window, and the raw
# footage is palette-optimized into a looping GIF plus an animated WebP. The
# demo library is generated on demand and the app's settings live in a
# throwaway config dir, so your real session is untouched.
#
# Playback needs an audio server. When none is reachable (sandboxed or headless
# runs), a private PipeWire null sink is started in a temp directory; the app
# still "plays", so the playhead animates, but nothing is audible.
#
# Requires: Xvfb, xdotool, xwininfo, ffmpeg.
# Usage: scripts/record-demo.sh [--window WxH] [--scale N] [--width N] [--fps N]
#                               [--keep-raw FILE]

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
bin="${SOUNDCHEST_BIN:-$root/build-release/soundchest}"
library="$root/demo-library"
outdir="$root/assets/media"
window_w=1280
window_h=800
scale=2
width=2560
fps=12
keep_raw=""

while [ $# -gt 0 ]; do
  case "$1" in
    --window) window_w="${2%x*}"; window_h="${2#*x}"; shift 2 ;;
    --scale) scale="$2"; shift 2 ;;
    --width) width="$2"; shift 2 ;;
    --fps) fps="$2"; shift 2 ;;
    --keep-raw) keep_raw="$2"; shift 2 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
done

missing=()
for cmd in Xvfb xdotool xwininfo ffmpeg; do
  command -v "$cmd" >/dev/null || missing+=("$cmd")
done
if [ "${#missing[@]}" -gt 0 ]; then
  printf 'missing tools: %s\n' "${missing[*]}" >&2
  echo "Void: sudo xbps-install -S xorg-server-xvfb xdotool xwininfo ffmpeg" >&2
  exit 1
fi
[ -x "$bin" ] || { echo "no binary at $bin (run 'just build-release')" >&2; exit 1; }

if [ ! -d "$library" ]; then
  "$root/scripts/make-demo-library.sh" "$library"
fi

tmp="$(mktemp -d "${TMPDIR:-/tmp}/soundchest-demo-XXXXXX")"
private_pids=()

cleanup() {
  [ -n "${ffpid:-}" ] && kill "$ffpid" 2>/dev/null || true
  [ -n "${app_pid:-}" ] && kill "$app_pid" 2>/dev/null || true
  [ -n "${xvfb_pid:-}" ] && kill "$xvfb_pid" 2>/dev/null || true
  for pid in ${private_pids[@]+"${private_pids[@]}"}; do kill "$pid" 2>/dev/null || true; done
  if [ -n "$keep_raw" ] && [ -f "$tmp/raw.mp4" ]; then
    cp "$tmp/raw.mp4" "$keep_raw"
    echo "raw footage: $keep_raw"
  fi
  rm -rf "$tmp"
}
trap cleanup EXIT INT TERM

# --- app settings -----------------------------------------------------------

mkdir -p "$tmp/config/soundchest"
cat >"$tmp/config/soundchest/soundchest.conf" <<EOF
[open]
lastDir=$library

[audio]
volume=0
loop=false
autoplay=false
EOF

# --- fonts ------------------------------------------------------------------

# Sandboxes can hide /etc/fonts; fall back to a minimal config so the UI does
# not render in a random fallback face.
if [ ! -r /etc/fonts/fonts.conf ]; then
  cat >"$tmp/fonts.conf" <<EOF
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">
<fontconfig>
  <dir>/usr/share/fonts</dir>
  <dir>/usr/local/share/fonts</dir>
  <cachedir>$tmp/font-cache</cachedir>
  <alias binding="strong"><family>sans-serif</family><prefer><family>Inter</family><family>Noto Sans</family><family>DejaVu Sans</family></prefer></alias>
  <alias binding="strong"><family>Sans Serif</family><prefer><family>Inter</family><family>Noto Sans</family><family>DejaVu Sans</family></prefer></alias>
  <alias binding="strong"><family>Helvetica</family><prefer><family>Inter</family><family>Noto Sans</family><family>DejaVu Sans</family></prefer></alias>
  <alias binding="strong"><family>Arial</family><prefer><family>Inter</family><family>Noto Sans</family><family>DejaVu Sans</family></prefer></alias>
</fontconfig>
EOF
  export FONTCONFIG_FILE="$tmp/fonts.conf"
fi

# --- audio ------------------------------------------------------------------

if ! pactl info >/dev/null 2>&1; then
  if command -v pipewire >/dev/null && command -v pipewire-pulse >/dev/null; then
    rt="$tmp/rt"
    mkdir -p "$rt"
    chmod 700 "$rt"
    if command -v dbus-daemon >/dev/null; then
      dbus-daemon --session --address="unix:path=$rt/dbus" --fork --print-address >/dev/null 2>&1 || true
      export DBUS_SESSION_BUS_ADDRESS="unix:path=$rt/dbus"
    fi
    setsid nohup env XDG_RUNTIME_DIR="$rt" pipewire >"$rt/pipewire.log" 2>&1 </dev/null &
    private_pids+=("$!")
    sleep 0.5
    setsid nohup env XDG_RUNTIME_DIR="$rt" pipewire-pulse >"$rt/pulse.log" 2>&1 </dev/null &
    private_pids+=("$!")
    for _ in $(seq 1 30); do XDG_RUNTIME_DIR="$rt" pactl info >/dev/null 2>&1 && break; sleep 0.1; done
    if XDG_RUNTIME_DIR="$rt" pactl info >/dev/null 2>&1; then
      if command -v pw-cli >/dev/null; then
        XDG_RUNTIME_DIR="$rt" pw-cli create-node adapter \
          '{ factory.name=support.null-audio-sink node.name=sc-null node.description="Sound Chest demo" media.class=Audio/Sink object.linger=true }' \
          >/dev/null 2>&1 || true
      fi
      export XDG_RUNTIME_DIR="$rt"
      export PULSE_SERVER="unix:$rt/pulse/native"
    else
      echo "warning: could not start a private audio server; the playhead may not animate" >&2
    fi
  else
    echo "warning: no audio server reachable; the playhead may not animate" >&2
  fi
fi

# --- display ----------------------------------------------------------------

phys_w=$((window_w * scale))
phys_h=$((window_h * scale))

disp=""
for n in 99 98 97 96; do
  if [ ! -e "/tmp/.X${n}-lock" ]; then
    disp=":$n"
    break
  fi
done
[ -n "$disp" ] || { echo "no free Xvfb display found" >&2; exit 1; }

Xvfb "$disp" -screen 0 "$((phys_w + 320))x$((phys_h + 200))x24" -nolisten tcp >"$tmp/xvfb.log" 2>&1 &
xvfb_pid=$!
sleep 0.5

export DISPLAY="$disp"

# --- app --------------------------------------------------------------------

XDG_CONFIG_HOME="$tmp/config" QT_QPA_PLATFORM=xcb QT_SCALE_FACTOR="$scale" \
  "$bin" >"$tmp/app.log" 2>&1 &
app_pid=$!

wid=""
for _ in $(seq 1 100); do
  wid="$(xdotool search --onlyvisible --name '^Sound Chest$' 2>/dev/null | head -1 || true)"
  [ -n "$wid" ] && break
  sleep 0.1
done
[ -n "$wid" ] || { echo "app window never appeared; see $tmp/app.log" >&2; exit 1; }

xdotool windowsize "$wid" "$phys_w" "$phys_h"
xdotool windowmove "$wid" 0 0
sleep 2.5 # let the scan, peaks and analysis settle

info="$(xwininfo -id "$wid")"
wx="$(awk '/Absolute upper-left X/{print $4}' <<<"$info")"
wy="$(awk '/Absolute upper-left Y/{print $4}' <<<"$info")"
ww="$(awk '/Width:/{print $2; exit}' <<<"$info")"
wh="$(awk '/Height:/{print $2; exit}' <<<"$info")"

# --- layout -----------------------------------------------------------------

# Widget geometry in logical pixels, derived from the window height: the
# waveform stretches 3/8 of the free space, the browser splitter takes 5/8.
header_h=57
transport_h=67
status_h=28
free_h=$((window_h - header_h - transport_h - status_h))
wave_h=$((free_h * 3 / 8))
splitter_y=$((header_h + wave_h + transport_h))

play_y=$((splitter_y - 34))
wave_y=$((header_h + wave_h / 2))
seek_x=$((window_w * 70 / 100))
tree_x=100
file_x=340
play_x=43
stop_x=85
tree_root_y=$((splitter_y + 17))
row1_y=$((splitter_y + 45))
ambience_y=$((tree_root_y + 28))
foley_y=$((tree_root_y + 56))
scifi_y=$((tree_root_y + 140))
row3_y=$((row1_y + 68))
row5_y=$((row1_y + 136))

# --- capture ----------------------------------------------------------------

ffmpeg -hide_banner -loglevel error -y \
  -f x11grab -framerate 30 -video_size "${ww}x${wh}" -i "$disp.0+$wx,$wy" \
  -c:v libx264 -preset ultrafast -crf 12 -pix_fmt yuv420p "$tmp/raw.mp4" &
ffpid=$!
sleep 0.4

now_ms() {
  local t="${EPOCHREALTIME/./}"
  echo "${t:0:13}"
}

t0="$(now_ms)"
at() { # sleep until the given millisecond mark of the capture
  local wait=$(( $1 - ($(now_ms) - t0) ))
  [ "$wait" -le 0 ] || sleep "$(printf '%d.%03d' $((wait / 1000)) $((wait % 1000)))"
}

glide() { # move the pointer in small steps so the GIF shows a moving cursor
  eval "$(xdotool getmouselocation --shell)"
  local sx="$X" sy="$Y" dx="$1" dy="$2" steps=12 i
  for ((i = 1; i <= steps; i++)); do
    xdotool mousemove $((sx + (dx - sx) * i / steps)) $((sy + (dy - sy) * i / steps))
    sleep 0.012
  done
}

click() { # logical coordinates
  glide "$(( $1 * scale ))" "$(( $2 * scale ))"
  xdotool click 1
}

shift_click() {
  glide "$(( $1 * scale ))" "$(( $2 * scale ))"
  xdotool keydown shift click 1 keyup shift
}

xdotool mousemove $((900 * scale)) $((700 * scale))

# Select a folder, audition a file, seek, multi-select, browse the formats and
# settle back on the starting state so the loop is seamless.
at 300;   click "$tree_x" "$ambience_y"
at 1100;  click "$play_x" "$play_y"
at 3300;  click "$seek_x" "$wave_y"
at 4000;  click "$stop_x" "$play_y"
at 4300;  click "$tree_x" "$foley_y"
at 5000;  click "$file_x" "$row3_y"
at 5700;  shift_click "$file_x" "$row5_y"
at 6400;  click "$tree_x" "$scifi_y"
at 7100;  click "$file_x" "$row1_y"
at 7700;  click "$play_x" "$play_y"
at 8400;  click "$tree_x" "$ambience_y"
at 9100;  click "$play_x" "$play_y"
at 11600; click "$stop_x" "$play_y"
at 12400; click "$tree_x" "$tree_root_y"
at 12900; xdotool mousemove $((900 * scale)) $((700 * scale))

sleep 0.5
kill -INT "$ffpid"
wait "$ffpid" 2>/dev/null || true
ffpid=""

# --- render -----------------------------------------------------------------

mkdir -p "$outdir"
gif="$outdir/soundchest-demo.gif"
webp="$outdir/soundchest-demo.webp"

ffmpeg -hide_banner -loglevel error -y -i "$tmp/raw.mp4" \
  -vf "fps=$fps,scale=$width:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=256:stats_mode=diff[p];[b][p]paletteuse=dither=none" \
  -loop 0 -an "$gif"

ffmpeg -hide_banner -loglevel error -y -i "$tmp/raw.mp4" \
  -vf "fps=$fps,scale=$width:-1:flags=lanczos" \
  -c:v libwebp_anim -q:v 80 -loop 0 -an "$webp"

ls -lh "$gif" "$webp" | awk '{print $5, $9}'
