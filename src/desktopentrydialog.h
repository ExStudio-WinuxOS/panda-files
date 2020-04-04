#ifndef DESKTOPENTRYDIALOG_H
#define DESKTOPENTRYDIALOG_H

#include <QDialog>

class DesktopEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DesktopEntryDialog(QWidget *parent = nullptr);

    virtual ~DesktopEntryDialog();

    virtual void accept() override;

Q_SIGNALS:
    void desktopEntryCreated(const QString& name);

private Q_SLOTS:
    void onChangingType(int type);
    void onClickingIconButton();
    void onClickingCommandButton();
};

#endif // DESKTOPENTRYDIALOG_H
