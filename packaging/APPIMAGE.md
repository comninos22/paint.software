# AppImage

Download `paint.software-1.1.93-security.1-x86_64.AppImage` from
[the personal release](https://github.com/comninos22/paint.software/releases/tag/v1.1.93-security.1).
Check it against the release's `SHA256SUMS`, then make it executable and launch it.
Use `--appimage-extract-and-run` if FUSE is unavailable; no installation or root access is required.

This build targets x86-64 desktops, bundles Qt 6.12.0 and ICU 73.2, and requires glibc 2.38+
and a C++ runtime providing `GLIBCXX_3.4.29`. Normal desktop graphics libraries and drivers
remain system dependencies. It was built on Ubuntu 24.04 and smoke-tested there with X11
and offscreen rendering, and on Fedora 44 with offscreen rendering and Wayland.
These are startup tests, not exhaustive desktop, printing, GPU, or distribution compatibility tests.

The application source is the existing immutable `v1.1.93-security.1` tag,
commit `e07dfa988bb143282717985c23df758a2e47deb6`. The packaging recipe was added later on `main`;
the tag and original source archive were not moved. Build metadata records both separately.
Native application plugins remain disabled by default. The AppImage is not a sandbox.
Its Qt and codecs are bundled, so update the entire AppImage when a rebuilt release is available.

## Build recipe

Use a disposable Ubuntu 24.04 x86-64 container. Install these build/runtime dependencies:

```sh
apt-get update
apt-get install -y --no-install-recommends build-essential cmake ninja-build python3 \
  libarchive-tools ca-certificates file desktop-file-utils qt6-base-dev libgl1-mesa-dev \
  libfontconfig1-dev libxkbcommon-x11-0 libxcb-cursor0 libxcb-icccm4 \
  libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libwayland-cursor0 \
  libwayland-egl1 libcups2t64 libdbus-1-3

# Run the following as an unprivileged build user, from this repository.
python3 packaging/prepare-appimage.py /tmp/paint-appimage-build
JOBS=2 bash packaging/build-appimage.sh /tmp/paint-appimage-build
```

The preparation script checks every input against `appimage-inputs.json` before extracting or
executing anything. To use cached downloads, place the exact files in the work directory's `inputs/`
subdirectory first. Use a new work directory for each build. Packaging accepts an optional second
argument pointing to a different source checkout, for example the original release tag with the
current packaging notice/manifest copied into its `packaging/` directory.

The recipe builds and tests the application, installs it into an AppDir, bundles X11/Wayland
support, omits the optional SVG-dependent Adwaita decoration and unused raster plugins,
includes license notices and Qt SBOMs, then uses an explicitly pinned AppImage runtime.
Nothing is uploaded or published. All three regression suites must pass before packaging.
The result is in the work directory's `dist/` subdirectory.

Upstream deployment tools use mutable `continuous` URLs. The fixed hashes intentionally reject
replaced downloads; retain verified files or review a new tool version and update its pin.
This is a repeatable recipe with pinned upstream inputs, not a claim of byte-for-byte reproducible
builds: Ubuntu package updates, compiler versions and build timestamps can change the output.

## Notices and corresponding source

`APPIMAGE-NOTICES.txt`, Qt license texts/SBOMs, and Ubuntu package notices are included under
`usr/share/doc` inside the image. The release's `AppImage-dependency-sources.tar` contains the
Qt and bundled Ubuntu library sources, source manifests, and AppImage runtime/FUSE source.
For Ubuntu libraries, the `.dsc`, original tarball and Debian patch archive together form the source.
Use `dpkg-source -x package.dsc` to unpack them; Qt modules include their CMake build instructions.

To replace compatible LGPL libraries, run the AppImage with `--appimage-extract`, replace libraries
in `squashfs-root/usr/lib` or Qt plugins in `usr/plugins`, then run `squashfs-root/AppRun`.
There is no signature lock preventing modified libraries. Preserve the included notices when redistributing.
