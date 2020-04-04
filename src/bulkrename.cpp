#include "bulkrename.h"
#include "lib/utilities.h"

#include <QRegularExpression>
#include <QProgressDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>

BulkRenameDialog::BulkRenameDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    ui.setupUi(this);
    ui.lineEdit->setFocus();
    connect(ui.buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &QDialog::accept);
    connect(ui.buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, this, &QDialog::reject);
    resize(minimumSize());
    setMaximumHeight(minimumHeight()); // no vertical resizing
}

void BulkRenameDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if(ui.lineEdit->text().endsWith(QLatin1Char('#'))) { // select what's before "#"
        QTimer::singleShot(0, [this]() {
            ui.lineEdit->setSelection(0, ui.lineEdit->text().size() - 1);
        });
    }
}

BulkRenamer::BulkRenamer(const Fm::FileInfoList& files, QWidget* parent) {
    if(files.size() <= 1) { // no bulk rename with just one file
        return;
    }
    QString baseName;
    int start = 0;
    BulkRenameDialog dlg(parent);
    switch(dlg.exec()) {
    case QDialog::Accepted:
        baseName = dlg.getBaseName();
        start = dlg.getStart();
        break;
    default:
        return;
    }

    if(!baseName.contains(QLatin1Char('#'))) {
        // insert "#" before the last dot
        int end = baseName.lastIndexOf(QLatin1Char('.'));
        if(end == -1) {
            end = baseName.size();
        }
        baseName.insert(end, QLatin1Char('#'));
    }
    QProgressDialog progress(QObject::tr("Renaming files..."), QObject::tr("Abort"), 0, files.size(), parent);
    progress.setWindowModality(Qt::WindowModal);
    int i = 0, failed = 0;
    const QRegularExpression extension(QStringLiteral("\\.[^.#]+$"));
    bool noExtension(baseName.indexOf(extension) == -1);
    for(auto& file: files) {
        progress.setValue(i);
        if(progress.wasCanceled()) {
            progress.close();
            QMessageBox::warning(parent, QObject::tr("Warning"), QObject::tr("Renaming is aborted."));
            return;
        }
        // NOTE: "Edit name" seems to be the best way of handling non-UTF8 filename encoding.
        auto fileName = QString::fromUtf8(g_file_info_get_edit_name(file->gFileInfo().get()));
        if(fileName.isEmpty()) {
            fileName = QString::fromStdString(file->name());
        }
        QString newName = baseName;

        // keep the extension if the new name doesn't have one
        if(noExtension) {
            QRegularExpressionMatch match;
            if(fileName.indexOf(extension, 0, &match) > -1) {
                newName += match.captured();
            }
        }

        newName.replace(QLatin1Char('#'), QString::number(start + i));
        if (newName == fileName || !Fm::changeFileName(file->path(), newName, nullptr, false)) {
            ++failed;
        }
        ++i;
    }
    progress.setValue(i);
    if(failed == i) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("No file could be renamed."));
    }
    else if(failed > 0) {
        QMessageBox::critical(parent, QObject::tr("Error"), QObject::tr("Some files could not be renamed."));
    }
}

BulkRenamer::~BulkRenamer()
{

}
