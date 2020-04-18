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
      pathBar_(nullptr),
      sidePane_(new Fm::SidePane),
      splitter_(new QSplitter),
      viewSplitter_(new QSplitter(splitter_)),
      m_bookmarks(Fm::Bookmarks::globalInstance()),
      activeViewFrame_(nullptr),
      m_addressBar(new QToolBar)
{
    QVBoxLayout *layout = new QVBoxLayout;
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    QWidget *pathBarWidget = new QWidget;
    pathBarLayout_->setMargin(0);
    pathBarLayout_->setSpacing(0);
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

    splitter_->setOrientation(Qt::Horizontal);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(sidePane_->sizePolicy().hasHeightForWidth());
    sidePane_->setSizePolicy(sizePolicy);
    splitter_->addWidget(sidePane_);
    splitter_->setChildrenCollapsible(false);

    viewSplitter_->setOrientation(Qt::Horizontal);
    splitter_->addWidget(viewSplitter_);

    // init sidepane
    sidePane_->setIconSize(QSize(settings.sidePaneIconSize(), settings.sidePaneIconSize()));
    sidePane_->setMode(settings.sidePaneMode());
    sidePane_->restoreHiddenPlaces(settings.getHiddenPlaces());

    // setup the splitter
    splitter_->setStretchFactor(1, 1); // only the right pane can be stretched
    QList<int> sizes;
    sizes.append(settings.splitterPos());
    sizes.append(300);
    splitter_->setSizes(sizes);

    // size from settings
    resize(settings.windowWidth(), settings.windowHeight());
    if (settings.rememberWindowSize() && settings.windowMaximized()) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }

    addViewFrame(path);

    if (splitView_) {
        // put the menu button on the right (there's no path bar/entry on the toolbar)
        QWidget* w = new QWidget(this);
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//        menuSpacer_ = ui.toolBar->insertWidget(ui.actionMenu, w);

//        ui.actionSplitView->setChecked(true);
        addViewFrame(path);
        qApp->removeEventFilter(this); // precaution
        qApp->installEventFilter(this);
    } else {
//        ui.actionSplitView->setChecked(false);
        setAcceptDrops(true); // we want tab dnd in the simple mode
    }
    createPathBar(settings.pathBarButtons());

    // detect change of splitter position
    connect(splitter_, &QSplitter::splitterMoved, this, &MainWindow::onSplitterMoved);

    connect(sidePane_, &Fm::SidePane::chdirRequested, this, &MainWindow::onSidePaneChdirRequested);
    connect(sidePane_, &Fm::SidePane::openFolderInNewWindowRequested, this, &MainWindow::onSidePaneOpenFolderInNewWindowRequested);
    connect(sidePane_, &Fm::SidePane::openFolderInNewTabRequested, this, &MainWindow::onSidePaneOpenFolderInNewTabRequested);
    connect(sidePane_, &Fm::SidePane::openFolderInTerminalRequested, this, &MainWindow::onSidePaneOpenFolderInTerminalRequested);
//    connect(sidePane_, &Fm::SidePane::createNewFolderRequested, this, &MainWindow::onSidePaneCreateNewFolderRequested);
    connect(sidePane_, &Fm::SidePane::modeChanged, this, &MainWindow::onSidePaneModeChanged);
    connect(sidePane_, &Fm::SidePane::hiddenPlaceSet, this, &MainWindow::onSettingHiddenPlace);
}

MainWindow::~MainWindow()
{
}

void MainWindow::chdir(Fm::FilePath path, ViewFrame* viewFrame)
{
    // wait until queued events are processed
    QTimer::singleShot(0, viewFrame, [this, path, viewFrame] {
        if(TabPage* page = currentPage(viewFrame)) {
            page->chdir(path, true);
            if(viewFrame == activeViewFrame_) {
                updateUIForCurrentPage();
            }
            else {
                if(Fm::PathBar* pathBar = qobject_cast<Fm::PathBar*>(viewFrame->getTopBar())) {
                    pathBar->setPath(page->path());
                }
                else if(Fm::PathEdit* pathEntry = qobject_cast<Fm::PathEdit*>(viewFrame->getTopBar())) {
                    pathEntry->setText(page->pathName());
                }
            }
        }
    });
}

