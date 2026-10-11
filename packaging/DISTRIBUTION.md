# Distribution of this fork

The release artifacts are source, an x86-64 [AppImage](APPIMAGE.md), and an amd64 Debian package built on Ubuntu 24.04.
Build from the reviewed checkout:

```sh
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build --target all paintdotnet_tests paintdotnet_brush_tests paintdotnet_security_tests paintdotnet_ui_tests -j2
ctest --test-dir build --output-on-failure
cpack --config build/CPackConfig.cmake -G DEB -B dist
(cd dist && sha256sum *.deb > SHA256SUMS)
```

The manual GitHub package workflow performs the same build and tests and uploads an artifact only.
Publishing a GitHub release is a separate maintainer action after checking its exact source commit and assets.
No workflow publishes automatically, and all workflows use read-only repository permissions.

The upstream AppImage, APT and Flatpak workflows and upstream-fetching manifests have been retired in this fork.
No unsigned APT fallback is offered. The Debian package uses operating-system Qt and codec libraries.
The replacement AppImage recipe pins Qt 6.12.0, deployment tools, and the runtime by SHA-256,
runs all three test suites, and publishes nothing automatically. Its bundled libraries require
a rebuilt AppImage to receive updates.
The Debian dependency-rewriting helper is retained for historical reference but is not used for releases;
the package retains the dependency names and minimum versions detected on its actual build distribution.
