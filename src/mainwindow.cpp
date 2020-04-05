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

// -------------------------------

ViewFrame::ViewFrame(QWidget* parent)
    : QFrame(parent),
      topBar_(nullptr)
{
    QVBoxLayout* vBox = new QVBoxLayout;
    vBox->setContentsMargins(0, 0, 0, 0);

    tabBar_ = new TabBar;
    tabBar_->setFocusPolicy(Qt::NoFocus);
    stackedWidget_ = new QStackedWidget;
    vBox->addWidget(tabBar_);
    vBox->addWidget(stackedWidget_, 1);
    setLayout(vBox);

    // tabbed browsing interface
    tabBar_->setDocumentMode(true);
    tabBar_->setElideMode(Qt::ElideRight);
    tabBar_->setExpanding(false);
    tabBar_->setMovable(true); // reorder the tabs by dragging
    // switch to the tab under the cursor during dnd.
    tabBar_->setChangeCurrentOnDrag(true);
    tabBar_->setAcceptDrops(true);
    tabBar_->setContextMenuPolicy(Qt::CustomContextMenu);
}

void ViewFrame::createTopBar(bool usePathButtons)
{
    if (QVBoxLayout* vBox = qobject_cast<QVBoxLayout*>(layout())) {
        if (usePathButtons) {
            if (qobject_cast<Fm::PathEdit*>(topBar_)) {
                delete topBar_;
                topBar_ = nullptr;
            }
            if (topBar_ == nullptr) {
                topBar_ = new Fm::PathBar();
                vBox->insertWidget(0, topBar_);
            }
        }
        else {
            if (qobject_cast<Fm::PathBar*>(topBar_)) {
                delete topBar_;
                topBar_ = nullptr;
            }
            if (topBar_ == nullptr) {
                topBar_ = new Fm::PathEdit();
                vBox->insertWidget(0, topBar_);
            }
        }
    }
}

void ViewFrame::removeTopBar()
{
    if (topBar_ != nullptr) {
        if (QVBoxLayout* vBox = qobject_cast<QVBoxLayout*>(layout())) {
            vBox->removeWidget(topBar_);
            delete topBar_;
            topBar_ = nullptr;
        }
    }
}

// -------------------------------

MainWindow *MainWindow::m_lastActive = nullptr;

MainWindow::MainWindow(Fm::FilePath path)
    : QMainWindow(),
      m_pathEntry(nullptr),
      m_pathBar(nullptr),
      m_bookmarks(Fm::Bookmarks::globalInstance()),
      m_activeViewFrame(nullptr),
      m_addressBar(new QToolBar)
{
    QVBoxLayout *layout = new QVBoxLayout;
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_addressBar);

    Settings &settings = static_cast<Application *>(qApp)->settings();

    setAttribute(Qt::WA_DeleteOnClose);

    ViewFrame* viewFrame = new ViewFrame();
    layout->addWidget(viewFrame);
}

MainWindow::~MainWindow()
{
}

void MainWindow::chdir(Fm::FilePath path, ViewFrame* viewFrame)
{
}

int MainWindow::addTab(Fm::FilePath path, ViewFrame* viewFrame)
{
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

    QWidget::closeEvent(e);
}