int MainWindow::addTab(Fm::FilePath path, ViewFrame* viewFrame)
{
    TabPage* newPage = new TabPage(this);
    return addTabWithPage(newPage, viewFrame, path);
}

bool MainWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowActivate:
        m_lastActive = this;
    default:
        break;
    }

    return QMainWindow::event(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_lastActive == this) {
        m_lastActive = nullptr;
    }

    static_cast<Application*>(qApp)->settings().save();

    QWidget::closeEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);

    Settings &settings = static_cast<Application*>(qApp)->settings();
    if (settings.rememberWindowSize()) {
        settings.setLastWindowMaximized(isMaximized());

        if (!isMaximized()) {
            settings.setLastWindowWidth(width());
            settings.setLastWindowHeight(height());
        }
    }
}

void MainWindow::updateUIForCurrentPage(bool setFocus)
{
    TabPage* tabPage = currentPage();

    if(tabPage) {
        setWindowTitle(tabPage->windowTitle());
        if(splitView_) {
            if(Fm::PathBar* pathBar = qobject_cast<Fm::PathBar*>(activeViewFrame_->getTopBar())) {
                pathBar->setPath(tabPage->path());
            }
            else if(Fm::PathEdit* pathEntry = qobject_cast<Fm::PathEdit*>(activeViewFrame_->getTopBar())) {
                pathEntry->setText(tabPage->pathName());
            }
        }
        else {
            if(pathEntry_ != nullptr) {
                pathEntry_->setText(tabPage->pathName());
            }
            else if(pathBar_ != nullptr) {
                pathBar_->setPath(tabPage->path());
            }
        }
//        ui.statusbar->showMessage(tabPage->statusText());
//        fsInfoLabel_->setText(tabPage->statusText(TabPage::StatusTextFSInfo));
        if(setFocus) {
            tabPage->folderView()->childView()->setFocus();
        }

        // update side pane
//        ui.sidePane->setCurrentPath(tabPage->path());
//        ui.sidePane->setShowHidden(tabPage->showHidden());

        // update back/forward/up toolbar buttons
//        ui.actionGoUp->setEnabled(tabPage->canUp());
//        ui.actionGoBack->setEnabled(tabPage->canBackward());
//        ui.actionGoForward->setEnabled(tabPage->canForward());

        updateViewMenuForCurrentPage();
        // updateStatusBarForCurrentPage();
    }

    // also update the enabled state of Edit actions
    updateEditSelectedActions();
    bool isWritable(false);
    if(tabPage && tabPage->folder()) {
        if(auto info = tabPage->folder()->info()) {
            isWritable = info->isWritable();
        }
    }
//    ui.actionPaste->setEnabled(isWritable);
//    ui.menuCreateNew->setEnabled(isWritable);
//    // disable creation shortcuts too
//    ui.actionNewFolder->setEnabled(isWritable);
//    ui.actionNewBlankFile->setEnabled(isWritable);
}

void MainWindow::updateViewMenuForCurrentPage()
{

}

