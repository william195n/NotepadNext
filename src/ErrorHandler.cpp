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


#include "ErrorHandler.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

bool ErrorHandler::handleFileError(QWidget *parent,
                                   const QString &operation,
                                   const QString &filePath,
                                   QFileDevice::FileError error,
                                   Severity severity)
{
    // Log the error first (always)
    QString fileName = QFileInfo(filePath).fileName();
    QString errorStr = fileErrorToString(error);

    logError("FileOperation",
             QString("%1 file '%2'").arg(operation, fileName),
             QString("Error code %1: %2").arg(static_cast<int>(error)).arg(errorStr),
             severity);

    // If no error, return success
    if (error == QFileDevice::NoError) {
        return true;
    }

    // Show user-friendly message
    QString title;
    QString message;
    QMessageBox::Icon icon;

    switch (severity) {
    case Severity::Info:
        title = QObject::tr("Information");
        icon = QMessageBox::Information;
        break;
    case Severity::Warning:
        title = QObject::tr("Warning");
        icon = QMessageBox::Warning;
        break;
    case Severity::Critical:
    default:
        title = QObject::tr("Error");
        icon = QMessageBox::Critical;
        break;
    }

    message = QObject::tr("Failed %1 <b>%2</b><br><br>Error: %3")
                  .arg(operation, fileName, errorStr);

    // For permission errors, add helpful hint
    if (error == QFileDevice::PermissionsError) {
        message += "<br><br>" + QObject::tr("Please check that you have the necessary permissions.");
    }

    // For resource errors, add helpful hint
    if (error == QFileDevice::ResourceError) {
        message += "<br><br>" + QObject::tr("Please check that the file is not open in another program.");
    }

    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();

    return false;
}

QString ErrorHandler::fileErrorToString(QFileDevice::FileError error)
{
    switch (error) {
    case QFileDevice::NoError:
        return QObject::tr("No error");
    case QFileDevice::ReadError:
        return QObject::tr("Read error");
    case QFileDevice::WriteError:
        return QObject::tr("Write error");
    case QFileDevice::FatalError:
        return QObject::tr("Fatal error");
    case QFileDevice::ResourceError:
        return QObject::tr("Resource error (file may be locked)");
    case QFileDevice::OpenError:
        return QObject::tr("Open error");
    case QFileDevice::AbortError:
        return QObject::tr("Abort error");
    case QFileDevice::TimeOutError:
        return QObject::tr("Timeout error");
    case QFileDevice::UnspecifiedError:
        return QObject::tr("Unspecified error");
    case QFileDevice::RemoveError:
        return QObject::tr("Remove error");
    case QFileDevice::RenameError:
        return QObject::tr("Rename error");
    case QFileDevice::PositionError:
        return QObject::tr("Position error");
    case QFileDevice::ResizeError:
        return QObject::tr("Resize error");
    case QFileDevice::PermissionsError:
        return QObject::tr("Permissions error");
    case QFileDevice::CopyError:
        return QObject::tr("Copy error");
    default:
        return QObject::tr("Unknown error (%1)").arg(static_cast<int>(error));
    }
}

void ErrorHandler::logError(const QString &context,
                            const QString &operation,
                            const QString &details,
                            Severity severity)
{
    QString prefix;
    switch (severity) {
    case Severity::Info:
        prefix = "INFO";
        qInfo("%s [%s]: %s - %s",
              qPrintable(prefix),
              qPrintable(context),
              qPrintable(operation),
              qPrintable(details));
        break;
    case Severity::Warning:
        prefix = "WARNING";
        qWarning("%s [%s]: %s - %s",
                qPrintable(prefix),
                qPrintable(context),
                qPrintable(operation),
                qPrintable(details));
        break;
    case Severity::Critical:
        prefix = "CRITICAL";
        qCritical("%s [%s]: %s - %s",
                 qPrintable(prefix),
                 qPrintable(context),
                 qPrintable(operation),
                 qPrintable(details));
        break;
    }
}

void ErrorHandler::showError(QWidget *parent,
                             const QString &title,
                             const QString &message,
                             const QString &details,
                             Severity severity)
{
    // Log first
    logError("Application", title, message + (details.isEmpty() ? "" : " - " + details), severity);

    // Determine icon
    QMessageBox::Icon icon;
    switch (severity) {
    case Severity::Info:
        icon = QMessageBox::Information;
        break;
    case Severity::Warning:
        icon = QMessageBox::Warning;
        break;
    case Severity::Critical:
    default:
        icon = QMessageBox::Critical;
        break;
    }

    // Show message box
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    if (!details.isEmpty()) {
        msgBox.setInformativeText(details);
    }
    msgBox.setIcon(icon);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}
