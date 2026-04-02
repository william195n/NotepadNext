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


#ifndef FILELOADER_H
#define FILELOADER_H

#include <QByteArray>
#include <QFile>
#include <functional>

class ScintillaNext;

/**
 * @brief FileLoader handles low-level file reading operations
 *
 * This class separates file I/O concerns from the editor logic:
 * - BOM detection and removal
 * - Chunked reading for large files
 * - Encoding detection (future)
 * - Indentation detection (future)
 */
class FileLoader
{
public:
    enum class BomType {
        None,
        Utf8,
        Utf16LE,
        Utf16BE
    };

    struct FileMetadata {
        BomType bom = BomType::None;
        qint64 totalBytesRead = 0;
        bool isReadOnly = false;
    };

    /**
     * @brief Load a file and call the chunk handler for each chunk
     * @param file The file to read (must be closed)
     * @param chunkHandler Function called for each chunk of data
     * @param chunkSize Size of each chunk (default 4MB)
     * @return FileMetadata containing BOM type and other info, or nullptr on error
     */
    static FileMetadata* loadFile(QFile &file,
                                   std::function<bool(const QByteArray&)> chunkHandler,
                                   int chunkSize = 1024 * 1024 * 4);

    /**
     * @brief Detect BOM type from data
     */
    static BomType detectBom(const QByteArray &data);

    /**
     * @brief Get the byte sequence for a BOM type
     */
    static QByteArray bomData(BomType bom);

    /**
     * @brief Get the length of a BOM type
     */
    static int bomLength(BomType bom);

    /**
     * @brief Remove BOM from data if present
     */
    static void removeBom(QByteArray &data, BomType bom);

private:
    static const QByteArray BOM_UTF8;
    static const QByteArray BOM_UTF16LE;
    static const QByteArray BOM_UTF16BE;
};

#endif // FILELOADER_H