void MainWindow::createPathBar(bool usePathButtons)
{
    // NOTE: Path bars/entries may be created after tab pages; so, their paths/texts should be set.
    if (splitView_) {
        for (int i = 0; i < viewSplitter_->count(); ++i) {
            if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(viewSplitter_->widget(i))) {
                viewFrame->createTopBar(usePathButtons);
                TabPage* curPage = currentPage(viewFrame);
                if (Fm::PathBar* pathBar = qobject_cast<Fm::PathBar*>(viewFrame->getTopBar())) {
                    connect(pathBar, &Fm::PathBar::chdir, this, &MainWindow::onPathBarChdir);
                    connect(pathBar, &Fm::PathBar::middleClickChdir, this, &MainWindow::onPathBarMiddleClickChdir);
                    connect(pathBar, &Fm::PathBar::editingFinished, this, &MainWindow::onResetFocus);
                    if (curPage) {
                        pathBar->setPath(curPage->path());
                    }
                }
                else if(Fm::PathEdit* pathEntry = qobject_cast<Fm::PathEdit*>(viewFrame->getTopBar())) {
                    connect(pathEntry, &Fm::PathEdit::returnPressed, this, &MainWindow::onPathEntryReturnPressed);
                    if (curPage) {
                        pathEntry->setText(curPage->pathName());
                    }
                }
            }
        }
    }
    else {
        QWidget* bar = nullptr;
        TabPage* curPage = currentPage();
        if (usePathButtons) {
            if (pathEntry_ != nullptr) {
                delete pathEntry_;
                pathEntry_ = nullptr;
            }
            if (pathBar_ == nullptr) {
                bar = pathBar_ = new Fm::PathBar(this);
                connect(pathBar_, &Fm::PathBar::chdir, this, &MainWindow::onPathBarChdir);
                connect(pathBar_, &Fm::PathBar::middleClickChdir, this, &MainWindow::onPathBarMiddleClickChdir);
                connect(pathBar_, &Fm::PathBar::editingFinished, this, &MainWindow::onResetFocus);
                if (curPage) {
                    pathBar_->setPath(currentPage()->path());
                }
            }
        }
        else {
            if (pathBar_ != nullptr) {
                delete pathBar_;
                pathBar_ = nullptr;
            }
            if (pathEntry_ == nullptr) {
                bar = pathEntry_ = new Fm::PathEdit(this);
                connect(pathEntry_, &Fm::PathEdit::returnPressed, this, &MainWindow::onPathEntryReturnPressed);
                if(curPage) {
                    pathEntry_->setText(curPage->pathName());
                }
            }
        }
        if (bar != nullptr) {
            pathBarLayout_->addWidget(bar);
//            ui.actionGo->setVisible(!usePathButtons);
        }
    }
}

void MainWindow::addViewFrame(const Fm::FilePath &path)
{
    Settings &settings = static_cast<Application*>(qApp)->settings();
    ViewFrame* viewFrame = new ViewFrame();
    viewFrame->getTabBar()->setDetachable(!splitView_); // no tab DND with the split view
    viewFrame->getTabBar()->setTabsClosable(settings.showTabClose());
    splitter_->addWidget(viewFrame);  // the splitter takes ownership of viewFrame

    qDebug() << splitter_->count();

    if (splitter_->count() == 3) {
        activeViewFrame_ = viewFrame;
    } else { // give equal widths to all view frames
        QTimer::singleShot(0, this, [this] {
            QList<int> sizes;
            for(int i = 0; i < splitter_->count(); ++i) {
                sizes << splitter_->width() / splitter_->count();
            }
            splitter_->setSizes(sizes);
        });
    }

    connect(viewFrame->getTabBar(), &QTabBar::currentChanged, this, &MainWindow::onTabBarCurrentChanged);
    connect(viewFrame->getTabBar(), &QTabBar::tabCloseRequested, this, &MainWindow::onTabBarCloseRequested);
    connect(viewFrame->getTabBar(), &QTabBar::tabMoved, this, &MainWindow::onTabBarTabMoved);
    connect(viewFrame->getTabBar(), &QTabBar::tabBarClicked, this, &MainWindow::onTabBarClicked);
    connect(viewFrame->getTabBar(), &QTabBar::customContextMenuRequested, this, &MainWindow::tabContextMenu);
    connect(viewFrame->getTabBar(), &TabBar::tabDetached, this, &MainWindow::detachTab);
    connect(viewFrame->getStackedWidget(), &QStackedWidget::widgetRemoved, this, &MainWindow::onStackedWidgetWidgetRemoved);

    if (path) {
        addTab(path, viewFrame);
    }
}

