#!/usr/bin/env bash
# Generates a synthetic sound library for screenshots and manual testing.
#
# Every extension the scanner accepts is covered by the "Format Sampler"
# folder. Everything is synthesized by ffmpeg, so the library is
# license-clean and reproducible.
#
# Usage: scripts/make-demo-library.sh [output-dir]
# Default output: <repo>/demo-library (gitignored)

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out="${1:-$root/demo-library}"

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
bases="$work/bases"
mkdir -p "$bases"

gen() { # name, lavfi input, audio filter, stereo(0/1)
  local name="$1" input="$2" af="$3" stereo="${4:-0}"
  local pan=""
  [ "$stereo" = "1" ] && pan=",pan=stereo|c0=c0|c1=c0"
  ffmpeg -hide_banner -loglevel error -y -f lavfi -i "$input" -af "${af}${pan}" -c:a pcm_s16le "$bases/$name.wav"
}

enc() { # source base name, destination path
  local src="$bases/$1" dst="$2"
  case "${dst##*.}" in
    wav)      ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s16le "$dst" ;;
    aiff|aif) ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s16be -f aiff "$dst" ;;
    au)       ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s16be -f au "$dst" ;;
    flac)     ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a flac "$dst" ;;
    ogg)      ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a libvorbis -q:a 4 "$dst" ;;
    opus)     ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a libopus -b:a 96k -f opus "$dst" ;;
    mp3)      ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a libmp3lame -b:a 192k "$dst" ;;
    caf)      ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s16le -f caf "$dst" ;;
    w64)      ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s24le -f w64 "$dst" ;;
    rf64)     ffmpeg -hide_banner -loglevel error -y -i "$src" -c:a pcm_s16le -f wav -rf64 always "$dst" ;;
    *)        echo "unknown extension: $dst" >&2; return 1 ;;
  esac
}

# --- base sounds -----------------------------------------------------------

gen impact "anoisesrc=color=pink:duration=0.5:amplitude=0.9:sample_rate=48000" \
  "lowpass=f=450,afade=t=in:d=0.004,afade=t=out:st=0.05:d=0.45,volume=1.3"
gen click "sine=frequency=1500:duration=0.05:sample_rate=48000" \
  "afade=t=out:st=0.004:d=0.045"
gen hover "sine=frequency=900:duration=0.05:sample_rate=48000" \
  "afade=t=out:st=0.004:d=0.045"
gen confirm "aevalsrc=0.5*sin(2*PI*(880+220*t)*t):d=0.18:s=48000" \
  "afade=t=out:st=0.12:d=0.06"
gen error "aevalsrc=0.5*sin(2*PI*(520-180*t)*t):d=0.28:s=44100" \
  "afade=t=out:st=0.18:d=0.1"
gen toggle "sine=frequency=640:duration=0.03:sample_rate=48000" \
  "afade=t=out:st=0.004:d=0.025"
gen forest "anoisesrc=color=brown:duration=8:amplitude=0.6:sample_rate=44100" \
  "lowpass=f=1400,tremolo=f=0.25:d=0.5,volume=0.5"
gen rain "anoisesrc=color=white:duration=8:amplitude=0.5:sample_rate=44100" \
  "highpass=f=900,lowpass=f=9000,volume=0.35"
gen wind "anoisesrc=color=pink:duration=8:amplitude=0.6:sample_rate=48000" \
  "lowpass=f=700,tremolo=f=0.12:d=0.7,volume=0.6" 1
gen room "anoisesrc=color=brown:duration=6:amplitude=0.5:sample_rate=44100" \
  "lowpass=f=400,volume=0.4"
gen hum "aevalsrc=0.18*sin(2*PI*60*t)+0.08*sin(2*PI*120*t)+0.04*sin(2*PI*180*t):d=6:s=48000" \
  "volume=0.8" 1
gen gravel "anoisesrc=color=pink:duration=0.35:amplitude=0.8:sample_rate=48000" \
  "lowpass=f=3500,tremolo=f=9:d=0.95,volume=0.9"
gen wood "anoisesrc=color=brown:duration=0.3:amplitude=0.9:sample_rate=44100" \
  "lowpass=f=1000,tremolo=f=6:d=0.9"
gen cloth "anoisesrc=color=white:duration=0.6:amplitude=0.5:sample_rate=44100" \
  "highpass=f=1200,lowpass=f=7000,tremolo=f=14:d=0.8,volume=0.5"
