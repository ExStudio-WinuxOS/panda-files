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


#ifndef FM_SIDEPANE_H
#define FM_SIDEPANE_H

#include "libfmqtglobals.h"
#include "placesview.h"
#include <QWidget>

#include "core/filepath.h"

class QComboBox;
class QVBoxLayout;
class QWidget;

namespace Fm {

class FileMenu;

class LIBFM_QT_API SidePane : public QWidget
{
    Q_OBJECT

public:
    explicit SidePane(QWidget* parent = nullptr);
    ~SidePane() override;

    QSize iconSize() const {
        return iconSize_;
    }

    void setIconSize(QSize size);

    const Fm::FilePath& currentPath() const {
        return currentPath_;
    }

    void setCurrentPath(Fm::FilePath path);

    QWidget *view() const {
        return view_;
    }

    bool showHidden() const {
        return showHidden_;
    }

    void chdir(Fm::FilePath path) {
        setCurrentPath(std::move(path));
    }

    void restoreHiddenPlaces(const QSet<QString>& items);

Q_SIGNALS:
    void chdirRequested(int type, const Fm::FilePath& path);
    void openFolderInNewWindowRequested(const Fm::FilePath& path);
    void openFolderInNewTabRequested(const Fm::FilePath& path);
    void openFolderInTerminalRequested(const Fm::FilePath& path);
    void createNewFolderRequested(const Fm::FilePath& path);

    void prepareFileMenu(Fm::FileMenu* menu); // emit before showing a Fm::FileMenu

    void hiddenPlaceSet(const QString& str, bool hide);

protected:
    bool event(QEvent* event) override;

private:
    Fm::FilePath currentPath_;
    Fm::PlacesView *view_;
    QVBoxLayout *verticalLayout;
    QSize iconSize_;
    bool showHidden_;
    QSet<QString> restorableHiddenPlaces_;
};

}

#endif // FM_SIDEPANE_H
