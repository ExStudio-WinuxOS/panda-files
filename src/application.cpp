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

#include "application.h"
#include "mainwindow.h"
#include "launcher.h"

// #include mountoperation.h>

#include "lib/core/terminal.h"
#include "lib/core/bookmarks.h"
#include "lib/core/folderconfig.h"
#include "lib/filesearchdialog.h"
#include "lib/fileoperation.h"

// Qt
#include <QPixmapCache>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QMessageBox>
#include <QWindow>
#include <QDebug>

// Generated header file
#include "applicationadaptor.h"

static const char* serviceName = "org.panda.files";
static const char* ifaceName = "org.panda.Files";

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv),
      m_daemonMode(false),
      m_enableDesktopManager(false)
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService(QLatin1String(serviceName))) {
        m_isInstance = true;

        new ApplicationAdaptor(this);
        dbus.registerObject(QStringLiteral("/Files"), this);

        connect(this, &Application::aboutToQuit, this, &Application::onAboutToQuit);

        installSigtermHandler();

    } else {
        // an service of the same name is already registered.
        // we're not the first instance
        m_isInstance = false;
    }
}

Application::~Application()
{
}

void Application::init()
{

}

int Application::exec()
{
    if(!parseCommandLineArgs()) {
        return 0;
    }

    // keep running even when there is no window opened.
    if (m_daemonMode) {
        setQuitOnLastWindowClosed(false);
    }

    return QCoreApplication::exec();
}

void Application::launchFiles(QString cwd, QStringList paths, bool inNewWindow)
{
    Fm::FilePathList pathList;
    Fm::FilePath cwd_path;
    QStringList::iterator it;

    for (const QString& it : qAsConst(paths)) {
        QByteArray pathName = it.toLocal8Bit();
        Fm::FilePath path;

        if (pathName == "~") { // special case for home dir
            path = Fm::FilePath::homeDir();
        }

        if (pathName[0] == '/') { // absolute path
            path = Fm::FilePath::fromLocalPath(pathName.constData());
        } else if (pathName.contains(":/")) { // URI
            path = Fm::FilePath::fromUri(pathName.constData());
        } else { // basename
            if (Q_UNLIKELY(!cwd_path)) {
                cwd_path = Fm::FilePath::fromLocalPath(cwd.toLocal8Bit().constData());
            }
            path = cwd_path.relativePath(pathName.constData());
        }
        pathList.push_back(std::move(path));
    }

    Launcher(nullptr).launchPaths(nullptr, pathList);
}

void Application::setWallpaper(QString path)
{
    bool changed = false;

    if (!path.isEmpty() && path != m_settings.wallpaper()) {
        if (QFile(path).exists()) {
            m_settings.setWallpaper(path);
            changed = true;
        }
    }

    if (!changed)
        return;

    // FIXME: support different wallpapers on different screen.
    // update wallpaper
    if (m_enableDesktopManager) {
        for (DesktopWindow* desktopWin :  m_desktopWindows) {
            desktopWin->setWallpaperFile(path);
            desktopWin->updateWallpaper();
            desktopWin->update();
        }

        m_settings.save(); // save the settings to the config file
    }
}

void Application::preferences(QString page)
{

}

void Application::desktopPrefrences(QString page)
{

}

void Application::editBookmarks()
{

}

