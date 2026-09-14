#!/usr/bin/env bash
# Regenerates the committed brand assets from assets/branding/logo.svg.
#
# Requires: inkscape, imagemagick (magick), python3.
# Outputs generated SVGs to assets/branding/ and rasters to assets/generated/.

set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
brand="$root/assets/branding"
out="$root/assets/generated"

mkdir -p "$out"

python3 "$root/scripts/make-svg-assets.py"

render() {
  local source="$1" width="$2" target="$3"
  inkscape "$source" -w "$width" -o "$target" >/dev/null 2>&1
}

sizes=(16 24 32 48 64 128 256 512 1024)
for size in "${sizes[@]}"; do
  render "$brand/soundchest-appicon.svg" "$size" "$out/soundchest-$size.png"
done

magick "$out"/soundchest-{16,24,32,48,64,128,256}.png "$out/soundchest.ico"

icns_sizes=(16 32 64 128 256 512 1024)
icns_sources=()
for size in "${icns_sizes[@]}"; do
  icns_sources+=("$out/soundchest-$size.png")
done
python3 "$root/scripts/png-to-icns.py" "$out/soundchest.icns" "${icns_sources[@]}"

# itch.io cover image (630x500): app tile above the wordmark on the dark theme.
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

render "$brand/soundchest-appicon.svg" 256 "$work/tile.png"
render "$brand/soundchest-lockup-on-dark.svg" 440 "$work/lockup.png"

magick -size 630x500 xc:'#282828' \
  \( "$work/tile.png" \) -gravity North -geometry +0+64 -composite \
  \( "$work/lockup.png" \) -gravity South -geometry +0+84 -composite \
  "$out/itch-cover-630x500.png"

echo "brand assets written to assets/generated/"
