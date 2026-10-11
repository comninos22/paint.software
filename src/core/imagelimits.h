#pragma once

#include <QSize>
#include <QtGlobal>

// Limits for untrusted files, before allocating pixels or stream strings.
namespace ImageLimits {
inline constexpr int MaxDimension = 32768;
inline constexpr qint64 MaxPixels = 32LL * 1024 * 1024;
inline constexpr qint64 MaxDocumentPixels = 128LL * 1024 * 1024;
inline constexpr qint64 MaxFileBytes = 512LL * 1024 * 1024;
inline constexpr int MaxLayers = 256;
inline constexpr int MaxNameBytes = 8192;

inline bool validSize(qint64 width, qint64 height) {
    return width > 0 && height > 0 && width <= MaxDimension && height <= MaxDimension
           && width * height <= MaxPixels;
}
inline bool validSize(const QSize &size) { return validSize(size.width(), size.height()); }
}
