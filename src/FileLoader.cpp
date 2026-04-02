/*
 * This file is part of Notepad Next.
 * Copyright 2019 Justin Dailey
 *
 * Notepad Next is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Notepad Next is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Notepad Next.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "FileLoader.h"
#include "ErrorHandler.h"
#include <QFileInfo>
#include <QDebug>
#include <QTextStream>
#include <QStringDecoder>

// BOM byte sequences
const QByteArray FileLoader::BOM_UTF8    = QByteArray::fromHex("EFBBBF");
const QByteArray FileLoader::BOM_UTF16LE = QByteArray::fromHex("FFFE");
const QByteArray FileLoader::BOM_UTF16BE = QByteArray::fromHex("FEFF");

FileLoader::FileMetadata* FileLoader::loadFile(QFile &file,
                                                std::function<bool(const QByteArray&)> chunkHandler,
                                                int chunkSize)
{
    if (!file.exists()) {
        ErrorHandler::logError("FileLoader",
                              QString("Cannot read \"%1\"").arg(file.fileName()),
                              "File doesn't exist",
                              ErrorHandler::Severity::Warning);
        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        ErrorHandler::logError("FileLoader",
                              QString("Opening \"%1\"").arg(file.fileName()),
                              QString("Error code %1: %2").arg(file.error()).arg(file.errorString()),
                              ErrorHandler::Severity::Critical);
        return nullptr;
    }

    auto metadata = new FileMetadata();
    metadata->isReadOnly = !QFileInfo(file).isWritable();

    QByteArray chunk;
    qint64 bytesRead;
    bool firstRead = true;

    do {
        // Read a chunk
        chunk.resize(chunkSize);
        bytesRead = file.read(chunk.data(), chunkSize);
        chunk.resize(bytesRead);

        if (bytesRead == -1) {
            ErrorHandler::logError("FileLoader",
                                  QString("Reading file \"%1\"").arg(file.fileName()),
                                  QString("Error code %1: %2").arg(file.error()).arg(file.errorString()),
                                  ErrorHandler::Severity::Critical);
            file.close();
            delete metadata;
            return nullptr;
        }

        metadata->totalBytesRead += bytesRead;

        // Detect and remove BOM on first read
        if (firstRead) {
            firstRead = false;
            BomType detectedBom = detectBom(chunk);
            metadata->bom = detectedBom;

            if (detectedBom != BomType::None) {
                qDebug("FileLoader: BOM detected - type %d", static_cast<int>(detectedBom));
            }

            // UTF-16 files need special handling with full file conversion
            if (detectedBom == BomType::Utf16BE || detectedBom == BomType::Utf16LE) {
                file.close();
                delete metadata;
                qInfo("FileLoader: UTF-16 detected, switching to conversion mode");
                return loadUtf16File(file, detectedBom, chunkHandler, chunkSize);
            }

            // Remove BOM from UTF-8 files
            if (detectedBom == BomType::Utf8) {
                removeBom(chunk, detectedBom);
            }
        }

        // Call the chunk handler
        if (!chunkHandler(chunk)) {
            ErrorHandler::logError("FileLoader",
                                  "Processing file chunk",
                                  "Chunk handler returned false, aborting load",
                                  ErrorHandler::Severity::Critical);
            file.close();
            delete metadata;
            return nullptr;
        }

    } while (!file.atEnd() && bytesRead > 0);

    file.close();

    qDebug("FileLoader: Successfully loaded %lld bytes from \"%s\"",
           metadata->totalBytesRead, qUtf8Printable(file.fileName()));

    return metadata;
}

FileLoader::BomType FileLoader::detectBom(const QByteArray &data)
{
    if (data.startsWith(BOM_UTF8))    return BomType::Utf8;
    if (data.startsWith(BOM_UTF16LE)) return BomType::Utf16LE;
    if (data.startsWith(BOM_UTF16BE)) return BomType::Utf16BE;

    return BomType::None;
}

QByteArray FileLoader::bomData(BomType bom)
{
    switch (bom) {
    case BomType::Utf8:    return BOM_UTF8;
    case BomType::Utf16LE: return BOM_UTF16LE;
    case BomType::Utf16BE: return BOM_UTF16BE;
    case BomType::None:    return QByteArray();
    }
    return QByteArray();
}

int FileLoader::bomLength(BomType bom)
{
    switch (bom) {
    case BomType::Utf8:    return BOM_UTF8.length();
    case BomType::Utf16LE: return BOM_UTF16LE.length();
    case BomType::Utf16BE: return BOM_UTF16BE.length();
    case BomType::None:    return 0;
    }
    return 0;
}

void FileLoader::removeBom(QByteArray &data, BomType bom)
{
    if (bom != BomType::None) {
        int length = bomLength(bom);
        if (data.size() >= length) {
            data.remove(0, length);
        }
    }
}

QByteArray FileLoader::convertUtf16ToUtf8(const QByteArray &data, BomType bom)
{
    if (bom != BomType::Utf16LE && bom != BomType::Utf16BE) {
        ErrorHandler::logError("FileLoader",
                              "UTF-16 to UTF-8 conversion",
                              QString("Invalid BOM type: %1").arg(static_cast<int>(bom)),
                              ErrorHandler::Severity::Critical);
        return QByteArray();
    }

    // Remove BOM
    QByteArray dataWithoutBom = data;
    removeBom(dataWithoutBom, bom);

    // UTF-16 characters are 2 bytes each (or 4 for surrogate pairs)
    // Make sure we have complete characters
    if (dataWithoutBom.size() % 2 != 0) {
        ErrorHandler::logError("FileLoader",
                              "UTF-16 validation",
                              QString("UTF-16 data has odd byte count (%1), may be corrupted").arg(dataWithoutBom.size()),
                              ErrorHandler::Severity::Warning);
    }

    QString converted;

    if (bom == BomType::Utf16LE) {
        // Little Endian
        const char16_t *utf16Data = reinterpret_cast<const char16_t*>(dataWithoutBom.constData());
        int charCount = dataWithoutBom.size() / 2;
        converted = QString::fromUtf16(utf16Data, charCount);
    } else {
        // Big Endian - need to swap bytes
        QByteArray swapped;
        swapped.reserve(dataWithoutBom.size());
        for (int i = 0; i < dataWithoutBom.size(); i += 2) {
            if (i + 1 < dataWithoutBom.size()) {
                swapped.append(dataWithoutBom[i + 1]);
                swapped.append(dataWithoutBom[i]);
            }
        }
        const char16_t *utf16Data = reinterpret_cast<const char16_t*>(swapped.constData());
        int charCount = swapped.size() / 2;
        converted = QString::fromUtf16(utf16Data, charCount);
    }

    return converted.toUtf8();
}

FileLoader::FileMetadata* FileLoader::loadUtf16File(QFile &file, BomType bomType,
                                                     std::function<bool(const QByteArray&)> chunkHandler,
                                                     int chunkSize)
{
    qInfo("FileLoader: Loading UTF-16 file with conversion to UTF-8");

    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly)) {
            ErrorHandler::logError("FileLoader",
                                  QString("Opening UTF-16 file \"%1\"").arg(file.fileName()),
                                  QString("Error code %1: %2").arg(file.error()).arg(file.errorString()),
                                  ErrorHandler::Severity::Critical);
            return nullptr;
        }
    }

    // Read entire file (UTF-16 files need to be read completely for proper conversion)
    QByteArray allData = file.readAll();
    file.close();

    if (allData.isEmpty()) {
        ErrorHandler::logError("FileLoader",
                              QString("Reading UTF-16 file \"%1\"").arg(file.fileName()),
                              "File is empty",
                              ErrorHandler::Severity::Warning);
        return nullptr;
    }

    auto metadata = new FileMetadata();
    metadata->bom = bomType;
    metadata->totalBytesRead = allData.size();
    metadata->isReadOnly = !QFileInfo(file).isWritable();
    metadata->wasConverted = true;

    // Convert to UTF-8
    QByteArray utf8Data = convertUtf16ToUtf8(allData, bomType);

    if (utf8Data.isEmpty()) {
        ErrorHandler::logError("FileLoader",
                              QString("Converting UTF-16 file \"%1\"").arg(file.fileName()),
                              "UTF-16 to UTF-8 conversion failed",
                              ErrorHandler::Severity::Critical);
        delete metadata;
        return nullptr;
    }

    qInfo("FileLoader: Converted UTF-16 (%lld bytes) to UTF-8 (%lld bytes)",
          allData.size(), utf8Data.size());

    // Now send the UTF-8 data in chunks to the handler
    qint64 offset = 0;
    while (offset < utf8Data.size()) {
        qint64 remaining = utf8Data.size() - offset;
        qint64 chunkLen = qMin(static_cast<qint64>(chunkSize), remaining);

        QByteArray chunk = utf8Data.mid(offset, chunkLen);

        if (!chunkHandler(chunk)) {
            ErrorHandler::logError("FileLoader",
                                  "Processing UTF-16 file chunk",
                                  "Chunk handler returned false during UTF-16 file processing",
                                  ErrorHandler::Severity::Critical);
            delete metadata;
            return nullptr;
        }

        offset += chunkLen;
    }

    return metadata;
}
