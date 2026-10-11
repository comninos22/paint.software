#include "core/document.h"
#include "core/imagelimits.h"
#include <QApplication>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <limits>

namespace {
int failures = 0;
void check(bool ok, const char *message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}
QByteArray nativeFile(qint32 version = 1, qint32 active = 0, qint32 blend = 0,
                      float opacity = 1, QPoint offset = {}, QSize imageSize = {2, 2}) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    QImage image(imageSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    out << quint32(0x50535731) << version << qint32(2) << qint32(2) << active << qint32(1);
    out << QString::fromUtf8("Layer λ") << opacity << true << false << blend << offset << image;
    return data;
}
QByteArray word(QByteArray data, int position, quint32 value) {
    qToBigEndian(value, data.data() + position);
    return data;
}
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QTemporaryDir tmp;
    check(tmp.isValid(), "isolated test directory");
    if (!tmp.isValid()) return 1;
    const QString path = tmp.filePath("input.psw");
    auto write = [&](const QByteArray &data) {
        QFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
    };
    auto rejects = [&](const QByteArray &data, const char *message) {
        check(write(data), "write malformed fixture");
        Document doc(3, 3);
        doc.activeLayer()->clear(Qt::blue);
        doc.setModified(true);
        check(!doc.loadNative(path), message);
        check(doc.size() == QSize(3, 3) && doc.layerCount() == 1
              && doc.activeLayer()->image().pixelColor(0, 0) == QColor(Qt::blue)
              && doc.isModified() && doc.filePath().isEmpty(), "failed load preserves document");
    };
    const QByteArray good = nativeFile();
    check(write(good), "write valid legacy-format fixture");
    Document doc(1, 1);
    check(doc.loadNative(path), "Qt_6_0 native format remains compatible");
    check(doc.activeLayer()->name() == QString::fromUtf8("Layer λ"), "UTF-16 name preserved");
    check(doc.saveNative(tmp.filePath("roundtrip.psw")), "atomic native save");
    check(doc.loadNative(tmp.filePath("roundtrip.psw")), "native roundtrip");

    // These small, benign fixtures demonstrate validation failures in upstream
    // without allocating excessive memory or executing any code.
    rejects(nativeFile(2), "unsupported version");
    rejects(nativeFile(1, -1), "negative active index");
    rejects(nativeFile(1, 1), "out-of-range active index");
    rejects(nativeFile(1, 0, 99), "unknown blend mode");
    rejects(nativeFile(1, 0, 0, -0.1f), "negative opacity");
    rejects(nativeFile(1, 0, 0, 1.1f), "excessive opacity");
    rejects(nativeFile(1, 0, 0, std::numeric_limits<float>::quiet_NaN()), "NaN opacity");
    rejects(nativeFile(1, 0, 0, std::numeric_limits<float>::infinity()), "infinite opacity");
    rejects(nativeFile(1, 0, 0, 1, {std::numeric_limits<int>::min(), 0}), "overflow-prone offset");
    rejects(nativeFile(1, 0, 0, 1, {}, {1, 1}), "layer/canvas size mismatch");
    rejects(good + "junk", "trailing data");

    if (!qEnvironmentVariableIsSet("PAINTSW_BASELINE_TEST")) {
        rejects(word(good, 0, 0), "wrong magic");
        rejects(word(good, 8, 0x7fffffff), "excessive width");
        rejects(word(good, 12, 0xffffffff), "negative height");
        rejects(word(word(good, 8, 32768), 12, 32768), "pixel budget");
        rejects(word(good, 20, 257), "layer count budget");
        rejects(word(good, 20, 0), "empty layer stack");
        rejects(word(word(word(good, 8, 32768), 12, 1024), 20, 4), "aggregate layer budget");
        rejects(word(good, 24, 0x7ffffffe), "huge string length rejected before allocation");
        rejects(word(good, 24, 3), "odd UTF-16 byte length");
        const int png = good.indexOf(QByteArray::fromHex("89504e470d0a1a0a"));
        check(png > 0, "locate embedded PNG");
        if (png > 0) {
            rejects(word(good, png - 4, 0), "null image marker");
            rejects(word(good, png + 16, 0x7fffffff), "huge PNG width rejected before decoding");
            rejects(word(good, png + 8, 0xffffffff), "invalid IHDR length");
        }
        for (int size = 0; size < good.size(); ++size)
            rejects(good.left(size), "truncated native document");
    }

    const QString raster = tmp.filePath("image.png");
    check(doc.save(raster), "atomic PNG save");
    check(!Document::readImage(raster).isNull(), "bounded PNG reader");
    QFile svg(tmp.filePath("renamed.png"));
    check(svg.open(QIODevice::WriteOnly), "create disguised format");
    svg.write("<svg xmlns='http://www.w3.org/2000/svg' width='2' height='2'/>");
    svg.close();
    check(Document::readImage(svg.fileName()).isNull(), "unadvertised format rejected");
    check(!doc.saveNative(tmp.path()), "failed save to directory reported");

    // A rejected save must leave an existing file untouched.
    check(write(good), "seed original file");
    doc.activeLayer()->setName(QString(ImageLimits::MaxNameBytes, QChar('x')));
    check(!doc.saveNative(path), "oversized name cannot create unreadable document");
    QFile original(path);
    check(original.open(QIODevice::ReadOnly) && original.readAll() == good, "original file preserved");
    std::printf("Security regression failures: %d\n", failures);
    return failures ? 1 : 0;
}
