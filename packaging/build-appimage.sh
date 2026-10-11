#!/usr/bin/env bash
set -euo pipefail
# Run in a disposable Ubuntu 24.04 x86-64 build container. Nothing is published.
work=$(realpath "${1:?Usage: build-appimage.sh WORK_DIR [SOURCE_DIR]}")
source_dir=$(realpath "${2:-$(dirname "$0")/..}")
build_dir=${BUILD_DIR:-"$work/build"}
appdir="$work/AppDir"
qt="$work/Qt"
version=${APPIMAGE_VERSION:-1.1.95}
export LANG=C.UTF-8
export QMAKE="$qt/bin/qmake6"
export LD_LIBRARY_PATH="$qt/lib"
export PATH="$work/tools/linuxdeploy-plugin-qt/squashfs-root/usr/bin:$PATH"
export EXTRA_QT_MODULES=printsupport
export EXTRA_PLATFORM_PLUGINS='libqwayland.so;libqoffscreen.so'
test "$("$QMAKE" -query QT_VERSION)" = 6.12.0
test ! -e "$appdir" # never overwrite an unrelated directory

cmake --fresh -S "$source_dir" -B "$build_dir" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH="$qt"
cmake --build "$build_dir" --target all paintdotnet_tests paintdotnet_brush_tests paintdotnet_security_tests paintdotnet_ui_tests -j"${JOBS:-2}"
ctest --test-dir "$build_dir" --output-on-failure
DESTDIR="$appdir" cmake --install "$build_dir"
# CMake needs the complete SDK while configuring; deployment only needs the
# standard bradient decoration, which does not depend on the optional QtSvg.
adwaita="$qt/plugins/wayland-decoration-client/libadwaita.so"
mv "$adwaita" "$work/libadwaita.so.undeployed"
trap 'mv "$work/libadwaita.so.undeployed" "$adwaita"' EXIT
"$work/tools/linuxdeploy/squashfs-root/AppRun" --appdir "$appdir" \
  --desktop-file "$appdir/usr/share/applications/paint.software.desktop" \
  --icon-file "$appdir/usr/share/pixmaps/paint.software.png" --plugin qt
# Only the application's advertised raster formats are needed.
rm -f "$appdir/usr/plugins/imageformats/"libq{icns,ico,tga,wbmp}.so
docs="$appdir/usr/share/doc/paint.software"
cp -a "$work/notices" "$docs/third-party"
cp -a "$qt/sbom" "$docs/qt-sbom"
cp -a /usr/share/common-licenses "$docs/common-licenses"
cp "$source_dir/packaging/APPIMAGE-NOTICES.txt" "$docs/"
cp "$source_dir/packaging/appimage-inputs.json" "$docs/"
mkdir -p "$work/dist"
# Supplying the verified runtime prevents appimagetool from downloading one.
ARCH=x86_64 VERSION="$version" "$work/tools/appimagetool/squashfs-root/AppRun" \
  --runtime-file "$work/inputs/runtime-x86_64" "$appdir" \
  "$work/dist/paint.software-$version-x86_64.AppImage"
(cd "$work/dist" && sha256sum *.AppImage > AppImage-SHA256SUMS)
