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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include "settings.h"
#include "desktopwindow.h"

class Application : public QApplication
{
    Q_OBJECT
    Q_PROPERTY(bool desktopManagerEnabled READ desktopManagerEnabled)
    Q_PROPERTY(QString wallpaper READ wallpaper)

public:
    Application(int& argc, char** argv);
    ~Application();

    void init();
    int exec();

    // public interface exported via dbus
    void launchFiles(QString cwd, QStringList paths, bool inNewWindow);
    void setWallpaper(QString path);
    void preferences(QString page);
    void desktopPrefrences(QString page);
    void editBookmarks();
    void desktopManager(bool enabled);
    void findFiles(QStringList paths = QStringList());
    void connectToServer();

    void openFolders(Fm::FileInfoList files);
    void openFolderInTerminal(Fm::FilePath path);

    bool desktopManagerEnabled() { return m_enableDesktopManager; }
    QString wallpaper() { return m_settings.wallpaper(); }

    Settings &settings() { return m_settings; };

protected:
    bool parseCommandLineArgs();
    DesktopWindow *createDesktopWindow(int screenNum);

private:
    void installSigtermHandler();
    void updateFromSettings();
    void updateDesktopsFromSettings(bool changeSlide = true);

    void onVirtualGeometryChanged(const QRect &rect);
    void onAvailableGeometryChanged(const QRect &rect);
    void onScreenDestroyed(QObject *screenObj);
    void onScreenAdded(QScreen *newScreen);
    void onScreenRemoved(QScreen *oldScreen);
    void reloadDesktopsAsNeeded();

    void onAboutToQuit();

private:
    Settings m_settings;
    bool m_isInstance;
    bool m_daemonMode;
    bool m_enableDesktopManager;

    QVector<DesktopWindow*> m_desktopWindows;
};

#endif
