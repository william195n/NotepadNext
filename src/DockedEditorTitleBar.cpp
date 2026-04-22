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


#include "DockedEditorTitleBar.h"
#include <QHBoxLayout>
#include <QStyle>

DockedEditorTitleBar::DockedEditorTitleBar(ads::CDockAreaWidget* parent)
    : ads::CDockAreaTitleBar(parent)
{
    // Create the "New Tab" button (equivalent to Ctrl+N)
    // Similar to adding a trigger button in a database form
    newTabButton = new QPushButton("+", this);
    newTabButton->setObjectName("newTabButton");
    newTabButton->setFixedSize(22, 22);
    newTabButton->setToolTip(tr("New File (Ctrl+N)"));
    newTabButton->setFocusPolicy(Qt::NoFocus);

    // Set button style to make it look like a tab button
    // Like formatting a clickable UI element in Oracle Forms
    newTabButton->setStyleSheet(
        "QPushButton#newTabButton {"
        "    background: rgb(192, 192, 192);"
        "    border: 1px solid gray;"
        "    border-radius: 2px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    padding: 0px;"
        "    margin: 2px;"
        "}"
        "QPushButton#newTabButton:hover {"
        "    background: rgb(160, 160, 160);"
        "}"
        "QPushButton#newTabButton:pressed {"
        "    background: rgb(128, 128, 128);"
        "}"
    );

    // Connect button click to signal (like a database trigger)
    // When clicked, fires the newTabRequested signal
    connect(newTabButton, &QPushButton::clicked, this, &DockedEditorTitleBar::newTabRequested);

    // Insert the button into the title bar's layout
    // The title bar layout typically has: [tabs] [spacer] [close button]
    // We want: [tabs] [+] [spacer] [close button]
    QBoxLayout* layout = qobject_cast<QBoxLayout*>(this->layout());
    if (layout) {
        // Insert before the last item (which is usually the close button or spacer)
        // Count - 1 gives us the position just before the end
        int insertPos = layout->count() > 0 ? layout->count() - 1 : 0;
        layout->insertWidget(insertPos, newTabButton);
    }
}