int MainWindow::addTabWithPage(TabPage *page, ViewFrame *viewFrame, Fm::FilePath path)
{
    if (page == nullptr || viewFrame == nullptr) {
            return -1;
    }

    page->setFileLauncher(&fileLauncher_);
    int index = viewFrame->getStackedWidget()->addWidget(page);
    connect(page, &TabPage::titleChanged, this, &MainWindow::onTabPageTitleChanged);
    connect(page, &TabPage::statusChanged, this, &MainWindow::onTabPageStatusChanged);
    connect(page, &TabPage::sortFilterChanged, this, &MainWindow::onTabPageSortFilterChanged);
//    connect(page, &TabPage::backwardRequested, this, &MainWindow::on_actionGoBack_triggered);
//    connect(page, &TabPage::forwardRequested, this, &MainWindow::on_actionGoForward_triggered);
//    connect(page, &TabPage::folderUnmounted, this, &MainWindow::onFolderUnmounted);

    if (path) {
        page->chdir(path, true);
    }
    viewFrame->getTabBar()->insertTab(index, page->windowTitle());

    Settings& settings = static_cast<Application*>(qApp)->settings();
    if(!settings.alwaysShowTabs()) {
        viewFrame->getTabBar()->setVisible(viewFrame->getTabBar()->count() > 1);
    }

    return index;
}

void MainWindow::closeTab(int index, ViewFrame *viewFrame)
{
    QWidget* page = viewFrame->getStackedWidget()->widget(index);

    if (page) {
        viewFrame->getStackedWidget()->removeWidget(page); // this does not delete the page widget
        delete page;
        // NOTE: we do not remove the tab here.
        // it'll be done in onStackedWidgetWidgetRemoved()
    }
}

ViewFrame *MainWindow::viewFrameForTabPage(TabPage *page)
{
    if (page) {
        if (QStackedWidget* sw = qobject_cast<QStackedWidget*>(page->parentWidget())) {
            if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(sw->parentWidget())) {
                return viewFrame;
            }
        }
    }

    return nullptr;
}

void MainWindow::updateEditSelectedActions()
{
    bool hasAccessible(false);
    bool hasDeletable(false);
    int renamable(0);
    if (TabPage* page = currentPage()) {
        auto files = page->selectedFiles();
        for(auto& file: files) {
            if(file->isAccessible()) {
                hasAccessible = true;
            }
            if(file->isDeletable()) {
                hasDeletable = true;
            }
            if(file->canSetName()) {
                ++renamable;
            }
            if (hasAccessible && hasDeletable && renamable > 1) {
                break;
            }
        }
//        ui.actionCopyFullPath->setEnabled(files.size() == 1);
    }
//    ui.actionCopy->setEnabled(hasAccessible);
//    ui.actionCut->setEnabled(hasDeletable);
//    ui.actionDelete->setEnabled(hasDeletable);
//    ui.actionRename->setEnabled(renamable > 0);
    //    ui.actionBulkRename->setEnabled(renamable > 1);
}

void MainWindow::onPathEntryReturnPressed()
{
    Fm::PathEdit* pathEntry = pathEntry_;
    if (pathEntry == nullptr) {
        pathEntry = static_cast<Fm::PathEdit*>(sender());
    }
    if (pathEntry != nullptr) {
        QString text = pathEntry->text();
        QByteArray utext = text.toLocal8Bit();
        chdir(Fm::FilePath::fromPathStr(utext.constData()));
    }
}

