#include "xdgdir.h"
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QSaveFile>

QString XdgDir::readUserDirsFile()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/user-dirs.dirs"));

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        file.close();

        return QString::fromLocal8Bit(data);
    }

    return QString();
}

QString XdgDir::readDesktopDir()
{
    QString str = readUserDirsFile();
    if (str.isEmpty())
        return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/Desktop");

    QRegExp reg(QStringLiteral("XDG_DESKTOP_DIR=\"([^\n]*)\""));
    if (reg.lastIndexIn(str) != -1) {
        str = reg.cap(1);
        if (str.startsWith(QStringLiteral("$HOME")))
            str = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + str.mid(5);
        return str;
    }

    return QString();
}

void XdgDir::setDesktopDir(QString path)
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (path.startsWith(home))
        path = QStringLiteral("$HOME") + path.mid(home.length());
    QString str = readUserDirsFile();
    QRegExp reg(QStringLiteral("XDG_DESKTOP_DIR=\"([^\n]*)\""));
    QString line = QStringLiteral("XDG_DESKTOP_DIR=\"") + path + QLatin1Char('\"');
    if (reg.indexIn(str) != -1)
        str.replace(reg, line);
    else {
        if (!str.endsWith(QLatin1Char('\n')))
            str += QLatin1Char('\n');
        str += line + QLatin1Char('\n');
    }
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (QDir().mkpath(dir)) { // write the file
        QSaveFile file(dir + QStringLiteral("/user-dirs.dirs"));
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(str.toLocal8Bit());
            file.commit();
        }
    }
}
