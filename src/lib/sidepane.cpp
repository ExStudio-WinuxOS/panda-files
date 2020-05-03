/*
 * Copyright (C) 2013 - 2015  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "sidepane.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QHeaderView>
#include "filemenu.h"

namespace Fm {

SidePane::SidePane(QWidget* parent)
  : QWidget(parent),
    view_(new Fm::PlacesView),
    iconSize_(24, 24),
    showHidden_(false)
{
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->addWidget(view_);

    // visually merge it with its surroundings
//    view_->setFrameShape(QFrame::NoFrame);
//    QPalette p = view_->palette();
//    p.setColor(QPalette::Base, QColor(Qt::transparent));
//    p.setColor(QPalette::Text, p.color(QPalette::WindowText));
//    view_->setPalette(p);
//    view_->viewport()->setAutoFillBackground(false);

    view_->restoreHiddenItems(restorableHiddenPlaces_);
    view_->setIconSize(iconSize_);
    view_->setCurrentPath(currentPath_);
    connect(view_, &PlacesView::chdirRequested, this, &SidePane::chdirRequested);
    connect(view_, &PlacesView::hiddenItemSet, this, &SidePane::hiddenPlaceSet);
}

SidePane::~SidePane()
{
    // qDebug("delete SidePane");
}

void SidePane::setIconSize(QSize size)
{
    iconSize_ = size;
    view_->setIconSize(size);
}

void SidePane::setCurrentPath(Fm::FilePath path)
{
    Q_ASSERT(path);

    currentPath_ = std::move(path);
    view_->setCurrentPath(currentPath_);
}

void SidePane::restoreHiddenPlaces(const QSet<QString>& items)
{
    static_cast<PlacesView*>(view_)->restoreHiddenItems(items);
}

} // namespace Fm
