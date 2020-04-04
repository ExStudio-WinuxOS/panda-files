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
#include <QTabBar>

#include "settings.h"
#include "launcher.h"
#include "tabbar.h"
#include "lib/core/filepath.h"
#include "lib/core/bookmarks.h"

namespace Fm {
class PathEdit;
class PathBar;
}

class ViewFrame : public QFrame
{
    Q_OBJECT

public:
    ViewFrame(QWidget* parent = nullptr);
    ~ViewFrame() {};

    void createTopBar(bool usePathButtons);
    void removeTopBar();

    QWidget* getTopBar() const {
        return topBar_;
    }
    TabBar* getTabBar() const {
        return tabBar_;
    }
    QStackedWidget* getStackedWidget() const {
        return stackedWidget_;
    }

private:
    QWidget* topBar_;
    TabBar* tabBar_;
    QStackedWidget* stackedWidget_;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Fm::FilePath path = Fm::FilePath());
    ~MainWindow();

    void chdir(Fm::FilePath path, ViewFrame* viewFrame);
    void chdir(Fm::FilePath path) {
        chdir(path, m_activeViewFrame);
    }

    int addTab(Fm::FilePath path, ViewFrame* viewFrame);
    int addTab(Fm::FilePath path) {
        return addTab(path, m_activeViewFrame);
    }

    void updateFromSettings(Settings& settings) {}
    static MainWindow* lastActive() {
        return m_lastActive;
    }

protected:
    bool event(QEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private:
    static MainWindow *m_lastActive;

    Fm::PathEdit *m_pathEntry;
    Fm::PathBar *m_pathBar;

    std::shared_ptr<Fm::Bookmarks> m_bookmarks;
    ViewFrame *m_activeViewFrame;

    Launcher m_fileLauncher;
};

#endif
