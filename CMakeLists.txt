cmake_minimum_required(VERSION 3.16)
project(PaintDali VERSION 1.1.95 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Core Gui PrintSupport)

option(PAINTSW_SANITIZERS "Enable address and undefined-behavior sanitizers" OFF)
if(PAINTSW_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
if(UNIX AND NOT APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-fstack-protector-strong)
  add_link_options(-Wl,-z,relro,-z,now)
endif()

# Single source of truth for the version shown to users. CI passes
# -DPAINTSW_VERSION=<major.minor.commitcount>; a local build falls back to the
# project version. It reaches both the binary (title bar, About) and the .deb, so
# they can no longer drift apart — they were hard-coded and stuck at 1.0/1.1.
if(DEFINED PAINTSW_VERSION AND NOT PAINTSW_VERSION STREQUAL "")
  set(PAINTSW_VERSION_STR "${PAINTSW_VERSION}")
else()
  set(PAINTSW_VERSION_STR "${PROJECT_VERSION}")
endif()
message(STATUS "paint.software version: ${PAINTSW_VERSION_STR}")

set(CORE_SOURCES
    src/core/document.cpp
    src/core/layer.cpp
    src/core/selection.cpp
    src/core/history.cpp
    src/core/hatchpatterns.cpp
)

set(CANVAS_SOURCES
    src/canvas/canvaswidget.cpp
    src/canvas/canvasview.cpp
)

set(TOOLS_SOURCES
    src/tools/tool.cpp
    src/tools/brushtool.cpp
    src/tools/erasertool.cpp
    src/tools/penciltool.cpp
    src/tools/filltool.cpp
    src/tools/colorpickertool.cpp
    src/tools/recolortool.cpp
    src/tools/selectiontool.cpp
    src/tools/lassotool.cpp
    src/tools/movetool.cpp
    src/tools/selectionhandles.cpp
    src/tools/moveselectiontool.cpp
    src/tools/zoomtool.cpp
    src/tools/pantool.cpp
    src/tools/texttool.cpp
    src/tools/shapetool.cpp
    src/tools/linetool.cpp
    src/tools/gradienttool.cpp
    src/tools/clonestamptool.cpp
    src/tools/magicwandtool.cpp
)

set(PANELS_SOURCES
    src/panels/layerspanel.cpp
    src/panels/colorspanel.cpp
    src/panels/historypanel.cpp
    src/panels/tooloptionspanel.cpp
    src/panels/imagelistbar.cpp
)

set(EFFECTS_SOURCES
    src/effects/effect.cpp
    src/effects/blureffect.cpp
    src/effects/sharpeneffect.cpp
    src/effects/noiseeffect.cpp
    src/effects/embosseffect.cpp
    src/effects/edgedetecteffect.cpp
    src/effects/oilpainteffect.cpp
    src/effects/pixelateeffect.cpp
    src/effects/motionblureffect.cpp
    src/effects/radialblureffect.cpp
    src/effects/zoomblureffect.cpp
    src/effects/surfaceblureffect.cpp
    src/effects/unfocuseffect.cpp
    src/effects/fragmenteffect.cpp
    src/effects/bulgeeffect.cpp
    src/effects/twisteffect.cpp
    src/effects/frostedglasseffect.cpp
    src/effects/crystalizeeffect.cpp
    src/effects/tileeffect.cpp
    src/effects/dentseffect.cpp
    src/effects/polarinversioneffect.cpp
    src/effects/medianeffect.cpp
    src/effects/reducenoiseeffect.cpp
    src/effects/quantizeeffect.cpp
    src/effects/gloweffect.cpp
    src/effects/redeyeremoveeffect.cpp
    src/effects/softenportraiteffect.cpp
    src/effects/vignetteeffect.cpp
    src/effects/turbulenceeffect.cpp
    src/effects/reliefeffect.cpp
    src/effects/outlineeffect.cpp
    src/effects/morphologyeffect.cpp
    src/effects/dropshadoweffect.cpp
)

set(ADJUSTMENTS_SOURCES
    src/adjustments/adjustment.cpp
    src/adjustments/brightnesscontrast.cpp
    src/adjustments/huesaturation.cpp
    src/adjustments/levels.cpp
    src/adjustments/curves.cpp
    src/adjustments/invertcolors.cpp
    src/adjustments/sepia.cpp
    src/adjustments/posterize.cpp
    src/adjustments/threshold.cpp
    src/adjustments/desaturate.cpp
    src/adjustments/colorbalance.cpp
    src/adjustments/exposure.cpp
    src/adjustments/highlightsshadows.cpp
    src/adjustments/temperaturetint.cpp
    src/adjustments/invertalpha.cpp
)

set(DIALOGS_SOURCES
    src/dialogs/resizedialog.cpp
    src/dialogs/canvassizedialog.cpp
    src/dialogs/newdocumentdialog.cpp
    src/dialogs/huesaturationdialog.cpp
    src/dialogs/layerpropertiesdialog.cpp
    src/dialogs/curvesdialog.cpp
    src/dialogs/settingsdialog.cpp
    src/dialogs/previewdialog.cpp
    src/dialogs/saveconfigdialog.cpp
)

set(PLUGINS_SOURCES
    src/plugins/pluginmanager.cpp
    src/plugins/plugineffect.cpp
)

set(MAIN_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/toolicons.cpp
    src/i18n.cpp
    src/theme.cpp
    src/dockdropoverlay.cpp
)

set(RESOURCES
    src/resources/resources.qrc
)

set(ALL_SOURCES
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${CANVAS_SOURCES}
    ${TOOLS_SOURCES}
    ${PANELS_SOURCES}
    ${EFFECTS_SOURCES}
    ${ADJUSTMENTS_SOURCES}
    ${DIALOGS_SOURCES}
    ${PLUGINS_SOURCES}
    ${RESOURCES}
)

add_executable(paintdotnet ${ALL_SOURCES})

target_include_directories(paintdotnet PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(paintdotnet PRIVATE
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Core
    Qt6::Gui
)

target_compile_definitions(paintdotnet PRIVATE PAINTSW_VERSION_STR="${PAINTSW_VERSION_STR}")

install(TARGETS paintdotnet DESTINATION bin)
install(FILES LICENSE DESTINATION share/doc/paint.software)

# ---- Test harness (headless functional tests) ----
set(NONMAIN_SOURCES
    ${CORE_SOURCES}
    ${CANVAS_SOURCES}
    ${TOOLS_SOURCES}
    ${PANELS_SOURCES}
    ${EFFECTS_SOURCES}
    ${ADJUSTMENTS_SOURCES}
    ${DIALOGS_SOURCES}
    ${PLUGINS_SOURCES}
    src/toolicons.cpp
    src/i18n.cpp
    src/theme.cpp
    ${RESOURCES}
)

add_executable(paintdotnet_tests EXCLUDE_FROM_ALL
    tests/test_all.cpp
    ${NONMAIN_SOURCES}
)
target_include_directories(paintdotnet_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(paintdotnet_tests PRIVATE
    Qt6::Widgets Qt6::PrintSupport Qt6::Core Qt6::Gui)

add_executable(paintdotnet_brush_tests EXCLUDE_FROM_ALL
    tests/test_brush.cpp tests/referencebrush.cpp
    ${CORE_SOURCES} ${CANVAS_SOURCES} src/i18n.cpp src/tools/tool.cpp src/tools/brushtool.cpp)
target_include_directories(paintdotnet_brush_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(paintdotnet_brush_tests PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)
enable_testing()
add_executable(paintdotnet_security_tests EXCLUDE_FROM_ALL tests/test_security.cpp ${CORE_SOURCES})
target_include_directories(paintdotnet_security_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(paintdotnet_security_tests PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)
add_test(NAME security_regression COMMAND paintdotnet_security_tests)
add_test(NAME functional COMMAND paintdotnet_tests)
add_test(NAME brush_regression COMMAND paintdotnet_brush_tests)
add_executable(paintdotnet_ui_tests EXCLUDE_FROM_ALL
    tests/test_ui.cpp src/mainwindow.cpp src/dockdropoverlay.cpp ${NONMAIN_SOURCES})
target_include_directories(paintdotnet_ui_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(paintdotnet_ui_tests PRIVATE Qt6::Widgets Qt6::PrintSupport Qt6::Core Qt6::Gui)
target_compile_definitions(paintdotnet_ui_tests PRIVATE PAINTSW_VERSION_STR="${PAINTSW_VERSION_STR}")
add_test(NAME ui_layout COMMAND paintdotnet_ui_tests)
set_tests_properties(functional brush_regression security_regression ui_layout PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 180)

# ---- Sample plugin (demonstrates the C ABI; built into build/plugins) ----
# A MODULE library is a shared object meant to be dlopen'd, never linked against.
add_library(sample_sepia_plugin MODULE plugins/sample/sepia_plugin.c)
target_include_directories(sample_sepia_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/plugins)
set_target_properties(sample_sepia_plugin PROPERTIES
    AUTOMOC OFF                                                 # plain C, no Qt
    C_VISIBILITY_PRESET default                                 # keep psw_register_plugin exported
    PREFIX ""                                                   # -> sample_sepia_plugin.so
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# ---- Desktop integration (installed by the package) ----
install(FILES packaging/paint.software.desktop DESTINATION share/applications)
# AppStream metadata, so software centres and the AppImage describe the app.
install(FILES packaging/io.github.comninos22.PaintSoftware.metainfo.xml
        DESTINATION share/metainfo)
install(FILES src/resources/paintdali-logo.png
        DESTINATION share/pixmaps RENAME paint.software.png)
# Ship the sample plugin as a system plugin so a fresh install has a working example
# (loaded from /usr/lib/paint.software/plugins).
install(TARGETS sample_sepia_plugin LIBRARY DESTINATION lib/paint.software/plugins)

# ---- CPack: build a Debian package with `cpack -G DEB` ----
set(CPACK_PACKAGE_NAME "paint.software")
set(CPACK_PACKAGE_VENDOR "comninos22 (fork of Univers4craft/paint.software)")
# CI passes -DPAINTSW_VERSION=<major.minor.commitcount> so every push to main
# produces a higher version that APT sees as an upgrade; otherwise use PROJECT_VERSION.
set(CPACK_PACKAGE_VERSION "${PAINTSW_VERSION_STR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-source Paint.NET clone for Linux (C++/Qt6)")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/comninos22/paint.software")
set(CPACK_PACKAGE_CONTACT "comninos22 <51789730+comninos22@users.noreply.github.com>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
# Auto-detect the exact Qt/GL runtime dependencies from the built binary. This
# resolves the correct package names for the build distro (e.g. the libqt6*t64
# variants on Ubuntu 24.04), so we do not hard-code a fragile fallback list.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# WebP and TIFF are decoded/encoded by Qt image-format PLUGINS that are loaded at
# runtime, not linked — so SHLIBDEPS can't see them. Depend on the plugin package
# explicitly, otherwise those formats appear in Open/Save but silently fail
# (issue #19).
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-image-formats-plugins")
# Qt renders the standard dialog buttons (OK / Cancel / Yes / No) from its own
# catalogue, which ships separately. Recommends (installed by default) rather than
# Depends: without it the app runs fine, those buttons just stay English.
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "qt6-translations-l10n")
# Say "not compatible with this system" in plain words rather than leaving the
# user with a raw dependency error. Only reachable via dpkg -i / double-click:
# apt refuses before the package is ever unpacked (see the script's comment).
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/deb-control/preinst")
set(CPACK_PACKAGE_FILE_NAME "paint.software_${CPACK_PACKAGE_VERSION}_amd64")
include(CPack)
