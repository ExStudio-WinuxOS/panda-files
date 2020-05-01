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

#include "mainwindow.h"
#include "application.h"
#include "settings.h"
#include "tabpage.h"

// Qt
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QToolButton>
#include <QShortcut>
#include <QKeySequence>
#include <QSettings>
#include <QMimeData>
#include <QApplication>
#include <QStandardPaths>
#include <QClipboard>
#include <QDebug>
#include <QEvent>

#include "lib/filemenu.h"
#include "lib/bookmarkaction.h"
#include "lib/fileoperation.h"
#include "lib/utilities.h"
#include "lib/filepropsdialog.h"
#include "lib/pathedit.h"
#include "lib/pathbar.h"
#include "lib/core/fileinfo.h"
#include "lib/mountoperation.h"

MainWindow *MainWindow::m_lastActive = nullptr;

MainWindow::MainWindow(Fm::FilePath path)
    : QMainWindow(),
      pathBarLayout_(new QHBoxLayout),
      pathEntry_(nullptr),
      pathBar_(new Fm::PathBar(this)),
      sidePane_(new Fm::SidePane),
      splitter_(new QSplitter(Qt::Horizontal)),
      viewFrame_(new ViewFrame),
      m_bookmarks(Fm::Bookmarks::globalInstance())
{
    QVBoxLayout *layout = new QVBoxLayout;
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    QWidget *pathBarWidget = new QWidget;
    pathBarLayout_->setMargin(0);
    pathBarLayout_->setSpacing(0);
    pathBarLayout_->addWidget(pathBar_);
    pathBarWidget->setLayout(pathBarLayout_);

    QWidget *topBarWidget = new QWidget;
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->addWidget(pathBarWidget);
    topBarLayout->setMargin(0);
    topBarLayout->setSpacing(0);
    topBarWidget->setFixedHeight(45);

    QWidget *contentWidget = new QWidget;
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(splitter_);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(topBarWidget);
    layout->addWidget(contentWidget);

    setAttribute(Qt::WA_DeleteOnClose);

    Settings &settings = static_cast<Application *>(qApp)->settings();
    splitView_ = settings.splitView();

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(sidePane_->sizePolicy().hasHeightForWidth());
    sidePane_->setSizePolicy(sizePolicy);

    // init sidepane
    sidePane_->setIconSize(QSize(settings.sidePaneIconSize(), settings.sidePaneIconSize()));
    sidePane_->setMode(Fm::SidePane::ModePlaces);
    sidePane_->restoreHiddenPlaces(settings.getHiddenPlaces());

    splitter_->setOrientation(Qt::Horizontal);
    splitter_->addWidget(sidePane_);
    splitter_->addWidget(viewFrame_);
    splitter_->setChildrenCollapsible(false);

    // setup the splitter
    splitter_->setStretchFactor(1, 1); // only the right pane can be stretched
    QList<int> sizes;
    sizes.append(settings.splitterPos());
    sizes.append(300);
    splitter_->setSizes(sizes);

    // size from settings
    resize(settings.windowWidth(), settings.windowHeight());
    if (settings.windowMaximized()) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }
    setWindowTitle(tr("File Manager"));

    initViewFrame();
    addTabWithPage(path);

    // detect change of splitter position
    connect(splitter_, &QSplitter::splitterMoved, this, &MainWindow::onSplitterMoved);
    connect(pathBar_, &Fm::PathBar::chdir, this, &MainWindow::onPathBarChdir);
    connect(sidePane_, &Fm::SidePane::chdirRequested, this, &MainWindow::onSidePaneChdirRequested);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_lastActive == this) {
        m_lastActive = nullptr;
    }

    static_cast<Application *>(qApp)->settings().save();

    QMainWindow::closeEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

    Settings &settings = static_cast<Application *>(qApp)->settings();
    settings.setLastWindowMaximized(isMaximized());

    if (!isMaximized()) {
        settings.setLastWindowWidth(width());
        settings.setLastWindowHeight(height());
    }
}

void MainWindow::initViewFrame()
{
    viewFrame_->tabBar()->setTabsClosable(true);

    connect(viewFrame_->tabBar(), &QTabBar::currentChanged, this, &MainWindow::onTabBarCurrentChanged);
    connect(viewFrame_->tabBar(), &QTabBar::tabCloseRequested, this, &MainWindow::onTabBarCloseRequested);
    connect(viewFrame_->tabBar(), &QTabBar::tabMoved, this, &MainWindow::onTabBarTabMoved);
    connect(viewFrame_->tabBar(), &QTabBar::tabBarClicked, this, &MainWindow::onTabBarClicked);
    connect(viewFrame_->stackedWidget(), &QStackedWidget::widgetRemoved, this, &MainWindow::onStackedWidgetWidgetRemoved);
}