void MainWindow::onPathBarChdir(const Fm::FilePath &dirPath)
{
    TabPage* page = nullptr;
    ViewFrame* viewFrame = nullptr;
    if (pathBar_ != nullptr) {
        page = currentPage();
        viewFrame = activeViewFrame_;
    }
    else {
        Fm::PathBar* pathBar = static_cast<Fm::PathBar*>(sender());
        viewFrame = qobject_cast<ViewFrame*>(pathBar->parentWidget());
        if(viewFrame != nullptr) {
            page = currentPage(viewFrame);
        }
    }
    if(page && dirPath != page->path()) {
        chdir(dirPath, viewFrame);
    }
}

void MainWindow::onPathBarMiddleClickChdir(const Fm::FilePath &dirPath)
{
    ViewFrame* viewFrame = nullptr;

    if (pathBar_ != nullptr) {
        viewFrame = activeViewFrame_;
    }
    else {
        Fm::PathBar* pathBar = static_cast<Fm::PathBar*>(sender());
        viewFrame = qobject_cast<ViewFrame*>(pathBar->parentWidget());
    }
    if (viewFrame) {
        addTab(dirPath, viewFrame);
    }
}

void MainWindow::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(index);

    static_cast<Application *>(qApp)->settings().setSplitterPos(pos);
}

void MainWindow::onResetFocus()
{
    if (TabPage* page = currentPage()) {
        page->folderView()->childView()->setFocus();
    }
}

void MainWindow::onTabBarCloseRequested(int index)
{
    TabBar* tabBar = static_cast<TabBar*>(sender());
    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(tabBar->parentWidget())) {
        // closeTab(index, viewFrame);
    }
}

void MainWindow::onTabBarCurrentChanged(int index)
{
    TabBar* tabBar = static_cast<TabBar*>(sender());
    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(tabBar->parentWidget())) {
        viewFrame->getStackedWidget()->setCurrentIndex(index);
        if (viewFrame == activeViewFrame_) {
            updateUIForCurrentPage();
        }
        else {
            if (TabPage* page = currentPage(viewFrame)) {
                if (Fm::PathBar* pathBar = qobject_cast<Fm::PathBar*>(viewFrame->getTopBar())) {
                    pathBar->setPath(page->path());
                }
                else if (Fm::PathEdit* pathEntry = qobject_cast<Fm::PathEdit*>(viewFrame->getTopBar())) {
                    pathEntry->setText(page->pathName());
                }
            }
        }
    }
}

void MainWindow::onTabBarTabMoved(int from, int to)
{
    TabBar* tabBar = static_cast<TabBar*>(sender());

    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(tabBar->parentWidget())) {
        // a tab in the tab bar is moved by the user, so we have to move the
        //  corredponding tab page in the stacked widget to the new position, too.
        QWidget* page = viewFrame->getStackedWidget()->widget(from);
        if (page) {
            // we're not going to delete the tab page, so here we block signals
            // to avoid calling the slot onStackedWidgetWidgetRemoved() before
            // removing the page. Otherwise the page widget will be destroyed.
            viewFrame->getStackedWidget()->blockSignals(true);
            viewFrame->getStackedWidget()->removeWidget(page);
            viewFrame->getStackedWidget()->insertWidget(to, page); // insert the page to the new position
            viewFrame->getStackedWidget()->blockSignals(false); // unblock signals
            viewFrame->getStackedWidget()->setCurrentWidget(page);
        }
    }
}

void MainWindow::onTabBarClicked(int index)
{
    Q_UNUSED(index);

    TabBar* tabBar = static_cast<TabBar*>(sender());

    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(tabBar->parentWidget())) {
        // focus the view on clicking the tab bar
        if (TabPage* page = currentPage(viewFrame)) {
            page->folderView()->childView()->setFocus();
        }
    }
}

