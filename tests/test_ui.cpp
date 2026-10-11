// Exercise actual MainWindow startup/restart with isolated settings. Run with
// offscreen in CTest and xcb under Xvfb to verify native palette mapping too.
#include "mainwindow.h"
#include "i18n.h"
#include "canvas/canvaswidget.h"
#include "core/document.h"
#include "dialogs/settingsdialog.h"
#include "dialogs/newdocumentdialog.h"
#include "panels/colorspanel.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMenuBar>
#include <QPushButton>

#include <QApplication>
#include <QDockWidget>
#include <QEventLoop>
#include <QPixmap>
#include <QScreen>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
#include <QWindow>
#include <cstdio>

static int failures = 0;

static void check(bool condition, const char *description) {
    std::printf("%s %s\n", condition ? "PASS" : "FAIL", description);
    if (!condition) ++failures;
}

static void settle(int milliseconds = 350) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

static QDockWidget *panel(MainWindow &window, const char *name) {
    auto *dock = window.findChild<QDockWidget *>(QLatin1String(name));
    if (!dock) qFatal("Missing panel: %s", name);
    return dock;
}

static bool closeGeometry(const QRect &actual, const QRect &expected) {
    return (actual.topLeft() - expected.topLeft()).manhattanLength() <= 4
        && (actual.bottomRight() - expected.bottomRight()).manhattanLength() <= 4;
}

