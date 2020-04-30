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
      rightView_(new QFrame),
      rightLayout_(new QVBoxLayout),
      m_bookmarks(Fm::Bookmarks::globalInstance()),
      activeViewFrame_(nullptr)
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

    rightView_->setLayout(rightLayout_);

    splitter_->setOrientation(Qt::Horizontal);
    splitter_->addWidget(sidePane_);
    splitter_->addWidget(rightView_);
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
