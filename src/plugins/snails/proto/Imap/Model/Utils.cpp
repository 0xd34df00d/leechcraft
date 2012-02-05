/* Copyright (C) 2006 - 2011 Jan Kundrát <jkt@gentoo.org>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or the version 3 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "Utils.h"
#include <cmath>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>
#include <QSysInfo>

#ifdef TROJITA_MOBILITY_SYSTEMINFO
#include <QSystemDeviceInfo>
#endif

namespace Imap {
namespace Mailbox {

QString PrettySize::prettySize(uint bytes)
{
    if (bytes == 0) {
        return tr("0");
    }
    int order = std::log( static_cast<double>(bytes) ) / std::log(1024.0);
    QString suffix;
    if (order <= 0) {
        return QString::number(bytes);
    } else if (order == 1) {
        suffix = tr("kB");
    } else if (order == 2) {
        suffix = tr("MB");
    } else if (order == 3) {
        suffix = tr("GB");
    } else {
        // make sure not to show wrong size for those that have > 1024 TB e-mail messages
        order = 4;
        suffix = tr("TB"); // shame on you for such mails
    }
    return tr("%1 %2").arg(QString::number(bytes / std::pow(1024.0, order), 'f', 1), suffix);
}

QString persistentLogFileName()
{
    QString logFileName = QDesktopServices::storageLocation( QDesktopServices::CacheLocation );
    if (logFileName.isEmpty()) {
        logFileName = QDir::homePath() + QLatin1String("/.trojita-connection-log");
    } else {
        QDir().mkpath(logFileName);
        logFileName += QString::fromAscii("/trojita-connection-log");
    }
    return logFileName;
}

QString systemPlatformVersion()
{
    QString os = QString::fromAscii(""
#ifdef Q_OS_AIX
"AIX"
#endif
#ifdef Q_OS_BSD4
  #ifndef Q_OS_MAC
  "AnyBSD4.4"
  #endif
#endif
#ifdef Q_OS_BSDI
"BSD/OS"
#endif
#ifdef Q_OS_CYGWIN
"Cygwin"
#endif
#ifdef Q_OS_DGUX
"DG/UX"
#endif
#ifdef Q_OS_DYNIX
"DYNIX/ptx"
#endif
#ifdef Q_OS_FREEBSD
"FreeBSD"
#endif
#ifdef Q_OS_HPUX
"HP-UX"
#endif
#ifdef Q_OS_HURD
"Hurd"
#endif
#ifdef Q_OS_IRIX
"Irix"
#endif
#ifdef Q_OS_LINUX
"Linux"
#endif
#ifdef Q_OS_LYNX
"LynxOS"
#endif
#ifdef Q_OS_MAC
"MacOS"
#endif
#ifdef Q_OS_MSDOS
"MSDOS"
#endif
#ifdef Q_OS_NETBSD
"NetBSD"
#endif
#ifdef Q_OS_OS2
"OS/2"
#endif
#ifdef Q_OS_OPENBSD
"OpenBSD"
#endif
#ifdef Q_OS_OS2EMX
"OS2EMX"
#endif
#ifdef Q_OS_OSF
"HPTru64UNIX"
#endif
#ifdef Q_OS_QNX
"QNXNeutrino"
#endif
#ifdef Q_OS_RELIANT
"ReliantUNIX"
#endif
#ifdef Q_OS_SCO
"SCOOpenServer5"
#endif
#ifdef Q_OS_SOLARIS
"Solaris"
#endif
#ifdef Q_OS_SYMBIAN
"Symbian"
#endif
#ifdef Q_OS_ULTRIX
"Ultrix"
#endif
#ifdef Q_OS_UNIXWARE
"Unixware"
#endif
#ifdef Q_OS_WIN32
"Windows"
#endif
#ifdef Q_OS_WINCE
"WinCE"
#endif
);
#ifdef Q_OS_UNIX
    if (os.isEmpty()) {
        os = "Unix";
    }
#endif

    QString ws = ""
#ifdef Q_WS_X11
"X11"
#endif
#ifdef Q_WS_S60
"S60"
#endif
#ifdef Q_WS_MAC
"Mac"
#endif
#ifdef Q_WS_QWS
"QWS"
#endif
#ifdef Q_WS_WIN
"Win"
#endif
;

    static QString platformVersion;
#ifdef TROJITA_MOBILITY_SYSTEMINFO
    if (platformVersion.isEmpty()) {
        QtMobility::QSystemDeviceInfo device;
        if (device.productName().isEmpty()) {
            platformVersion = device.manufacturer() + QLatin1String(" ") + device.model();
        } else {
            platformVersion = QString::fromAscii("%1 %2 (%3)").arg(device.manufacturer(), device.productName(), device.model());
        }
    }
#endif
    if (platformVersion.isEmpty()) {
#ifdef Q_OS_WIN32
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_32s:
        platformVersion = "3.1";
        break;
    case QSysInfo::WV_95:
        platformVersion = "95";
        break;
    case QSysInfo::WV_98:
        platformVersion = "98";
        break;
    case QSysInfo::WV_Me:
        platformVersion = "Me";
        break;
    case QSysInfo::WV_NT:
        platformVersion = "NT";
        break;
    case QSysInfo::WV_2000:
        platformVersion = "2000";
        break;
    case QSysInfo::WV_XP:
        platformVersion = "XP";
        break;
    case QSysInfo::WV_2003:
        platformVersion = "2003";
        break;
    case QSysInfo::WV_VISTA:
        platformVersion = "Vista";
        break;
    case QSysInfo::WV_WINDOWS7:
        platformVersion = "7";
        break;
    }
#endif
#ifdef Q_OS_WINCE
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_CE:
        platformVersion = "CE";
        break;
    case QSysInfo::WV_CENET:
        platformVersion = "CE.NET";
        break;
    case QSysInfo::WV_CE_5:
        platformVersion = "CE5.x";
        break;
    case QSysInfo::WV_CE_6:
        platformVersion = "CE6.x";
        break;
    }
#endif
#ifdef Q_WS_S60
    switch (QSysInfo:s60Version()) {
    case QSysInfo::SV_S60_3_1:
        platformVersion = "S60r3fp1";
        break;
    case QSysInfo::SV_S60_3_2:
        platformVersion = "S60r3fp2";
        break;
    case QSysInfo::SV_S60_5_0:
        platformVersion = "S60r5";
        break;
    case QSysInfo::SV_S60_5_1:
        platformVersion = "S60r5fp1";
        break;
    case QSysInfo::SV_S60_5_2:
        platformVersion = "S60r5fp2";
        break;
    case QSysInfo::SV_S60_Unnown:
        platformVersion = "SV_Unknown";
        break;
    }
#endif
#ifdef Q_OS_SYMBIAN
    switch (QSysInfo::symbianVersion()) {
    case QSysInfo::SV_SF_1:
        platformVersion = "Symbian^1";
        break;
    case QSysInfo::SV_SF_2:
        platformVersion = "Symbian^2";
        break;
    case QSysInfo::SV_SF_3:
        platformVersion = "Symbian^3";
        break;
    case QSysInfo::SV_SF_4:
        platformVersion = "Symbian^4";
        break;
    }
#endif
#ifdef Q_OS_MAC
    switch (QSysInfo::MacintoshVersion) {
    case QSysInfo::MV_9:
        platformVersion = "9.0";
        break;
    case QSysInfo::MV_10_0:
        platformVersion = "X 10.0";
        break;
    case QSysInfo::MV_10_1:
        platformVersion = "X 10.1";
        break;
    case QSysInfo::MV_10_2:
        platformVersion = "X 10.2";
        break;
    case QSysInfo::MV_10_3:
        platformVersion = "X 10.3";
        break;
    case QSysInfo::MV_10_4:
        platformVersion = "X 10.4";
        break;
    case QSysInfo::MV_10_5:
        platformVersion = "X 10.5";
        break;
    case QSysInfo::MV_10_6:
        platformVersion = "X 10.6";
        break;
    }
#endif
    if (platformVersion.isEmpty()) {
        // try to call the lsb_release
        QProcess *proc = new QProcess(0);
        proc->start("lsb_release", QStringList() << QLatin1String("-s") << QLatin1String("-d"));
        proc->waitForFinished();
        platformVersion = QString::fromLocal8Bit(proc->readAll()).trimmed().replace(QLatin1String("\""), QString()).replace(QLatin1String(";"), QLatin1String(","));
        proc->deleteLater();
    }
    }
    return QString::fromAscii("Qt/%1; %2; %3; %4").arg(qVersion(), ws, os, platformVersion);
}

}

}
