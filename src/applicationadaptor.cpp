/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -m -a applicationadaptor -c ApplicationAdaptor -i application.h -l Application /home/rekols/Projects/panda-files/src/org.panda.files.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#include "applicationadaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ApplicationAdaptor
 */

ApplicationAdaptor::ApplicationAdaptor(Application *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ApplicationAdaptor::~ApplicationAdaptor()
{
    // destructor
}

bool ApplicationAdaptor::desktopManagerEnabled() const
{
    // get the value of property desktopManagerEnabled
    return qvariant_cast< bool >(parent()->property("desktopManagerEnabled"));
}

QString ApplicationAdaptor::wallpaper() const
{
    // get the value of property wallpaper
    return qvariant_cast< QString >(parent()->property("wallpaper"));
}

void ApplicationAdaptor::desktopManager(bool in0)
{
    // handle method call org.panda.Files.desktopManager
    parent()->desktopManager(in0);
}

void ApplicationAdaptor::desktopPrefrences(const QString &in0)
{
    // handle method call org.panda.Files.desktopPrefrences
    parent()->desktopPrefrences(in0);
}

void ApplicationAdaptor::emptyTrash()
{
    // handle method call org.panda.Files.emptyTrash
    parent()->emptyTrash();
}

void ApplicationAdaptor::findFiles(const QStringList &in0)
{
    // handle method call org.panda.Files.findFiles
    parent()->findFiles(in0);
}

void ApplicationAdaptor::launchFiles(const QString &in0, const QStringList &in1, bool in2)
{
    // handle method call org.panda.Files.launchFiles
    parent()->launchFiles(in0, in1, in2);
}

void ApplicationAdaptor::preferences(const QString &in0)
{
    // handle method call org.panda.Files.preferences
    parent()->preferences(in0);
}

void ApplicationAdaptor::quit()
{
    // handle method call org.panda.Files.quit
    parent()->quit();
}

void ApplicationAdaptor::setWallpaper(const QString &in0)
{
    // handle method call org.panda.Files.setWallpaper
    parent()->setWallpaper(in0);

    Q_EMIT wallpaperChanged();
}