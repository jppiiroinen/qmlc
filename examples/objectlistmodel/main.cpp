/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Copyright (C) 2014 Nomovok Ltd. All rights reserved.
** Contact: info@nomovok.com
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QGuiApplication>

#include <qqmlengine.h>
#include <qqmlcontext.h>
#include <qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

#include <private/qv4engine_p.h>
#include <private/qqmlengine_p.h>
#include <private/qv4assembler_p.h>
#include <private/qv4isel_masm_p.h>

#include "dataobject.h"
#include "qmcloader.h"
#include "qmcloadingmeasurer.h"
/*
   This example illustrates exposing a QList<QObject*> as a
   model in QML
*/

//![0]
int main(int argc, char ** argv)
{
    QGuiApplication app(argc, argv);

    QList<QObject*> dataList;
    dataList.append(new DataObject("Item 1", "red"));
    dataList.append(new DataObject("Item 2", "green"));
    dataList.append(new DataObject("Item 3", "blue"));
    dataList.append(new DataObject("Item 4", "yellow"));

    QQuickView view;

    view.setResizeMode(QQuickView::SizeRootObjectToView);
    QQmlContext *ctxt = view.rootContext();
     ctxt->setContextProperty("myModel", QVariant::fromValue(dataList));

//![0]

#if 1
    // add precompiled files
    QQmlEngine *engine = view.engine();
    QQmlEnginePrivate::get(engine)->v4engine()->iselFactory.reset(new QV4::JIT::ISelFactory);
    QmcLoader loader(engine);

    QmcLoadingMeasurer timer;
    timer.start();

    QQmlComponent *component = loader.loadComponent(":/view.qmc");

    if (!component) {
        qDebug() << "Could not load component";
        return -1;
    }
    if (!component->isReady()) {
        qDebug() << "Component is not ready";
        if (component->isError()) {
            foreach (const QQmlError &error, component->errors()) {
                qDebug() << error.toString();
            }
        }
        return -1;
    }
    timer.done();

    QObject *rootObject = component->create();
    if (!rootObject) {
        qDebug() << "Could not create root object";
        return -1;
    }
#else
    QQmlEngine *engine = view.engine();
    QQmlEnginePrivate::get(engine)->v4engine()->iselFactory.reset(new QV4::JIT::ISelFactory);
    QQmlComponent *component = new QQmlComponent(engine);


    QmcLoadingMeasurer timer(component);
    timer.start();
    QObject::connect(component, SIGNAL(progressChanged(qreal)),
                     &timer, SLOT(update()), Qt::DirectConnection);

    component->loadUrl(QUrl("view.qml"));
    if (!component) {
        qDebug() << "Could not load component";
        return -1;
    }
    if (!component->isReady()) {
        qDebug() << "Component is not ready";
        foreach (const QQmlError &error, component->errors()) {
            qDebug() << error.toString();
        }
        return -1;
    }

    QObject *rootObject = component->create();
    if (!rootObject) {
        qDebug() << "Could not create root object";
        return -1;
    }
#endif

 //   view.setSource(QUrl("qrc:view.qml"));
    view.setContent(component->url(), component, rootObject);

    view.show();

    int ret = app.exec();
    delete rootObject;
    delete component;
    return ret;
}

