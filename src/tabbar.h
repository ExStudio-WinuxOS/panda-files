#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>

class QMouseEvent;

class TabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit TabBar(QWidget *parent = 0);
    void finishMouseMoveEvent();
    void releaseMouse();

    void setDetachable(bool detachable) {
        detachable_ = detachable;
    }

Q_SIGNALS:
    void tabDetached();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    // from qtabbar.cpp
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);

private:
    QPoint dragStartPosition_;
    bool dragStarted_;
    bool detachable_;
};

#endif // TABBAR_H
