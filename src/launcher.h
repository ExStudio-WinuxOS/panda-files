#ifndef PCMANFM_LAUNCHER_H
#define PCMANFM_LAUNCHER_H

#include "lib/filelauncher.h"

class MainWindow;
class Launcher : public Fm::FileLauncher
{
public:
    Launcher(MainWindow* mainWindow = nullptr);
    ~Launcher();

    void openInNewTab() {
        openInNewTab_ = true;
    }

protected:
    bool openFolder(GAppLaunchContext* ctx, const Fm::FileInfoList& folderInfos, Fm::GErrorPtr& err) override;

private:
    MainWindow* mainWindow_;
    bool openInNewTab_;
};

#endif // LAUNCHER_H