void MainWindow::tabContextMenu(const QPoint &pos)
{
    TabBar* tabBar = static_cast<TabBar*>(sender());
//    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(tabBar->parentWidget())) {
//        int tabNum = viewFrame->getTabBar()->count();
//        if (tabNum <= 1) {
//            return;
//        }

//        rightClickIndex_ = viewFrame->getTabBar()->tabAt(pos);
//        if (rightClickIndex_ < 0) {
//            return;
//        }

//        QMenu menu;
//        if (rightClickIndex_ > 0) {
//            menu.addAction(ui.actionCloseLeft);
//        }
//        if (rightClickIndex_ < tabNum - 1) {
//            menu.addAction(ui.actionCloseRight);
//            if(rightClickIndex_ > 0) {
//                menu.addSeparator();
//                menu.addAction(ui.actionCloseOther);
//            }
//        }
//        menu.exec(viewFrame->getTabBar()->mapToGlobal(pos));
    //    }
}

void MainWindow::detachTab()
{
    if (activeViewFrame_->getStackedWidget()->count() == 1) { // don't detach a single tab
        activeViewFrame_->getTabBar()->finishMouseMoveEvent();
        return;
    }

    // close the tab and move its page to a new window
    TabPage* dropPage = currentPage();
    if (dropPage) {
        disconnect(dropPage, nullptr, this, nullptr);

        activeViewFrame_->getTabBar()->releaseMouse(); // as in dropTab()

        QWidget* page = activeViewFrame_->getStackedWidget()->currentWidget();
        activeViewFrame_->getStackedWidget()->removeWidget(page);
        MainWindow* newWin = new MainWindow();
        newWin->addTabWithPage(dropPage, newWin->activeViewFrame_);
        newWin->show();
    } else {
        activeViewFrame_->getTabBar()->finishMouseMoveEvent(); // impossible
    }
}

