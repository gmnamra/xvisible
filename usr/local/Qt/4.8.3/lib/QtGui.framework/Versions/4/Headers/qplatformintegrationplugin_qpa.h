/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QPLATFORMINTEGRATIONPLUGIN_H
#define QPLATFORMINTEGRATIONPLUGIN_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QPlatformIntegration;

struct QPlatformIntegrationFactoryInterface : public QFactoryInterface
{
    virtual QPlatformIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

#define QPlatformIntegrationFactoryInterface_iid "com.nokia.Qt.QPlatformIntegrationFactoryInterface"

Q_DECLARE_INTERFACE(QPlatformIntegrationFactoryInterface, QPlatformIntegrationFactoryInterface_iid)

class Q_GUI_EXPORT QPlatformIntegrationPlugin : public QObject, public QPlatformIntegrationFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QPlatformIntegrationFactoryInterface:QFactoryInterface)
public:
    explicit QPlatformIntegrationPlugin(QObject *parent = 0);
    ~QPlatformIntegrationPlugin();

    virtual QStringList keys() const = 0;
    virtual QPlatformIntegration *create(const QString &key, const QStringList &paramList) = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPLATFORMINTEGRATIONPLUGIN_H
