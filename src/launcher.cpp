#include "launcher.h"
#include "mainwindow.h"
#include "application.h"
#include "lib/core/filepath.h"

Launcher::Launcher(MainWindow* mainWindow)
    : Fm::FileLauncher(),
      mainWindow_(mainWindow),
      openInNewTab_(false)
{
    Application* app = static_cast<Application*>(qApp);
    setQuickExec(app->settings().quickExec());
}

Launcher::~Launcher()
{

}

bool Launcher::openFolder(GAppLaunchContext* /*ctx*/, const Fm::FileInfoList& folderInfos, Fm::GErrorPtr& /*err*/)
{
    auto fi = folderInfos[0];
    Application* app = static_cast<Application*>(qApp);
    MainWindow* mainWindow = mainWindow_;
    Fm::FilePath path = fi->path();
    if (!mainWindow) {
        mainWindow = new MainWindow(std::move(path));
        mainWindow->resize(app->settings().windowWidth(), app->settings().windowHeight());

        if (app->settings().windowMaximized()) {
            mainWindow->setWindowState(mainWindow->windowState() | Qt::WindowMaximized);
        }
    } else {
        if (openInNewTab_) {
            mainWindow_->addTabWithPage(std::move(path));
        } else {
            mainWindow_->chdir(std::move(path));
        }
    }

    for (size_t i = 1; i < folderInfos.size(); ++i) {
        fi = folderInfos[i];
        path = fi->path();
        mainWindow_->addTabWithPage(std::move(path));
    }

    mainWindow->show();
    mainWindow->raise();
    mainWindow->activateWindow();
    openInNewTab_ = false;

    return true;
}
