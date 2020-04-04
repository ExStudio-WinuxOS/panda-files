
#ifndef PCMANFM_FOLDERVIEW_H
#define PCMANFM_FOLDERVIEW_H

#include "lib/folderview.h"
#include "lib/core/filepath.h"

namespace Fm {
class FileMenu;
class FolderMenu;
}

class Settings;

class View : public Fm::FolderView
{
    Q_OBJECT

public:
    explicit View(Fm::FolderView::ViewMode _mode = IconMode, QWidget* parent = 0);
    virtual ~View();

    void updateFromSettings(Settings& settings);

    QSize  getMargins() const {
        return Fm::FolderView::getMargins();
    }
    void setMargins(QSize size) {
        Fm::FolderView::setMargins(size);
    }

protected Q_SLOTS:
    void onNewWindow();
    void onNewTab();
    void onOpenInTerminal();
    void onSearch();

protected:
    virtual void onFileClicked(int type, const std::shared_ptr<const Fm::FileInfo>& fileInfo);
    virtual void prepareFileMenu(Fm::FileMenu* menu);
    virtual void prepareFolderMenu(Fm::FolderMenu* menu);

private:

};

#endif // VIEW_H
