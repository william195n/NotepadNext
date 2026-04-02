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
#include <QFileInfo>
#include <QDebug>

// BOM byte sequences
const QByteArray FileLoader::BOM_UTF8    = QByteArray::fromHex("EFBBBF");
const QByteArray FileLoader::BOM_UTF16LE = QByteArray::fromHex("FFFE");
const QByteArray FileLoader::BOM_UTF16BE = QByteArray::fromHex("FEFF");

FileLoader::FileMetadata* FileLoader::loadFile(QFile &file,
                                                std::function<bool(const QByteArray&)> chunkHandler,
                                                int chunkSize)
{
    if (!file.exists()) {
        qWarning("FileLoader: Cannot read \"%s\": doesn't exist", qUtf8Printable(file.fileName()));
        return nullptr;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("FileLoader: QFile::open() failed when opening \"%s\" - error code %d: %s",
                 qUtf8Printable(file.fileName()), file.error(), qUtf8Printable(file.errorString()));
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
            qWarning("FileLoader: Error reading file \"%s\" - error code %d: %s",
                     qUtf8Printable(file.fileName()), file.error(), qUtf8Printable(file.errorString()));
            file.close();
            delete metadata;
            return nullptr;
        }

        metadata->totalBytesRead += bytesRead;

        // Detect and remove BOM on first read
        if (firstRead) {
            firstRead = false;
            metadata->bom = detectBom(chunk);

            if (metadata->bom != BomType::None) {
                qDebug("FileLoader: BOM detected - type %d", static_cast<int>(metadata->bom));
            }

            // Remove BOM from UTF-8 files
            if (metadata->bom == BomType::Utf8) {
                removeBom(chunk, metadata->bom);
            }

            // UTF-16 handling would go here
            if (metadata->bom == BomType::Utf16BE || metadata->bom == BomType::Utf16LE) {
                qWarning("FileLoader: UTF-16 files not fully supported yet");
                // Could implement conversion to UTF-8 here in the future
            }
        }

        // Call the chunk handler
        if (!chunkHandler(chunk)) {
            qWarning("FileLoader: Chunk handler returned false, aborting load");
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
