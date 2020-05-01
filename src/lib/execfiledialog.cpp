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

#include "execfiledialog_p.h"
#include "core/iconinfo.h"
#include <QVBoxLayout>
#include <QLabel>

namespace Fm {

ExecFileDialog::ExecFileDialog(const FileInfo &fileInfo, QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f),
    result_(BasicFileLauncher::ExecAction::DIRECT_EXEC)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(10, 5, 10, 10);
    layout->setSpacing(10);
    setLayout(layout);

    QHBoxLayout *tipsLayout = new QHBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    layout->addLayout(tipsLayout);
    layout->addLayout(buttonLayout);

    // show file icon
    auto gicon = fileInfo.icon();
    if (gicon) {
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(gicon->qicon().pixmap(QSize(48, 48)));
        iconLabel->setFixedSize(QSize(48, 48));
        tipsLayout->addWidget(iconLabel);
        tipsLayout->addSpacing(10);
    }

    // 提示文本
    QLabel *msgLabel = new QLabel;
    tipsLayout->addWidget(msgLabel);
    tipsLayout->setSpacing(0);
    tipsLayout->addStretch();

    // init buttons.
    openButton_ = new QPushButton(tr("Open"));
    execButton_ = new QPushButton(tr("Execute"));
    execTermButton_ = new QPushButton(tr("Execute in Terminal"));
    canelButton_ = new QPushButton(tr("Canel"));
    buttonLayout->addWidget(openButton_);
    buttonLayout->addWidget(execButton_);
    buttonLayout->addWidget(execTermButton_);
    buttonLayout->addWidget(canelButton_);

    QString msg;
    if (fileInfo.isDesktopEntry()) {
        msg = tr("This file '%1' seems to be a desktop entry.\nWhat do you want to do with it?")
              .arg(fileInfo.displayName());
        execButton_->setDefault(true);
        execTermButton_->hide();
    } else if (fileInfo.isText()) {
        msg = tr("This text file '%1' seems to be an executable script.\nWhat do you want to do with it?")
              .arg(fileInfo.displayName());
        execTermButton_->setDefault(true);
    } else {
        msg = tr("This file '%1' is executable. Do you want to execute it?")
              .arg(fileInfo.displayName());
        execButton_->setDefault(true);
        openButton_->hide();
    }

    msgLabel->setText(msg);

    connect(canelButton_, &QPushButton::clicked, this, &ExecFileDialog::reject);
    connect(execButton_, &QPushButton::clicked, this, &ExecFileDialog::accept);
    connect(execTermButton_, &QPushButton::clicked, this, &ExecFileDialog::accept);
    connect(openButton_, &QPushButton::clicked, this, &ExecFileDialog::accept);
}

ExecFileDialog::~ExecFileDialog()
{
}

void ExecFileDialog::accept()
{
    QObject *_sender = sender();

    if (_sender == execButton_) {
        result_ = BasicFileLauncher::ExecAction::DIRECT_EXEC;
    } else if (_sender == execTermButton_) {
        result_ = BasicFileLauncher::ExecAction::EXEC_IN_TERMINAL;
    } else if (_sender == openButton_) {
        result_ = BasicFileLauncher::ExecAction::OPEN_WITH_DEFAULT_APP;
    } else {
        result_ = BasicFileLauncher::ExecAction::CANCEL;
    }

    QDialog::accept();
}

void ExecFileDialog::reject()
{
    result_ = BasicFileLauncher::ExecAction::CANCEL;
    QDialog::reject();
}

} // namespace Fm
