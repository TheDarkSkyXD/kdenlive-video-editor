/*
    SPDX-FileCopyrightText: 2021 Jean-Baptiste Mardelle <jb@kdenlive.org>

SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include "abstracttask.h"

class QProcess;

class ProxyTask : public AbstractTask
{
public:
    ProxyTask(const ObjectId &owner, QObject* object);
    static void start(const ObjectId &owner, QObject* object, bool force = false);

protected:
    void run() override;

private Q_SLOTS:
    void processLogInfo();

private:
    int m_jobDuration{0};
    bool m_isFfmpegJob;
    QProcess *m_jobProcess;
    QString m_errorMessage;
    QString m_logDetails;
};
