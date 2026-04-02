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


#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QString>
#include <QFileDevice>
#include <QWidget>

/**
 * @brief ErrorHandler provides unified error handling across the application
 *
 * Design principles:
 * 1. All critical errors (file I/O, data loss) are logged AND shown to user
 * 2. All errors return appropriate failure values (false, nullptr, error codes)
 * 3. User-facing messages are clear, actionable, and translated
 * 4. Technical details are logged but not shown to end users
 */
class ErrorHandler
{
public:
    /**
     * @brief Severity levels for error reporting
     */
    enum class Severity {
        Info,       // Informational, no user action needed
        Warning,    // Warning, user should be aware
        Critical    // Critical error, may cause data loss
    };

    /**
     * @brief Handle file operation errors with user notification
     * @param parent Parent widget for message box
     * @param operation Description of the operation (e.g., "opening", "saving")
     * @param filePath Path to the file
     * @param error The file error code
     * @param severity Error severity level
     * @return true if user wants to retry, false otherwise
     */
    static bool handleFileError(QWidget *parent,
                                const QString &operation,
                                const QString &filePath,
                                QFileDevice::FileError error,
                                Severity severity = Severity::Critical);

    /**
     * @brief Convert QFileDevice::FileError to user-friendly message
     * @param error The error code
     * @return Translated error message
     */
    static QString fileErrorToString(QFileDevice::FileError error);

    /**
     * @brief Log error to console with consistent format
     * @param context Source of error (e.g., "FileLoader", "ScintillaNext")
     * @param operation What operation failed
     * @param details Technical details
     * @param severity Error severity
     */
    static void logError(const QString &context,
                        const QString &operation,
                        const QString &details,
                        Severity severity = Severity::Warning);

    /**
     * @brief Show generic error dialog to user
     * @param parent Parent widget
     * @param title Dialog title
     * @param message Main error message
     * @param details Optional technical details
     * @param severity Error severity
     */
    static void showError(QWidget *parent,
                         const QString &title,
                         const QString &message,
                         const QString &details = QString(),
                         Severity severity = Severity::Critical);
};

#endif // ERRORHANDLER_H