void MainWindow::addTabWithPage(Fm::FilePath path)
{
    TabPage *page = new TabPage(this);
    page->setFileLauncher(&fileLauncher_);
    int index = viewFrame_->stackedWidget()->addWidget(page);

    // onTabPageTitleChanged
    connect(page, &TabPage::titleChanged, this, [=] (const QString title) {
        TabPage* tabPage = static_cast<TabPage*>(sender());
        int index = viewFrame_->stackedWidget()->indexOf(tabPage);
        if (index >= 0) {
            viewFrame_->tabBar()->setTabText(index, title);
        }
    });

    if (path) {
        page->chdir(path, true);
    }
    viewFrame_->tabBar()->insertTab(index, page->windowTitle());
    updateTabBar();
}

TabPage *MainWindow::currentPage()
{
    return reinterpret_cast<TabPage *>(viewFrame_->stackedWidget()->currentWidget());
}

void MainWindow::chdir(Fm::FilePath path)
{
    // wait until queued events are processed
    QTimer::singleShot(0, viewFrame_, [this, path] {
        TabPage *page = currentPage();
        if (page) {
            page->chdir(path);

            pathBar_->setPath(page->path());
        }
    });
}

void MainWindow::updateCurrentPage()
{
    TabPage *page = currentPage();
    if (page) {
        // setWindowTitle(page->windowTitle());
        pathBar_->setPath(page->path());
    }
}

void MainWindow::updateTabBar()
{
    if (viewFrame_->tabBar()->count() == 1) {
        viewFrame_->tabBar()->setVisible(false);
    } else {
        viewFrame_->tabBar()->setVisible(true);
    }
}

void MainWindow::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(index);

    static_cast<Application *>(qApp)->settings().setSplitterPos(pos);
}

void MainWindow::onPathBarChdir(const Fm::FilePath &dirPath)
{
    TabPage *page = currentPage();

    if (page && dirPath != page->path()) {
        chdir(dirPath);
    }
}

void MainWindow::onTabBarCurrentChanged(int index)
{
    TabBar *tabBar = static_cast<TabBar *>(sender());
    viewFrame_->stackedWidget()->setCurrentIndex(index);
    updateCurrentPage();
}

void MainWindow::onTabBarCloseRequested(int index)
{
    TabBar *tabBar = static_cast<TabBar *>(sender());
    QWidget *page = viewFrame_->stackedWidget()->widget(index);

    if (page) {
        // this does not delete the page widget
        viewFrame_->stackedWidget()->removeWidget(page);

        delete page;
        // NOTE: we do not remove the tab here.
        // it'll be done in onStackedWidgetWidgetRemoved()
    }
}

void MainWindow::onTabBarTabMoved(int from, int to)
{
    TabBar *tabBar = static_cast<TabBar *>(sender());
    // a tab in the tab bar is moved by the user, so we have to move the
    //  corredponding tab page in the stacked widget to the new position, too.
    QWidget *page = viewFrame_->stackedWidget()->widget(from);
    if (page) {
        // we're not going to delete the tab page, so here we block signals
        // to avoid calling the slot onStackedWidgetWidgetRemoved() before
        // removing the page. Otherwise the page widget will be destroyed.
        viewFrame_->stackedWidget()->blockSignals(true);
        viewFrame_->stackedWidget()->removeWidget(page);
        // insert the page to the new position.
        viewFrame_->stackedWidget()->insertWidget(to, page);
        // unblock signals.
        viewFrame_->stackedWidget()->blockSignals(false);
        viewFrame_->stackedWidget()->setCurrentWidget(page);
    }
}

void MainWindow::onTabBarClicked(int index)
{
    Q_UNUSED(index);

    TabBar *tabBar = static_cast<TabBar *>(sender());
    TabPage *page = currentPage();
    if (page) {
        page->folderView()->childView()->setFocus();
    }
}

void MainWindow::onStackedWidgetWidgetRemoved(int index)
{
    QStackedWidget *sw = static_cast<QStackedWidget *>(sender());
    viewFrame_->tabBar()->removeTab(index);
    updateTabBar();
}

void MainWindow::onSidePaneChdirRequested(int type, const Fm::FilePath &path)
{
    // FIXME: use enum for type value or change it to button.
    if (type == 0) { // left button (default)
        chdir(path);
    } else if(type == 1) { // middle button
        addTabWithPage(path);
    } else if(type == 2) { // new window
        (new MainWindow(path))->show();
    }
}
