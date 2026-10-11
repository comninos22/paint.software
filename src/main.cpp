#include <QApplication>
#include <QIcon>
#include <QFileInfo>
#include "mainwindow.h"
#include "i18n.h"

// Set by CMake; only a hand-rolled build without the definition lands here.
#ifndef PAINTSW_VERSION_STR
#define PAINTSW_VERSION_STR "dev"
#endif
#include "theme.h"

int main(int argc, char *argv[]) {
    // The four floating palettes need persistent screen positions. Wayland's
    // xdg-shell does not allow clients to position top-level windows, whereas
    // XWayland does. Honour an explicit Qt platform selection; on native-only
    // Wayland the main window uses docked palettes instead.
    bool explicitPlatform = !qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM");
    for (int i = 1; i < argc; ++i) {
        const QByteArray arg(argv[i]);
        if (arg == "-platform" || arg.startsWith("-platform=")
            || arg == "--platform" || arg.startsWith("--platform="))
            explicitPlatform = true;
    }
    const bool waylandSession = qEnvironmentVariable("XDG_SESSION_TYPE") == "wayland"
        || !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
    if (!explicitPlatform && waylandSession && !qEnvironmentVariableIsEmpty("DISPLAY"))
        qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication app(argc, argv);
    app.setApplicationName("paint.software");
    app.setApplicationVersion(QStringLiteral(PAINTSW_VERSION_STR));
    app.setOrganizationName("PaintDali");
    app.setWindowIcon(QIcon(":/paintdali-logo.png"));

    // Restore the saved language / colour scheme before any UI is built.
    I18n::loadFromSettings();
    I18n::applyQtTranslations();
    Theme::loadFromSettings();

    MainWindow window;
    window.show();

    // Open any images passed on the command line (each becomes an entry in the
    // image list, like paint.net).
    for (int i = 1; i < argc; ++i) {
        const QString path = QString::fromLocal8Bit(argv[i]);
        if (QFileInfo::exists(path))
            window.openFile(path);
    }

    return app.exec();
}
