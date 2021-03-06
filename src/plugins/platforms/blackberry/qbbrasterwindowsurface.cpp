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

// #define QBBRASTERWINDOWSURFACE_DEBUG


#include "qbbrasterwindowsurface.h"
#include "qbbwindow.h"

#include <QtGui/QWidget>
#include <QDebug>

#include <errno.h>

QT_BEGIN_NAMESPACE

QBBRasterWindowSurface::QBBRasterWindowSurface(QWidget *window)
    : QWindowSurface(window)
{
#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::QBBRasterWindowSurface - w=" << window;
#endif

    // save platform window associated with widget
    mPlatformWindow = static_cast<QBBWindow*>(window->platformWindow());
}

QBBRasterWindowSurface::~QBBRasterWindowSurface()
{
#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::~QBBRasterWindowSurface - w=" << window();
#endif
}

QPaintDevice *QBBRasterWindowSurface::paintDevice()
{
    return mPlatformWindow->renderBuffer().image();
}

void QBBRasterWindowSurface::flush(QWidget *widget, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(widget);
    Q_UNUSED(offset);

#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::flush - w=" << window();
#endif

    // visit all pending scroll operations
    for (int i = mScrollOpList.size() - 1; i >= 0; i--) {

        // do the scroll operation
        ScrollOp& op = mScrollOpList[i];
        QRegion srcArea = op.totalArea.intersected( op.totalArea.translated(-op.dx, -op.dy) );
        mPlatformWindow->scroll(srcArea, op.dx, op.dy);
    }

    // clear all pending scroll operations
    mScrollOpList.clear();

    // update the display with newly rendered content
    mPlatformWindow->post(region);
}

void QBBRasterWindowSurface::resize(const QSize &size)
{
#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::resize - w=" << window() << ", s=" << size;
#endif

    // call parent method
    QWindowSurface::resize(size);

    // NOTE: defer resizing window buffers until next paint as
    // resize() can be called multiple times before a paint occurs
}

bool QBBRasterWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::scroll - w=" << window();
#endif

    // calculate entire region affected by scroll operation (src + dst)
    QRegion totalArea = area.translated(dx, dy);
    totalArea += area;

    // visit all pending scroll operations
    for (int i = mScrollOpList.size() - 1; i >= 0; i--) {

        ScrollOp& op = mScrollOpList[i];
        if (op.totalArea == totalArea) {
            // same area is being scrolled again - update delta
            op.dx += dx;
            op.dy += dy;
            return true;
        } else if (op.totalArea.intersects(totalArea)) {
            // current scroll overlaps previous scroll but is
            // not equal in area - just paint everything
            qWarning("QBB: pending scroll operations overlap but not equal");
            return false;
        }
    }

    // create new scroll operation
    mScrollOpList.append( ScrollOp(totalArea, dx, dy) );
    return true;
}

void QBBRasterWindowSurface::beginPaint(const QRegion &region)
{
    Q_UNUSED(region);

#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::beginPaint - w=" << window();
#endif

    // resize window buffers if surface resized
    QSize s = size();
    if (s != mPlatformWindow->bufferSize()) {
        mPlatformWindow->setBufferSize(s);
    }
}

void QBBRasterWindowSurface::endPaint(const QRegion &region)
{
    Q_UNUSED(region);
#if defined(QBBRASTERWINDOWSURFACE_DEBUG)
    qDebug() << "QBBRasterWindowSurface::endPaint - w=" << window();
#endif
}

QT_END_NAMESPACE
