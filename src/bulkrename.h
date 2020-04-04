#ifndef BULKRENAME_H
#define BULKRENAME_H

#include <QDialog>
#include "lib/core/fileinfo.h"

class BulkRenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BulkRenameDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);

    QString getBaseName() const {
        return ui.lineEdit->text();
    }
    int getStart() const {
        return ui.spinBox->value();
    }

protected:
    virtual void showEvent(QShowEvent* event) override;

private:
    Ui::BulkRenameDialog ui;
};

class BulkRenamer
{
public:
    BulkRenamer(const Fm::FileInfoList& files, QWidget* parent = nullptr);
    ~BulkRenamer();
};

#endif // BULKRENAME_H