gen creak "aevalsrc=0.3*sin(2*PI*(280+90*sin(2*PI*0.6*t))*t):d=1.6:s=44100" \
  "volume=0.8"
gen drip "sine=frequency=1300:duration=0.12:sample_rate=48000" \
  "aecho=0.8:0.6:60|120:0.35|0.2,volume=0.9"
gen laser "aevalsrc=0.6*sin(2*PI*(2400-3200*t)*t):d=0.3:s=48000" \
  "afade=t=out:st=0.22:d=0.08"
gen drone "aevalsrc=0.28*sin(2*PI*110*t)+0.18*sin(2*PI*164.81*t)+0.12*sin(2*PI*220*t):d=8:s=48000" \
  "tremolo=f=0.15:d=0.4,volume=0.9" 1
gen teleport "aevalsrc=0.5*sin(2*PI*(180+1800*t)*t):d=0.6:s=48000" \
  "afade=t=out:st=0.45:d=0.15"
gen shield "anoisesrc=color=white:duration=0.4:amplitude=0.7:sample_rate=48000" \
  "highpass=f=600,lowpass=f=4500,volume=0.8"
gen powerup "aevalsrc=0.45*sin(2*PI*(300+1400*t)*t):d=0.55:s=48000" \
  "afade=t=in:d=0.02,afade=t=out:st=0.4:d=0.15"
gen tone "sine=frequency=440:duration=0.7:sample_rate=48000" \
  "afade=t=in:d=0.01,afade=t=out:st=0.55:d=0.15"

# --- library layout --------------------------------------------------------

mkdir -p "$out" \
  "$out/Impacts" "$out/UI" "$out/Ambience" "$out/Foley" "$out/SciFi" "$out/Format Sampler"

enc impact.wav  "$out/Impacts/impact_deep_01.wav"
enc impact.wav  "$out/Impacts/impact_metal_02.flac"
enc impact.wav  "$out/Impacts/impact_soft_03.aiff"
enc impact.wav  "$out/Impacts/impact_sub_04.ogg"
enc impact.wav  "$out/Impacts/impact_glass_05.opus"

enc click.wav   "$out/UI/ui_click_01.wav"
enc hover.wav   "$out/UI/ui_hover_02.aif"
enc confirm.wav "$out/UI/ui_confirm_03.mp3"
enc error.wav   "$out/UI/ui_error_04.opus"
enc toggle.wav  "$out/UI/ui_toggle_05.ogg"

enc forest.wav  "$out/Ambience/ambience_forest_loop.wav"
enc rain.wav    "$out/Ambience/ambience_rain_loop.ogg"
enc wind.wav    "$out/Ambience/ambience_wind_loop.flac"
enc room.wav    "$out/Ambience/ambience_room_tone.wav"
enc hum.wav     "$out/Ambience/ambience_machine_hum.caf"

enc gravel.wav  "$out/Foley/footsteps_gravel_01.wav"
enc wood.wav    "$out/Foley/footsteps_wood_02.au"
enc cloth.wav   "$out/Foley/cloth_rustle_03.flac"
enc creak.wav   "$out/Foley/door_creak_04.mp3"
enc drip.wav    "$out/Foley/water_drip_05.wav"

enc laser.wav    "$out/SciFi/laser_shot_01.wav"
enc drone.wav    "$out/SciFi/drone_loop_02.flac"
enc teleport.wav "$out/SciFi/teleport_03.ogg"
enc shield.wav   "$out/SciFi/shield_hit_04.opus"
enc powerup.wav  "$out/SciFi/power_up_05.wav"

enc tone.wav "$out/Format Sampler/sample_pcm16.wav"
enc tone.wav "$out/Format Sampler/sample_aiff.aiff"
enc tone.wav "$out/Format Sampler/sample_aif.aif"
enc tone.wav "$out/Format Sampler/sample_au.au"
enc tone.wav "$out/Format Sampler/sample_flac.flac"
enc tone.wav "$out/Format Sampler/sample_vorbis.ogg"
enc tone.wav "$out/Format Sampler/sample_opus.opus"
enc tone.wav "$out/Format Sampler/sample_mp3.mp3"
enc tone.wav "$out/Format Sampler/sample_caf.caf"
enc tone.wav "$out/Format Sampler/sample_w64.w64"
enc tone.wav "$out/Format Sampler/sample_rf64.rf64"

enc toggle.wav  "$out/menu_blip.wav"
enc confirm.wav "$out/ui_notify.ogg"

echo "demo library written to $out ($(find "$out" -type f | wc -l) files)"
