# paint.software — comninos22 fork

A Linux image editor with layers, selections, effects and adjustments, built with C++17 and Qt6.
This personal fork is based on [Univers4craft/paint.software](https://github.com/Univers4craft/paint.software),
upstream commit `4cb2bb87ca9bed4e8fc1816b8ba385fc3eabe204`. The original MIT license and attribution are preserved.

This fork adds bounded file parsing, atomic saves, native-plugin opt-in, and a smaller release pipeline.
See [the security review](docs/security-review.md) for findings, tests, and limitations. A review is not a guarantee that software is free of vulnerabilities.

## Download

Use only [this fork's releases](https://github.com/comninos22/paint.software/releases) to obtain these changes.
The release provides an x86-64 AppImage, source, and an amd64 `.deb` built on Ubuntu 24.04, with SHA-256 checksums.
The AppImage bundles Qt 6.12.0 and runs on modern Linux desktops; it was smoke-tested on Ubuntu 24.04
and Fedora 44 (including Wayland). See [AppImage requirements and packaging](packaging/APPIMAGE.md).
Replace the AppImage with an updated release to update its bundled libraries.
The `.deb` uses distribution Qt libraries; keep the operating system and image codecs updated.

```sh
chmod +x paint.software-1.1.93-security.1-x86_64.AppImage
./paint.software-1.1.93-security.1-x86_64.AppImage
# If FUSE is unavailable:
./paint.software-1.1.93-security.1-x86_64.AppImage --appimage-extract-and-run
```

```sh
sha256sum -c SHA256SUMS
sudo apt install ./paint.software_1.1.93_amd64.deb
```

There is no APT repository or Flatpak release for this fork. The Debian package uses the
same `paintdotnet` executable and package name as upstream and replaces an installed upstream package.
Checksums detect corruption; they are not an independent publisher signature.

## Build and test

Install a C++17 compiler, CMake 3.16+, Ninja, and Qt 6.4+ development packages (Core, Gui, Widgets, PrintSupport).
Use your distribution's supported, security-patched Qt packages.

```sh
# Ubuntu / Debian
sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-image-formats-plugins libgl1-mesa-dev
# Fedora (alternative)
# sudo dnf install gcc-c++ cmake ninja-build qt6-qtbase-devel qt6-qtimageformats mesa-libGL-devel

git clone https://github.com/comninos22/paint.software.git
cd paint.software
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target all paintdotnet_tests paintdotnet_brush_tests paintdotnet_security_tests -j2
ctest --test-dir build --output-on-failure
./build/paintdotnet
```

To check the file reader with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -B build-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DPAINTSW_SANITIZERS=ON
cmake --build build-sanitize --target paintdotnet_security_tests -j2
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 ctest --test-dir build-sanitize -R security_regression --output-on-failure
```

## File and plugin behavior

- Open/import supports PNG, JPEG, BMP, GIF, TIFF, WebP, and native layered `.psw` documents.
- Input limits: 32,768 pixels per axis, 32 megapixels (binary units) per image, 256 layers,
  128 megapixels across native canvas plus layers, 512 MiB encoded file size, 4,096 UTF-16 units per layer name.
- Invalid native versions, metadata, layer sizes, and truncated/trailing data are rejected before changing the document.
- Files exceeding these limits are rejected, including older upstream files that violate layer-size invariants.
- Native plugins are disabled by default. To deliberately enable trusted plugins, launch with
  `PAINTSW_ENABLE_PLUGINS=1 ./build/paintdotnet` (or `PAINTSW_ENABLE_PLUGINS=1 paintdotnet` after installation).
  Plugins execute arbitrary native code with your account's permissions; they are not sandboxed.
  See [the plugin API guide](plugins/README.md).

The [historical upstream README](docs/README.upstream.md) describes the full feature set and original project.
Its upstream downloads do not contain this fork's fixes.
