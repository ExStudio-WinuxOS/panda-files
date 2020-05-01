#include "tabbar.h"
#include <QPointer>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>

TabBar::TabBar(QWidget *parent)
  : QTabBar(parent),
    dragStarted_(false),
    detachable_(true)
{
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    QTabBar::mousePressEvent (event);

    if (detachable_){
        if (event->button() == Qt::LeftButton
        && tabAt(event->pos()) > -1) {
            dragStartPosition_ = event->pos();
        }
        dragStarted_ = false;
    }
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!detachable_) {
        QTabBar::mouseMoveEvent(event);
        return;
    }

    if (!dragStartPosition_.isNull()
       && (event->pos() - dragStartPosition_).manhattanLength() >= QApplication::startDragDistance()) {
        dragStarted_ = true;
    }

    if ((event->buttons() & Qt::LeftButton)
       && dragStarted_
       && !window()->geometry().contains(event->globalPos())) {
        if (currentIndex() == -1) {
            return;
        }

        QPointer<QDrag> drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(QStringLiteral("application/panda-files-tab"), QByteArray());
        drag->setMimeData(mimeData);
        int N = count();
        Qt::DropAction dragged = drag->exec(Qt::MoveAction);
        if (dragged != Qt::MoveAction) { // a tab is dropped outside all windows
            if (N > 1) {
                Q_EMIT tabDetached();
            } else {
                finishMouseMoveEvent();
            }
        } else { // a tab is dropped into another window
            if (count() == N) {
                releaseMouse(); // release the mouse if the drop isn't accepted
            }
        }
        event->accept();
        drag->deleteLater();
    } else {
        QTabBar::mouseMoveEvent(event);
    }
}

void TabBar::finishMouseMoveEvent()
{
    QMouseEvent finishingEvent(QEvent::MouseMove, QPoint(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    mouseMoveEvent(&finishingEvent);
}

void TabBar::releaseMouse()
{
    QMouseEvent releasingEvent(QEvent::MouseButtonRelease, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    mouseReleaseEvent(&releasingEvent);
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        int index = tabAt(event->pos());
        if (index != -1) {
            Q_EMIT tabCloseRequested(index);
        }
    }

    QTabBar::mouseReleaseEvent(event);
}

// Let the main window receive dragged tabs!
void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (detachable_ && event->mimeData()->hasFormat(QStringLiteral("application/panda-files-tab"))) {
        event->ignore();
    }
}
