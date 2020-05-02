/*
 * Copyright (C) 2020 PandaOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QPushButton>
#include <QSplitter>
#include <QTabBar>

#include "settings.h"
#include "launcher.h"
#include "viewframe.h"
#include "tabbar.h"
#include "tabpage.h"
#include "lib/core/filepath.h"
#include "lib/core/bookmarks.h"

namespace Fm {
class PathEdit;
class PathBar;
class SidePane;
}

class QHBoxLayout;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Fm::FilePath path = Fm::FilePath());
    ~MainWindow();

    void chdir(Fm::FilePath path);
    void addTabWithPage(Fm::FilePath path);

protected:
    void paintEvent(QPaintEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void initViewFrame();

    TabPage *currentPage();

    void updateCurrentPage();
    void updateTabBar();

    void onSplitterMoved(int pos, int index);
    void onPathBarChdir(const Fm::FilePath &dirPath);
    void onTabBarCurrentChanged(int index);
    void onTabBarCloseRequested(int index);
    void onTabBarTabMoved(int from, int to);
    void onTabBarClicked(int index);
    void onStackedWidgetWidgetRemoved(int index);
    void onSidePaneChdirRequested(int type, const Fm::FilePath &path);

    void onGoBackButtonClicked();
    void onForwardButtonClicked();
    void onIconViewButtonClicked();
    void onListViewButtonClicked();

private:
    static MainWindow *m_lastActive;

    QHBoxLayout *pathBarLayout_;

    QPushButton *goBackButton_;
    QPushButton *goForwardButton_;
    QPushButton *iconViewButton_;
    QPushButton *listViewButton_;

    Fm::PathEdit *pathEntry_;
    Fm::PathBar *pathBar_;
    Fm::SidePane *sidePane_;
    QSplitter *splitter_;
    ViewFrame *viewFrame_;

    std::shared_ptr<Fm::Bookmarks> m_bookmarks;

    Launcher fileLauncher_;

    int rightClickIndex_;
    bool splitView_;
};

#endif
