
#ifndef VIEWFRAME_H
#define VIEWFRAME_H

#include <QFrame>
#include <QStackedWidget>
#include "tabbar.h"

class ViewFrame : public QFrame
{
    Q_OBJECT

public:
    ViewFrame(QWidget* parent = nullptr);
    ~ViewFrame() {};

    void createTopBar(bool usePathButtons);
    void removeTopBar();

    QWidget *topBar() const {
        return topBar_;
    }
    TabBar *tabBar() const {
        return tabBar_;
    }
    QStackedWidget *stackedWidget() const {
        return stackedWidget_;
    }

private:
    QWidget* topBar_;
    TabBar* tabBar_;
    QStackedWidget* stackedWidget_;
};

#endif // VIEWFRAME_H
