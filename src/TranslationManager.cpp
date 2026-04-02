/*
 * This file is part of Notepad Next.
 * Copyright 2024 Justin Dailey
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

#include "TranslationManager.h"

#include <QApplication>
#include <QDirIterator>
#include <QDebug>

#include <memory>

QString TranslationFileNameToLocaleName(const QString &baseName)
{
    // return "zh_CN" from "NotepadNext_zh_CN"
    return baseName.mid(QApplication::applicationName().length() + 1); // +1 for the underscore
}


TranslationManager::TranslationManager(QObject *parent, const QString &path)
    : QObject(parent), path(path)
{
}

QStringList TranslationManager::availableTranslations() const
{
    QStringList translations;
    QDirIterator it(path);

    while (it.hasNext()) {
        it.next();

        if (it.fileName().startsWith(QApplication::applicationName())) { // Some are Qt translation files, not for the application
            QString localeName = TranslationFileNameToLocaleName(it.fileInfo().baseName());

            translations.append(localeName);
        }
    }

    return translations;
}

QString TranslationManager::FormatLocaleTerritoryAndLanguage(QLocale &locale)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    const QString territory = QLocale::countryToString(locale.country());
#else
    const QString territory = QLocale::territoryToString(locale.territory());
#endif
    const QString language = QLocale::languageToString(locale.language());
    return QStringLiteral("%1 (%2)").arg(language, territory);
}

void TranslationManager::loadSystemDefaultTranslation()
{
    // The wrong translation file may be loaded when passing Locale::system() to loadTranslation function, e.g. "zh_CN" translation file will be loaded when the locale is "en_US". It's probably a Qt bug.

    QLocale systemLocale = QLocale::system();
    QString systemLocaleName = systemLocale.name();

    // Debug logging to help diagnose Qt locale bug
    qInfo() << "=== System Locale Detection ===";
    qInfo() << "  System locale name:" << systemLocaleName;
    qInfo() << "  Language:" << QLocale::languageToString(systemLocale.language());
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    qInfo() << "  Country:" << QLocale::countryToString(systemLocale.country());
#else
    qInfo() << "  Territory:" << QLocale::territoryToString(systemLocale.territory());
#endif
    qInfo() << "  Script:" << QLocale::scriptToString(systemLocale.script());
    qInfo() << "  UI Languages:" << systemLocale.uiLanguages();

    // Create new QLocale from name to work around Qt bug
    QLocale actualLocale(systemLocaleName);
    qInfo() << "  Actual locale used for loading:" << actualLocale.name();

    if (actualLocale.name() != systemLocaleName) {
        qWarning() << "  WARNING: Locale name mismatch! System:" << systemLocaleName
                   << "Actual:" << actualLocale.name();
    }

    loadTranslation(actualLocale);
}

void TranslationManager::loadTranslation(QLocale locale)
{
    qInfo(Q_FUNC_INFO);

    qInfo() << "=== Loading Translation ===";
    qInfo() << "  Locale:" << locale.name();
    qInfo() << "  Language:" << QLocale::languageToString(locale.language());
    qInfo() << "  Translation path:" << path;

    const QStringList filenames{QStringList{QApplication::applicationName(), "qt", "qtbase"}};
    int successCount = 0;
    int failureCount = 0;

    for (const auto& filename : filenames) {
        qInfo() << "  Attempting to load:" << filename;

        std::unique_ptr<QTranslator> translator = std::make_unique<QTranslator>();

        if (translator->load(locale, filename, QStringLiteral("_"), path)) {
            QString filePath = translator->filePath();
            qInfo() << "    ✓ Loaded file:" << filePath;

            if (QCoreApplication::installTranslator(translator.get())) {
                qInfo() << "    ✓ Installed translator successfully";
                translators.append(translator.release());
                successCount++;
            } else {
                qWarning() << "    ✗ Failed to install translator for:" << filePath;
                failureCount++;
            }
        } else {
            qInfo() << "    ✗ Translation file not found or failed to load";
            failureCount++;
        }
    }

    qInfo() << "=== Translation Loading Summary ===";
    qInfo() << "  Successfully loaded:" << successCount << "file(s)";
    qInfo() << "  Failed to load:" << failureCount << "file(s)";

    if (successCount == 0) {
        qWarning() << "  WARNING: No translation files were loaded! Application will use default (English) strings.";
    }
}

void TranslationManager::loadTranslation(QString localeName)
{
    qInfo() << "Loading translation by locale name:" << localeName;

    QLocale locale(localeName);

    // Verify the locale was created correctly
    if (locale.name() != localeName && locale.name() != "C") {
        qWarning() << "  Locale name mismatch! Requested:" << localeName
                   << "Created:" << locale.name();
    }

    loadTranslation(locale);
}
