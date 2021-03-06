/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "qt5nodeinstanceserver.h"


#include <QQuickItem>
#include <QQuickView>

#include <designersupport.h>
#include <addimportcontainer.h>
#include <createscenecommand.h>
#include <reparentinstancescommand.h>

namespace QmlDesigner {

Qt5NodeInstanceServer::Qt5NodeInstanceServer(NodeInstanceClientInterface *nodeInstanceClient)
    : NodeInstanceServer(nodeInstanceClient)
{
    DesignerSupport::activateDesignerMode();
}

Qt5NodeInstanceServer::~Qt5NodeInstanceServer()
{
    delete quickView();
}

QQuickView *Qt5NodeInstanceServer::quickView() const
{
    return m_quickView.data();
}

void Qt5NodeInstanceServer::initializeView()
{
    Q_ASSERT(!quickView());

    m_quickView = new QQuickView;
    DesignerSupport::createOpenGLContext(m_quickView.data());
}

QQmlView *Qt5NodeInstanceServer::declarativeView() const
{
    return 0;
}

QQmlEngine *Qt5NodeInstanceServer::engine() const
{
    if (quickView())
        return quickView()->engine();

    return 0;
}

void Qt5NodeInstanceServer::resizeCanvasSizeToRootItemSize()
{
}

void Qt5NodeInstanceServer::resetAllItems()
{
    foreach (QQuickItem *item, allItems())
        DesignerSupport::resetDirty(item);
}

void Qt5NodeInstanceServer::setupScene(const CreateSceneCommand &command)
{
    setupFileUrl(command.fileUrl());
    setupImports(command.imports());
    setupDummyData(command.fileUrl());

    setupInstances(command);
    quickView()->resize(rootNodeInstance().boundingRect().size().toSize());
}

QList<QQuickItem*> subItems(QQuickItem *parentItem)
{
    QList<QQuickItem*> itemList;
    itemList.append(parentItem->childItems());

    foreach (QQuickItem *childItem, parentItem->childItems())
        itemList.append(subItems(childItem));

    return itemList;
}

QList<QQuickItem*> Qt5NodeInstanceServer::allItems() const
{
    if (rootNodeInstance().isValid())
        return rootNodeInstance().allItemsRecursive();

    return QList<QQuickItem*>();
}

void Qt5NodeInstanceServer::refreshBindings()
{
    DesignerSupport::refreshExpressions(context());
}

DesignerSupport *Qt5NodeInstanceServer::designerSupport()
{
    return &m_designerSupport;
}

void Qt5NodeInstanceServer::createScene(const CreateSceneCommand &command)
{
    NodeInstanceServer::createScene(command);
}

void Qt5NodeInstanceServer::clearScene(const ClearSceneCommand &command)
{
    NodeInstanceServer::clearScene(command);
}

void Qt5NodeInstanceServer::reparentInstances(const ReparentInstancesCommand &command)
{
    NodeInstanceServer::reparentInstances(command.reparentInstances());
    startRenderTimer();
}


} // QmlDesigner
