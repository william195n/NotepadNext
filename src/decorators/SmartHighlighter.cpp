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


#include "SmartHighlighter.h"
#include "NotepadNextApplication.h"

using namespace Scintilla;


SmartHighlighter::SmartHighlighter(ScintillaNext *editor) :
    EditorDecorator(editor)
{
    setObjectName("SmartHighlighter");

    indicator = editor->allocateIndicator("smart_highlighter");

    editor->indicSetFore(indicator, 0x00FF00);
    editor->indicSetStyle(indicator, INDIC_ROUNDBOX);
    editor->indicSetOutlineAlpha(indicator, 150);
    editor->indicSetAlpha(indicator, 100);
    editor->indicSetUnder(indicator, true);

    // Create a timer to delay highlighting and avoid frequent updates
    highlightTimer = new QTimer(this);
    highlightTimer->setSingleShot(true);
    highlightTimer->setInterval(150); // 150ms delay
    connect(highlightTimer, &QTimer::timeout, this, &SmartHighlighter::highlightCurrentView);
}

void SmartHighlighter::notify(const NotificationData *pscn)
{
    if (pscn->nmhdr.code == Notification::UpdateUI && (FlagSet(pscn->updated, Update::Content) || FlagSet(pscn->updated, Update::Selection))) {
        scheduleHighlight();
    }
}

void SmartHighlighter::scheduleHighlight()
{
    // Restart timer to delay highlighting
    highlightTimer->start();
}

bool SmartHighlighter::shouldHighlightWholeFile() const
{
    NotepadNextApplication *app = qobject_cast<NotepadNextApplication *>(qApp);
    int thresholdKB = app->getSettings()->smartHighlightFileSizeThresholdKB();
    int fileSizeKB = editor->length() / 1024;
    return fileSizeKB < thresholdKB;
}

void SmartHighlighter::highlightCurrentView()
{
    editor->setIndicatorCurrent(indicator);
    editor->indicatorClearRange(0, editor->length());

    if (editor->selectionEmpty()) {
        return;
    }

    const int mainSelection = editor->mainSelection();
    const int selectionStart = editor->selectionNStart(mainSelection);
    const int selectionEnd = editor->selectionNEnd(mainSelection);

    // Make sure the current selection is valid
    if (selectionStart == selectionEnd) {
        return;
    }

    const int curPos = editor->currentPos();
    const int wordStart = editor->wordStartPosition(curPos, true);
    const int wordEnd = editor->wordEndPosition(wordStart, true);

    // Make sure the selection is on word boundaries
    if (wordStart == wordEnd || wordStart != selectionStart || wordEnd != selectionEnd) {
        return;
    }

    const QByteArray selText = editor->get_text_range(selectionStart, selectionEnd);

    NotepadNextApplication *app = qobject_cast<NotepadNextApplication *>(qApp);
    int flags = SCFIND_WHOLEWORD;
    if (app->getSettings()->smartHighlightCaseSensitive()) {
        flags |= SCFIND_MATCHCASE;
    }

    // Determine search range based on file size
    Sci_PositionCR searchStart, searchEnd;
    if (shouldHighlightWholeFile()) {
        // Small file: highlight whole file (supports HighlightedScrollBar)
        searchStart = 0;
        searchEnd = (Sci_PositionCR)editor->length();
    } else {
        // Large file: only highlight visible area for performance
        const int firstLine = editor->firstVisibleLine();
        const int linesOnScreen = editor->linesOnScreen();
        // Add some buffer lines above and below for smooth scrolling
        const int bufferLines = 10;
        const int startLine = qMax(0, firstLine - bufferLines);
        const int endLine = qMin(editor->lineCount() - 1, firstLine + linesOnScreen + bufferLines);

        searchStart = editor->positionFromLine(startLine);
        searchEnd = editor->lineEndPosition(endLine);
    }

    Sci_TextToFind ttf {{searchStart, searchEnd}, selText.constData(), {-1, -1}};

    while (editor->send(SCI_FINDTEXT, flags, (sptr_t)&ttf) != -1) {
        editor->indicatorFillRange(ttf.chrgText.cpMin, ttf.chrgText.cpMax - ttf.chrgText.cpMin);
        ttf.chrg.cpMin = ttf.chrgText.cpMax;
    }
}
