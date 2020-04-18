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

    void chdir(Fm::FilePath path, ViewFrame* viewFrame);
    void chdir(Fm::FilePath path) {
        chdir(path, activeViewFrame_);
    }

    int addTab(Fm::FilePath path, ViewFrame* viewFrame);
    int addTab(Fm::FilePath path) {
        return addTab(path, activeViewFrame_);
    }

    TabPage* currentPage(ViewFrame* viewFrame) {
        return reinterpret_cast<TabPage*>(viewFrame->getStackedWidget()->currentWidget());
    }
    TabPage* currentPage() {
        return currentPage(activeViewFrame_);
    }

    void updateFromSettings(Settings& settings) {}
    static MainWindow* lastActive() {
        return m_lastActive;
    }

protected:
    bool event(QEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void updateUIForCurrentPage(bool setFocus = true);
    void updateViewMenuForCurrentPage();
    void createPathBar(bool usePathButtons);
    void addViewFrame(const Fm::FilePath& path);
    int addTabWithPage(TabPage* page, ViewFrame* viewFrame, Fm::FilePath path = Fm::FilePath());
    void closeTab(int index, ViewFrame *viewFrame);
    void closeTab(int index) {
        closeTab(index, activeViewFrame_);
    }
    ViewFrame* viewFrameForTabPage(TabPage* page);
    void updateEditSelectedActions();

private Q_SLOTS:
    void onPathEntryReturnPressed();
    void onPathBarChdir(const Fm::FilePath& dirPath);
    void onPathBarMiddleClickChdir(const Fm::FilePath &dirPath);

    void onSplitterMoved(int pos, int index);
    void onResetFocus();

    void onTabBarCloseRequested(int index);
    void onTabBarCurrentChanged(int index);
    void onTabBarTabMoved(int from, int to);
    void onTabBarClicked(int index);
    void tabContextMenu(const QPoint& pos);
    void detachTab();

    void onStackedWidgetWidgetRemoved(int index);

    void onTabPageTitleChanged(QString title);
    void onTabPageStatusChanged(int type, QString statusText);
    void onTabPageSortFilterChanged();

    void onSidePaneChdirRequested(int type, const Fm::FilePath &path);
    void onSidePaneOpenFolderInNewWindowRequested(const Fm::FilePath &path);
    void onSidePaneOpenFolderInNewTabRequested(const Fm::FilePath &path);
    void onSidePaneOpenFolderInTerminalRequested(const Fm::FilePath &path);
    void onSidePaneModeChanged(Fm::SidePane::Mode mode);

    void onSettingHiddenPlace(const QString& str, bool hide);

private:
    static MainWindow *m_lastActive;

    QHBoxLayout *pathBarLayout_;

    Fm::PathEdit *pathEntry_;
    Fm::PathBar *pathBar_;
    Fm::SidePane *sidePane_;
    QSplitter *splitter_;
    QSplitter *viewSplitter_;

    std::shared_ptr<Fm::Bookmarks> m_bookmarks;
    ViewFrame *activeViewFrame_;

    Launcher fileLauncher_;

    QToolBar *m_addressBar;

    int rightClickIndex_;
    bool splitView_;
};

#endif
