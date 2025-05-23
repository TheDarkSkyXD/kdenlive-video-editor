/*
    SPDX-FileCopyrightText: 2018 Nicolas Carion
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QObject>
#include <QtQml>

/** @class ClipboardProxy
    @brief Provides an interface to the clipboard, to use directly from QML
    Inspired by https://stackoverflow.com/questions/40092352/passing-qclipboard-to-qml
 */
class ClipboardProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes NOTIFY changed)

    QML_ELEMENT
    QML_SINGLETON
public:
    explicit ClipboardProxy(QObject *parent = nullptr);

    QStringList mimeTypes() const;

Q_SIGNALS:
    void changed();
};