int main(int argc, char **argv) {
    QTemporaryDir isolated;
    if (!isolated.isValid()) return 2;
    qputenv("XDG_CONFIG_HOME", isolated.path().toUtf8());
    qputenv("XDG_DATA_HOME", isolated.path().toUtf8());
    QApplication app(argc, argv);
    app.setApplicationName("paint-software-ui-test");
    app.setOrganizationName("PaintSoftwareTests");
    app.setQuitOnLastWindowClosed(false);
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, isolated.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, isolated.path());
    const bool wayland = QGuiApplication::platformName().startsWith(QLatin1String("wayland"));
    QRect savedTools, savedHistory;

    {
        MainWindow window;
        window.show();
        settle();
        auto *tools = panel(window, "ToolsDock");
        auto *history = panel(window, "HistoryDock");
        auto *layers = panel(window, "LayersDock");
        auto *colors = panel(window, "ColorsDock");
        for (auto *dock : {tools, history, layers, colors}) {
            check(dock->isVisible(), "panel is visible on first launch without toggling");
            check(!dock->isFloating() || (dock->windowHandle() && dock->windowHandle()->isVisible()),
                  "panel's native window is shown, or panel is docked");
        }
        if (wayland) {
            check(!tools->isFloating() && !colors->isFloating()
                  && !history->isFloating() && !layers->isFloating(),
                  "native Wayland uses persistent dock areas");
            check(window.dockWidgetArea(tools) == Qt::LeftDockWidgetArea
                  && window.dockWidgetArea(colors) == Qt::LeftDockWidgetArea
                  && window.dockWidgetArea(history) == Qt::RightDockWidgetArea
                  && window.dockWidgetArea(layers) == Qt::RightDockWidgetArea,
                  "native Wayland has separate left/right panel groups");
        } else {
            check(tools->isFloating() && history->isFloating()
                  && layers->isFloating() && colors->isFloating(),
                  "desktop defaults retain four independent floating palettes");
            check(tools->pos() != colors->pos() && history->pos() != layers->pos()
                  && tools->pos() != history->pos(), "default palettes occupy distinct positions");
            const QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();
            if (screenSize.width() >= 1400 && screenSize.height() >= 900) {
                check(!tools->frameGeometry().intersects(colors->frameGeometry()),
                      "default Colors panel does not cover the Tools palette");
                check(!history->frameGeometry().intersects(layers->frameGeometry()),
                      "default History and Layers panels do not overlap");
            }
        }
        const QString screenshot = qEnvironmentVariable("PAINTSW_UI_SCREENSHOT");
        if (!screenshot.isEmpty())
            check(QGuiApplication::primaryScreen()->grabWindow(0).save(screenshot),
                  "save first-launch desktop screenshot");

        if (!wayland) {
            const QPoint origin = QGuiApplication::primaryScreen()->availableGeometry().topLeft();
            tools->resize(90, 350);
            tools->move(origin + QPoint(80, 90));
            history->resize(260, 220);
            history->move(origin + QPoint(350, 110));
        }
        layers->setFloating(false);
        window.addDockWidget(Qt::BottomDockWidgetArea, layers);
        colors->hide();
        settle(30);
        savedTools = tools->geometry();
        savedHistory = history->geometry();
        check(window.close(), "close first session cleanly");
        settle(10);
    }

    {
        MainWindow window;
        window.show();
        settle(); // Longer than the old 250 ms timer which overwrote positions.
        auto *tools = panel(window, "ToolsDock");
        auto *history = panel(window, "HistoryDock");
        auto *layers = panel(window, "LayersDock");
        auto *colors = panel(window, "ColorsDock");
        check(tools->isVisible() && history->isVisible() && layers->isVisible(),
              "enabled panels return on restart");
        check(colors->isHidden(), "deliberately hidden panel remains hidden");
        check(!layers->isFloating() && window.dockWidgetArea(layers) == Qt::BottomDockWidgetArea,
              "user's dock area survives restart");
        if (!wayland) {
            check(tools->isFloating() && closeGeometry(tools->geometry(), savedTools),
                  "custom tools position and size survive restart");
            check(history->isFloating() && closeGeometry(history->geometry(), savedHistory),
                  "custom history position and size survive restart");
        }
        colors->show();
        check(colors->isVisible(), "hidden panel can be enabled again");
        window.showMinimized();
        settle();
        check(window.close(), "close minimized session cleanly");
        settle(10);
        QSettings settings("PaintDali", "PaintDali");
        for (const char *key : {"ui/toolsVisible", "ui/historyVisible", "ui/layersVisible", "ui/colorsVisible"})
            check(settings.value(key).toBool(), "minimized close retains enabled-panel preference");
    }

    {
        MainWindow window;
        window.show();
        settle();
        for (const char *name : {"ToolsDock", "HistoryDock", "LayersDock", "ColorsDock"})
            check(panel(window, name)->isVisible(), "panel returns after minimized-close restart");
        if (!wayland) {
            panel(window, "ToolsDock")->move(50000, 50000);
            settle(10);
        }
        check(window.close(), "save layout with unreachable monitor position");
        settle(10);
    }

    if (!wayland) {
        MainWindow window;
        window.show();
        settle();
        const QRect frame = panel(window, "ToolsDock")->frameGeometry();
        bool reachable = false;
        for (QScreen *screen : QGuiApplication::screens())
            reachable |= screen->availableGeometry().contains(frame.topLeft())
                && screen->availableGeometry().intersects(frame);
        check(reachable, "panel from a removed monitor is brought back on screen");
        // Simulate upgrading a desktop V7 layout to ensure migration also keeps
        // valid user coordinates instead of replacing them with defaults.
        auto *tools = panel(window, "ToolsDock");
        tools->move(QGuiApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(120, 140));
        settle(10);
        savedTools = tools->geometry();
        QSettings settings("PaintDali", "PaintDali");
        settings.setValue("ui/mainWindowStateV7", QString(window.saveState(7).toBase64()));
        window.close();
        settings.remove("ui/mainWindowStateV8");
        settings.sync();
        settle(10);
    }

    if (!wayland) {
        MainWindow window;
        window.show();
        settle();
        check(closeGeometry(panel(window, "ToolsDock")->geometry(), savedTools),
              "existing V7 desktop layout migrates with its custom position");
        window.close();
    }

    if (!wayland) {
        {
            MainWindow window;
            window.show();
            settle();
            for (const char *name : {"ToolsDock", "HistoryDock", "LayersDock", "ColorsDock"}) {
                auto *dock = panel(window, name);
                dock->setFloating(true);
                // A formerly bottom-docked panel inherits the full main-window
                // width; keep this fixture within the 800px offscreen display
                // so Qt doesn't repair its position before migration is tested.
                dock->resize(dock->objectName() == QLatin1String("ToolsDock") ? QSize(90, 350) : QSize(260, 300));
                dock->move(200, 200);
            }
            settle(10);
            QSettings settings("PaintDali", "PaintDali");
            settings.setValue("ui/mainWindowStateV7", QString(window.saveState(7).toBase64()));
            window.close();
            settings.remove("ui/mainWindowStateV8");
            settings.sync();
            settle(10);
        }
        {
            MainWindow window;
            window.show();
            settle();
            check(panel(window, "ToolsDock")->pos() != panel(window, "HistoryDock")->pos()
                  && panel(window, "ColorsDock")->pos() != panel(window, "LayersDock")->pos(),
                  "coincident legacy palettes recover their separate default positions");
            window.close();
        }
    }

    {
        QSettings settings("PaintDali", "PaintDali");
        settings.setValue(wayland ? "ui/mainWindowStateV8Wayland" : "ui/mainWindowStateV8", "invalid-layout");
        settings.sync();
        MainWindow window;
        window.show();
        settle();
        for (const char *name : {"ToolsDock", "HistoryDock", "LayersDock", "ColorsDock"})
            check(panel(window, name)->isVisible(), "invalid saved layout recovers enabled panels");
        check(panel(window, "ToolsDock")->pos() != panel(window, "HistoryDock")->pos(),
              "invalid saved layout recovers distinct default positions");
        window.close();
    }

    // Switch through the real modal Settings dialog, then simulate startup.
    {
        I18n::setLanguage(I18n::Lang::English);
        I18n::applyQtTranslations();
        MainWindow window;
        window.show();
        settle();
        auto *canvas = window.findChild<CanvasWidget *>();
        check(canvas && canvas->document(), "language test has an active document");
        Document *document = canvas->document();
        const QImage before = document->flatten();
        const bool modified = document->isModified();
        const QPoint toolsPosition = panel(window, "ToolsDock")->pos();
        QTimer::singleShot(0, [&]() {
            auto *settings = window.findChild<SettingsDialog *>();
            check(settings != nullptr, "open actual Settings dialog");
            if (!settings) { if (auto *modal = QApplication::activeModalWidget()) modal->close(); return; }
            auto *combo = settings->findChild<QComboBox *>("languageCombo");
            const int greek = combo ? combo->findData(int(I18n::Lang::Greek)) : -1;
            check(greek >= 0 && combo->itemText(greek) == QStringLiteral("Ελληνικά"),
                  "Greek is discoverable by its native name");
            if (greek >= 0) combo->setCurrentIndex(greek);
            settle(40);
            check(settings->windowTitle() == QStringLiteral("Ρυθμίσεις"), "open Settings dialog retranslates immediately");
            check(panel(window, "ColorsDock")->windowTitle() == QStringLiteral("Χρώματα"),
                  "existing panel titles change to Greek immediately");
            auto *colors = window.findChild<ColorsPanel *>();
            auto *colorSlots = colors->findChild<QComboBox *>();
            check(colorSlots && colorSlots->itemText(0) == QStringLiteral("Πρωτεύον"),
                  "existing Colors panel controls change to Greek immediately");
            const QString screenshot = qEnvironmentVariable("PAINTSW_GREEK_SCREENSHOT");
            if (!screenshot.isEmpty())
                check(settings->grab().save(screenshot + ".settings.png"), "save Greek Settings screenshot");
            settings->accept();
        });
        check(QMetaObject::invokeMethod(&window, "showSettingsDialog", Qt::DirectConnection), "invoke Settings language selection");
        settle(250);
        check(window.menuBar()->actions().first()->text() == QStringLiteral("&Αρχείο"), "main menu changes to Greek immediately");
        check(window.menuBar()->cornerWidget(Qt::TopRightCorner)
              && window.menuBar()->cornerWidget(Qt::TopRightCorner)->isVisible(),
              "Settings and panel icons remain visible after changing language");
        check(document == canvas->document() && document->flatten() == before && document->isModified() == modified,
              "language switch preserves document pixels and modification state");
        check(panel(window, "ToolsDock")->pos() == toolsPosition, "language switch preserves panel position");
        QSettings settings("PaintDali", "PaintDali");
        check(settings.value("ui/language").toString() == "el", "Greek persists with ISO language code el");
        const QString screenshot = qEnvironmentVariable("PAINTSW_GREEK_SCREENSHOT");
        if (!screenshot.isEmpty())
            check(QGuiApplication::primaryScreen()->grabWindow(0).save(screenshot + ".png"), "save Greek desktop screenshot");
        window.close();
    }
    {
        I18n::setLanguage(I18n::Lang::English);
        I18n::loadFromSettings();
        I18n::applyQtTranslations();
        check(I18n::language() == I18n::Lang::Greek, "Greek survives restart");
        MainWindow window;
        window.show();
        settle();
        check(panel(window, "LayersDock")->windowTitle() == QStringLiteral("Στρώσεις")
              && panel(window, "LayersDock")->isVisible(), "Greek panels are visible after restart");
        NewDocumentDialog dialog;
        check(dialog.windowTitle() == QStringLiteral("Νέο"), "new document dialog opens in Greek");
        auto *buttons = dialog.findChild<QDialogButtonBox *>();
        check(buttons && buttons->button(QDialogButtonBox::Cancel)->text().remove('&') == QStringLiteral("Ακύρωση"),
              "Greek standard dialog buttons survive restart");
        window.close();
    }
    {
        QSettings settings("PaintDali", "PaintDali");
        for (const auto lang : {I18n::Lang::French, I18n::Lang::English, I18n::Lang::Greek}) {
            I18n::setLanguage(lang);
            I18n::saveToSettings();
            I18n::setLanguage(I18n::Lang::English);
            I18n::loadFromSettings();
            check(I18n::language() == lang, "each language round-trips through settings");
        }
        settings.setValue("ui/language", "invalid-language");
        I18n::loadFromSettings();
        check(I18n::language() == I18n::Lang::English, "unknown saved language falls back to English");
        I18n::applyQtTranslations();
    }

    std::printf("UI regression: %d failure(s)\n", failures);
    return failures ? 1 : 0;
}
