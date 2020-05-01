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

#ifndef FM_EXECFILEDIALOG_H
#define FM_EXECFILEDIALOG_H

#include "core/basicfilelauncher.h"
#include "core/fileinfo.h"

#include <QDialog>
#include <QPushButton>

#include <memory>

namespace Ui {
  class ExecFileDialog;
}

namespace Fm {

class ExecFileDialog : public QDialog
{
    Q_OBJECT

public:
    ExecFileDialog(const FileInfo& fileInfo, QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    ~ExecFileDialog() override;

    BasicFileLauncher::ExecAction result() {
        return result_;
    }

protected:
    void accept() override;
    void reject() override;

private:
    BasicFileLauncher::ExecAction result_;

    QPushButton *openButton_;
    QPushButton *execButton_;
    QPushButton *execTermButton_;
    QPushButton *canelButton_;
};

}

#endif // FM_EXECFILEDIALOG_H
