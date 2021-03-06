/****************************************************************************
**
** Copyright (C) 2011 Research In Motion Limited
**
** This file is part of the plugins of the Qt Toolkit.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

//#define QBBINTEGRATION_DEBUG

#include "qbbintegration.h"
#include "qbbinputcontext.h"
#include "qbbeventthread.h"
#include "qbbglcontext.h"
#include "qbbglwindowsurface.h"
#include "qbbnavigatorthread.h"
#include "qbbrasterwindowsurface.h"
#include "qbbscreen.h"
#include "qbbwindow.h"
#include "qbbvirtualkeyboard.h"
#include "qgenericunixfontdatabase.h"
#include "qbbclipboard.h"
#include "qbbglcontext.h"

#include "qapplication.h"
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtGui/QPlatformWindow>
#include <QtGui/QWindowSystemInterface>
#include <QtOpenGL/private/qpixmapdata_gl_p.h>
#include <QDebug>

#include <errno.h>

QT_BEGIN_NAMESPACE

QBBIntegration::QBBIntegration() :
    mFontDb(new QGenericUnixFontDatabase()),
    mPaintUsingOpenGL(getenv("QBB_USE_OPENGL") != NULL)
{
    if (mPaintUsingOpenGL) {
        // Set default window API to OpenGL
        QPlatformWindowFormat format = QPlatformWindowFormat::defaultFormat();
        format.setWindowApi(QPlatformWindowFormat::OpenGL);
        QPlatformWindowFormat::setDefaultFormat(format);
    }

    // initialize global OpenGL resources
    QBBGLContext::initialize();

    // open connection to QNX composition manager
    errno = 0;
    int result = screen_create_context(&mContext, SCREEN_APPLICATION_CONTEXT);
    if (result != 0) {
        qFatal("QBB: failed to connect to composition manager, errno=%d", errno);
    }

    // Create displays for all possible screens (which may not be attached)
    QBBScreen::createDisplays(mContext);

    // create/start event thread
    mEventThread = new QBBEventThread(mContext, *QBBScreen::primaryDisplay());
    mEventThread->start();

    // create/start navigator thread
    mNavigatorThread = new QBBNavigatorThread(*QBBScreen::primaryDisplay());
    mNavigatorThread->start();

    // create/start the keyboard class.
    QBBVirtualKeyboard::instance();

    // Set up the input context
    qApp->setInputContext(new QBBInputContext(qApp));
}

QBBIntegration::~QBBIntegration()
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBB: platform plugin shutdown begin";
#endif
    // destroy the keyboard class.
    QBBVirtualKeyboard::destroy();

    // stop/destroy event thread
    delete mEventThread;

    // stop/destroy navigator thread
    delete mNavigatorThread;

    // destroy all displays
    QBBScreen::destroyDisplays();

    // close connection to QNX composition manager
    screen_destroy_context(mContext);

    // cleanup global OpenGL resources
    QBBGLContext::shutdown();

#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBB: platform plugin shutdown end";
#endif
}

bool QBBIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps: return true;
#if defined(QT_OPENGL_ES)
    case OpenGL: return true;
#endif
    default: return QPlatformIntegration::hasCapability(cap);
    }
}

QPixmapData *QBBIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    if (paintUsingOpenGL())
        return new QGLPixmapData(type);
    else
        return new QRasterPixmapData(type);
}

QPlatformWindow *QBBIntegration::createPlatformWindow(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);

    // New windows are created on the primary display.
    return new QBBWindow(widget, mContext);
}

QWindowSurface *QBBIntegration::createWindowSurface(QWidget *widget, WId winId) const
{
    Q_UNUSED(winId);
    if (paintUsingOpenGL())
        return new QBBGLWindowSurface(widget);
    else
        return new QBBRasterWindowSurface(widget);
}

void QBBIntegration::moveToScreen(QWidget *window, int screen)
{
#if defined(QBBINTEGRATION_DEBUG)
    qDebug() << "QBBIntegration::moveToScreen - w=" << window << ", s=" << screen;
#endif

    // get platform window used by widget
    QBBWindow* platformWindow = static_cast<QBBWindow*>(window->platformWindow());

    // lookup platform screen by index
    QBBScreen* platformScreen = static_cast<QBBScreen*>(QBBScreen::screens().at(screen));

    // move the platform window to the platform screen
    platformWindow->setScreen(platformScreen);
}

QList<QPlatformScreen *> QBBIntegration::screens() const
{
    return QBBScreen::screens();
}

#ifndef QT_NO_CLIPBOARD
QPlatformClipboard *QBBIntegration::clipboard() const
{
    static QPlatformClipboard *clipboard = 0;
    if (!clipboard) {
        clipboard = static_cast<QPlatformClipboard *>(new QBBClipboard);
    }
    return clipboard;
}
#endif

QT_END_NAMESPACE