void MainWindow::onStackedWidgetWidgetRemoved(int index)
{
    QStackedWidget* sw = static_cast<QStackedWidget*>(sender());
    if (ViewFrame* viewFrame = qobject_cast<ViewFrame*>(sw->parentWidget())) {
        // qDebug("onStackedWidgetWidgetRemoved: %d", index);
        // need to remove associated tab from tabBar
        viewFrame->getTabBar()->removeTab(index);
        if (viewFrame->getTabBar()->count() == 0) { // this is the last one
            if (!splitView_) {
                deleteLater(); // destroy the whole window
                // qDebug("delete window");
            }
            else {
                // if we are in the split mode and the last tab of a view frame is closed,
                // remove that view frame and go to the simple mode
                for (int i = 0; i < viewSplitter_->count(); ++i) {
                    // first find and activate the next view frame
                    if (ViewFrame* thisViewFrame = qobject_cast<ViewFrame*>(viewSplitter_->widget(i))) {
                        if (thisViewFrame == viewFrame) {
                            int n = i < viewSplitter_->count() - 1 ? i + 1 : 0;
                            if (ViewFrame* nextViewFrame = qobject_cast<ViewFrame*>(viewSplitter_->widget(n))) {
                                if(activeViewFrame_ != nextViewFrame) {
                                    activeViewFrame_ = nextViewFrame;
                                    updateUIForCurrentPage();
                                    // if the window isn't active, eventFilter() won't be called,
                                    // so we should revert to the main palette here
                                    if (activeViewFrame_->palette().color(QPalette::Base)
                                       != qApp->palette().color(QPalette::Base)) {
                                        activeViewFrame_->setPalette(qApp->palette());
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
//                ui.actionSplitView->setChecked(false);
//                on_actionSplitView_triggered(false);
            }
        }
        else {
            Settings& settings = static_cast<Application*>(qApp)->settings();
            if (!settings.alwaysShowTabs() && viewFrame->getTabBar()->count() == 1) {
                viewFrame->getTabBar()->setVisible(false);
            }
        }
    }
}

void MainWindow::onTabPageTitleChanged(QString title)
{
    TabPage* tabPage = static_cast<TabPage*>(sender());
    if(ViewFrame* viewFrame = viewFrameForTabPage(tabPage)) {
        int index = viewFrame->getStackedWidget()->indexOf(tabPage);
        if(index >= 0) {
            viewFrame->getTabBar()->setTabText(index, title);
        }

        if(viewFrame == activeViewFrame_) {
            if(tabPage == currentPage()) {
                setWindowTitle(title);

                // Since TabPage::titleChanged is emitted on changing directory,
                // the enabled state of Paste action should be updated here
                bool isWritable(false);
                if(tabPage && tabPage->folder()) {
                    if(auto info = tabPage->folder()->info()) {
                        isWritable = info->isWritable();
                    }
                }
//                ui.actionPaste->setEnabled(isWritable);
//                ui.menuCreateNew->setEnabled(isWritable);
//                ui.actionNewFolder->setEnabled(isWritable);
//                ui.actionNewBlankFile->setEnabled(isWritable);
            }
        }
    }
}

void MainWindow::onTabPageStatusChanged(int type, QString statusText)
{
    TabPage* tabPage = static_cast<TabPage*>(sender());
    if(tabPage == currentPage()) {
        switch(type) {
        case TabPage::StatusTextNormal:
        case TabPage::StatusTextSelectedFiles: {
            // although the status text may change very frequently,
            // the text of PCManFM::StatusBar is updated with a delay
            QString text = tabPage->statusText(TabPage::StatusTextSelectedFiles);
            if(text.isEmpty()) {
//                ui.statusbar->showMessage(tabPage->statusText(TabPage::StatusTextNormal));
            }
            else {
//                ui.statusbar->showMessage(text);
            }
            break;
        }
        case TabPage::StatusTextFSInfo:
//            fsInfoLabel_->setText(tabPage->statusText(TabPage::StatusTextFSInfo));
//            fsInfoLabel_->setVisible(!statusText.isEmpty());
            break;
        }
    }

    // Since TabPage::statusChanged is always emitted after View::selChanged,
    // there is no need to connect a separate slot to the latter signal
    updateEditSelectedActions();
}

void MainWindow::onTabPageSortFilterChanged()
{
    // NOTE: This may be called from context menu too.
    TabPage* tabPage = static_cast<TabPage*>(sender());
    if(tabPage == currentPage()) {
        updateViewMenuForCurrentPage();
        if(!tabPage->hasCustomizedView()) { // remember sort settings globally
            Settings& settings = static_cast<Application*>(qApp)->settings();
            settings.setSortColumn(static_cast<Fm::FolderModel::ColumnId>(tabPage->sortColumn()));
            settings.setSortOrder(tabPage->sortOrder());
            settings.setSortFolderFirst(tabPage->sortFolderFirst());
            settings.setSortHiddenLast(tabPage->sortHiddenLast());
            settings.setSortCaseSensitive(tabPage->sortCaseSensitive());
            settings.setShowHidden(tabPage->showHidden());
        }
    }
}

void MainWindow::onSidePaneChdirRequested(int type, const Fm::FilePath &path)
{
    // FIXME: use enum for type value or change it to button.
    if (type == 0) { // left button (default)
        chdir(path);
    } else if(type == 1) { // middle button
        addTab(path);
    } else if(type == 2) { // new window
        (new MainWindow(path))->show();
    }
}

void MainWindow::onSidePaneOpenFolderInNewWindowRequested(const Fm::FilePath &path)
{
    (new MainWindow(path))->show();
}

void MainWindow::onSidePaneOpenFolderInNewTabRequested(const Fm::FilePath &path)
{
    addTab(path);
}

void MainWindow::onSidePaneOpenFolderInTerminalRequested(const Fm::FilePath &path)
{
    Application* app = static_cast<Application*>(qApp);
    app->openFolderInTerminal(path);
}

void MainWindow::onSidePaneModeChanged(Fm::SidePane::Mode mode)
{
    static_cast<Application*>(qApp)->settings().setSidePaneMode(mode);
}

void MainWindow::onSettingHiddenPlace(const QString &str, bool hide)
{
    static_cast<Application*>(qApp)->settings().setHiddenPlace(str, hide);
}
