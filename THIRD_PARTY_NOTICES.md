# Third-party notices

Sound Chest's own code is licensed under the MIT License (see [LICENSE](LICENSE)).
It depends on the third-party components below, each with its own license.

## Runtime dependencies

| Component | License | Used for | Source |
| --- | --- | --- | --- |
| [Qt 6](https://www.qt.io) | LGPL-3.0 | Core, Gui, Widgets, Svg, Concurrent, Multimedia | https://code.qt.io/cgit/qt/ |
| [FFmpeg](https://ffmpeg.org) (via Qt Multimedia) | LGPL-2.1-or-later in the builds we distribute | Audio decoding and playback | https://ffmpeg.org/download.html |
| [libsndfile](https://github.com/libsndfile/libsndfile) | LGPL-2.1-or-later | Reading audio file metadata and samples | https://github.com/libsndfile/libsndfile |
| [libebur128](https://github.com/jiixyj/libebur128) | MIT | EBU R128 loudness measurement (LUFS) | https://github.com/jiixyj/libebur128 |
| [Qlementine](https://github.com/oclero/qlementine) | MIT | Qt widget style | https://github.com/oclero/qlementine |
| [Phosphor Icons](https://github.com/phosphor-icons/core) | MIT | Application and toolbar icons | https://github.com/phosphor-icons/core |
| [Inter](https://github.com/rsms/inter) | SIL OFL 1.1 | Letterforms in the Sound Chest wordmark (converted to outlines) | https://github.com/rsms/inter |

## LGPL compliance

Sound Chest links Qt, libsndfile, and FFmpeg dynamically, so you can replace those
libraries with modified versions. If you receive a binary distribution of Sound
Chest, the corresponding source code for those libraries is available at the
links above; the exact versions are recorded in the build workflow and in the
release notes.

The gruvbox color palette used by the bundled theme follows the
[gruvbox](https://github.com/morhetz/gruvbox) project (MIT).