void Application::desktopManager(bool enabled)
{
    if (enabled) {
        if (!m_enableDesktopManager) {
            const auto allScreens = screens();
            for (QScreen *screen : allScreens) {
                connect(screen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
                connect(screen, &QScreen::availableGeometryChanged, this, &Application::onAvailableGeometryChanged);
                connect(screen, &QObject::destroyed, this, &Application::onScreenDestroyed);
            }
            connect(this, &QApplication::screenAdded, this, &Application::onScreenAdded);
            connect(this, &QApplication::screenRemoved, this, &Application::onScreenRemoved);

            if (primaryScreen() && primaryScreen()->virtualSiblings().size() > 1) {
                DesktopWindow *window = createDesktopWindow(-1);
                m_desktopWindows.push_back(window);
            } else {
                int n = qMax(allScreens.size(), 1);
                m_desktopWindows.reserve(n);
                for (int i = 0; i < n; ++i) {
                    DesktopWindow *window = createDesktopWindow(i);
                    m_desktopWindows.push_back(window);
                }
            }
        }
    } else {
        if (m_enableDesktopManager) {
            int n = m_desktopWindows.size();
            for (int i = 0; i < n; ++i) {
                DesktopWindow *window = m_desktopWindows.at(i);
                delete window;
            }
            m_desktopWindows.clear();
            const auto allScreens = screens();
            for(QScreen* screen : allScreens) {
                disconnect(screen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
                disconnect(screen, &QScreen::availableGeometryChanged, this, &Application::onAvailableGeometryChanged);
                disconnect(screen, &QObject::destroyed, this, &Application::onScreenDestroyed);
            }
            disconnect(this, &QApplication::screenAdded, this, &Application::onScreenAdded);
            disconnect(this, &QApplication::screenRemoved, this, &Application::onScreenRemoved);
        }
    }

    m_enableDesktopManager = enabled;
}

void Application::findFiles(QStringList paths)
{

}

void Application::connectToServer()
{

}

void Application::emptyTrash()
{
    Fm::FileOperation::emptyTrash(true, nullptr);
}

void Application::openFolders(Fm::FileInfoList files)
{
    Launcher(nullptr).launchFiles(nullptr, std::move(files));
}

void Application::openFolderInTerminal(Fm::FilePath path)
{
    if (!m_settings.terminal().isEmpty()) {
        Fm::GErrorPtr err;
        auto terminalName = m_settings.terminal().toUtf8();

        if (!Fm::launchTerminal(terminalName.constData(), path, err)) {
            QMessageBox::critical(nullptr, tr("Error"), err.message());
        }
    }
    else {
        // show an error message and ask the user to set the command
        QMessageBox::critical(nullptr, tr("Error"), tr("Terminal emulator is not set."));
        preferences(QStringLiteral("advanced"));
    }
}

bool Application::parseCommandLineArgs()
{
    bool keepRunning = false;
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption desktopOption(QStringLiteral("desktop"), tr("Launch desktop manager"));
    parser.addOption(desktopOption);

    QCommandLineOption newWindowOption(QStringList() << QStringLiteral("n") << QStringLiteral("new-window"), tr("Open new window"));
    parser.addOption(newWindowOption);

    QCommandLineOption setWallpaperOption(QStringList() << QStringLiteral("w") << QStringLiteral("set-wallpaper"), tr("Set desktop wallpaper from image FILE"), tr("FILE"));
    parser.addOption(setWallpaperOption);

    parser.addPositionalArgument(QStringLiteral("files"), tr("Files or directories to open"), tr("[FILE1, FILE2,...]"));
    parser.process(arguments());

    if (m_isInstance) {
        qDebug() << "is primary instance";

        m_settings.load();

        // decrease the cache size to reduce memory usage
        QPixmapCache::setCacheLimit(2048);

        // desktop icon management
        if (parser.isSet(desktopOption)) {
            desktopManager(true);
            keepRunning = true;
        } else if(parser.isSet(setWallpaperOption)) {
            setWallpaper(parser.value(setWallpaperOption));
        } else {
            if (!parser.isSet(desktopOption)) {
                QStringList paths = parser.positionalArguments();
                if (paths.isEmpty()) {
                    // if no path is specified and we're using daemon mode,
                    // don't open current working directory
                    if(!m_daemonMode) {
                        paths.push_back(QDir::currentPath());
                    }
                }
                if(!paths.isEmpty()) {
                    launchFiles(QDir::currentPath(), paths, parser.isSet(newWindowOption));
                }
                keepRunning = true;
            }
        }
    } else {
        QDBusConnection dbus = QDBusConnection::sessionBus();
        QDBusInterface iface(QLatin1String(serviceName), QStringLiteral("/Files"), QLatin1String(ifaceName), dbus, this);

        if (parser.isSet(desktopOption)) {
            iface.call(QStringLiteral("desktopManager"), true);
        } else if(parser.isSet(setWallpaperOption)) { // set wall paper
            iface.call("setWallpaper", parser.value(setWallpaperOption));
        } else {
            if (!parser.isSet(desktopOption)) {
                QStringList paths = parser.positionalArguments();
                if (paths.isEmpty()) {
                    paths.push_back(QDir::currentPath());
                }
                iface.call(QStringLiteral("launchFiles"), QDir::currentPath(), paths, parser.isSet(newWindowOption));
            }
        }
    }

    return keepRunning;
}

DesktopWindow *Application::createDesktopWindow(int screenNum)
{
    DesktopWindow *window = new DesktopWindow(screenNum);

    // one large virtual desktop only
    if (screenNum == -1) {
        QRect rect = primaryScreen()->virtualGeometry();
        window->setGeometry(rect);
    } else {
        QRect rect;
        const auto allScreens = screens();

        if (auto screen = window->getDesktopScreen()) {
            rect = screen->geometry();
        }
        window->setGeometry(rect);
    }

    window->updateFromSettings(m_settings);
    window->show();

    return window;
}

void Application::installSigtermHandler()
{

}

void Application::updateFromSettings()
{
    // update main windows and desktop windows
    QWidgetList windows = this->topLevelWidgets();
    QWidgetList::iterator it;
    for (it = windows.begin(); it != windows.end(); ++it) {
        QWidget* window = *it;
        if (window->inherits("PandaFiles")) {
            MainWindow* mainWindow = static_cast<MainWindow*>(window);
            // mainWindow->updateFromSettings(m_settings);
        }
    }

    if (desktopManagerEnabled()) {
        updateDesktopsFromSettings();
    }
}

void Application::updateDesktopsFromSettings(bool changeSlide)
{
    QVector<DesktopWindow*>::iterator it;
    for(it = m_desktopWindows.begin(); it != m_desktopWindows.end(); ++it) {
        DesktopWindow* desktopWin = static_cast<DesktopWindow*>(*it);
        desktopWin->updateFromSettings(m_settings, changeSlide);
    }

}

void Application::onVirtualGeometryChanged(const QRect &rect)
{
    if (m_enableDesktopManager) {
        for (DesktopWindow *desktopWin : qAsConst(m_desktopWindows)) {
            auto desktopScreen = desktopWin->getDesktopScreen();
            if (desktopScreen) {
                desktopWin->setGeometry(desktopScreen->virtualGeometry());
            }
        }
    }
}

void Application::onAvailableGeometryChanged(const QRect &rect)
{
    // update desktop layouts
    if (m_enableDesktopManager) {
        for(DesktopWindow *desktopWin : qAsConst(m_desktopWindows)) {
            desktopWin->queueRelayout();
        }
    }
}

void Application::onScreenDestroyed(QObject *screenObj)
{
    if (m_enableDesktopManager) {
        bool reloadNeeded = false;
        // FIXME: add workarounds for Qt5 bug #40681 and #40791 here.
        for (DesktopWindow *desktopWin :  qAsConst(m_desktopWindows)) {
            if (desktopWin->windowHandle()->screen() == screenObj) {
                desktopWin->destroy(); // destroy the underlying native window
                reloadNeeded = true;
            }
        }

        if (reloadNeeded) {
            QTimer::singleShot(0, this, SLOT(reloadDesktopsAsNeeded()));
        }
    }
}

void Application::onScreenAdded(QScreen *newScreen)
{
    if (m_enableDesktopManager) {
        connect(newScreen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
        connect(newScreen, &QScreen::availableGeometryChanged, this, &Application::onAvailableGeometryChanged);
        connect(newScreen, &QObject::destroyed, this, &Application::onScreenDestroyed);
        const auto siblings = primaryScreen()->virtualSiblings();
        if(siblings.contains(newScreen)) { // the primary screen is changed
            if (m_desktopWindows.size() == 1) {
                m_desktopWindows.at(0)->setGeometry(newScreen->virtualGeometry());
                if(siblings.size() > 1) { // a virtual desktop is created
                    m_desktopWindows.at(0)->setScreenNum(-1);
                }
            }
            else if (m_desktopWindows.isEmpty()) { // for the sake of certainty
                DesktopWindow* window = createDesktopWindow(m_desktopWindows.size());
                m_desktopWindows.push_back(window);
            }
        }
        else { // a separate screen is added
            DesktopWindow* window = createDesktopWindow(m_desktopWindows.size());
            m_desktopWindows.push_back(window);
        }
    }
}

void Application::onScreenRemoved(QScreen *oldScreen)
{
    if(m_enableDesktopManager){
        disconnect(oldScreen, &QScreen::virtualGeometryChanged, this, &Application::onVirtualGeometryChanged);
        disconnect(oldScreen, &QScreen::availableGeometryChanged, this, &Application::onAvailableGeometryChanged);
        disconnect(oldScreen, &QObject::destroyed, this, &Application::onScreenDestroyed);
        if(m_desktopWindows.isEmpty()) {
            return;
        }
        if(m_desktopWindows.size() == 1) { // a single desktop is changed
            if(primaryScreen() != nullptr) {
                m_desktopWindows.at(0)->setGeometry(primaryScreen()->virtualGeometry());
                if(primaryScreen()->virtualSiblings().size() == 1) {
                    m_desktopWindows.at(0)->setScreenNum(0); // there is no virtual desktop anymore
                }
            }
            else if (screens().isEmpty()) { // for the sake of certainty
                m_desktopWindows.at(0)->setScreenNum(0);
            }
        }
        else { // a separate desktop is removed
            int n = m_desktopWindows.size();
            for(int i = 0; i < n; ++i) {
                DesktopWindow* window = m_desktopWindows.at(i);
                if(window->getDesktopScreen() == oldScreen) {
                    m_desktopWindows.remove(i);
                    delete window;
                    break;
                }
            }
        }
    }
}

void Application::reloadDesktopsAsNeeded()
{
    if (m_enableDesktopManager) {
        // workarounds for Qt5 bug #40681 and #40791 here.
        for (DesktopWindow* desktopWin : qAsConst(m_desktopWindows)) {
            if (!desktopWin->windowHandle()) {
                desktopWin->create(); // re-create the underlying native window
                desktopWin->queueRelayout();
                desktopWin->show();
            }
        }
    }
}

void Application::onAboutToQuit()
{
    m_settings.save();
}
