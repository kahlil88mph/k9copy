//
// C++ Implementation: k9cddrive
//
// Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2007
// 2026: Solid replaced by libudev (enumeration, hotplug, media changes).
//
// Copyright: See COPYING file that comes with this distribution
//
#include "k9common.h"
#include "k9cddrive.h"
#include "k9config.h"
#include "k9tools.h"
#include "k9process.h"
#include "k9i18n.h"
#include "k9kshell.h"
#include <QFileInfo>
#include <QStandardPaths>

#include <QDir>
#include <QSocketNotifier>
#include <QStorageInfo>

#include <libudev.h>

static bool udevFlag(udev_device *dev, const char *prop)
{
    const char *v = udev_device_get_property_value(dev, prop);
    return v && qstrcmp(v, "1") == 0;
}

static QString udevString(udev_device *dev, const char *prop)
{
    const char *v = udev_device_get_property_value(dev, prop);
    return v ? QString::fromUtf8(v).replace(QLatin1Char('_'), QLatin1Char(' ')).trimmed() : QString();
}

k9CdDrive::k9CdDrive()
{
    num = 0;
    canReadDVD = false;
    canWriteCDR = false;
    canWriteDVD = false;
}
k9CdDrive::~k9CdDrive() {}

QList<int> k9CdDrive::getWriteSpeeds() const { return writeSpeeds; }
void k9CdDrive::setWriteSpeeds(const QList<int> &_value) { writeSpeeds = _value; }

void k9CdDrive::setReadAhead(const QString &_device, int _value)
{
    QString hdparm = QStandardPaths::findExecutable(QStringLiteral("hdparm"));
    if (hdparm.isEmpty())
        return;
    QString cmd = QStringLiteral("%1 -a%2 %3").arg(hdparm).arg(_value).arg(KShell::quoteArg(_device));
    k9Tools::runAsRoot(cmd, i18n("The root password is needed to change the read ahead property of the dvd drive"));
}

// ---------------------------------------------------------------- drives

k9CdDrives::k9CdDrives(QObject *parent) : QObject(parent)
{
    m_udev = udev_new();
    if (m_udev) {
        m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
        if (m_monitor) {
            udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "block", "disk");
            udev_monitor_enable_receiving(m_monitor);
            m_notifier = new QSocketNotifier(udev_monitor_get_fd(m_monitor), QSocketNotifier::Read, this);
            connect(m_notifier, &QSocketNotifier::activated, this, &k9CdDrives::udevEvent);
        }
    }
    scanDrives();
}

k9CdDrives::~k9CdDrives()
{
    clearDrives();
    if (m_monitor)
        udev_monitor_unref(m_monitor);
    if (m_udev)
        udev_unref(m_udev);
}

void k9CdDrives::clearDrives()
{
    while (!drives.isEmpty())
        delete drives.takeFirst();
}

k9CdDrive *k9CdDrives::driveFromUdev(udev_device *dev)
{
    if (!udevFlag(dev, "ID_CDROM"))
        return nullptr;
    const char *node = udev_device_get_devnode(dev);
    if (!node)
        return nullptr;

    auto *drive = new k9CdDrive;
    drive->device = QString::fromUtf8(node);
    drive->syspath = QString::fromUtf8(udev_device_get_syspath(dev));
    QString vendor = udevString(dev, "ID_VENDOR");
    QString model = udevString(dev, "ID_MODEL");
    drive->name = (vendor + QLatin1Char(' ') + model).trimmed();
    if (drive->name.isEmpty())
        drive->name = drive->device;
    drive->canReadDVD = udevFlag(dev, "ID_CDROM_DVD");
    drive->canWriteCDR = udevFlag(dev, "ID_CDROM_CD_R");
    drive->canWriteDVD = udevFlag(dev, "ID_CDROM_DVD_R") || udevFlag(dev, "ID_CDROM_DVD_PLUS_R");
    return drive;
}

k9CdDrive *k9CdDrives::findBySyspath(const QString &syspath) const
{
    for (k9CdDrive *d : drives)
        if (!d->syspath.isEmpty() && d->syspath == syspath)
            return d;
    return nullptr;
}

void k9CdDrives::scanDrives()
{
    clearDrives();
    if (m_udev) {
        udev_enumerate *en = udev_enumerate_new(m_udev);
        udev_enumerate_add_match_subsystem(en, "block");
        udev_enumerate_add_match_property(en, "ID_CDROM", "1");
        udev_enumerate_scan_devices(en);
        udev_list_entry *entry;
        udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(en)) {
            udev_device *dev = udev_device_new_from_syspath(m_udev, udev_list_entry_get_name(entry));
            if (!dev)
                continue;
            if (k9CdDrive *drive = driveFromUdev(dev)) {
                drive->num = drives.count();
                drives.append(drive);
                Q_EMIT deviceAdded(drive);
            }
            udev_device_unref(dev);
        }
        udev_enumerate_unref(en);
    }
    readConfig();
}

void k9CdDrives::udevEvent()
{
    udev_device *dev = udev_monitor_receive_device(m_monitor);
    if (!dev)
        return;
    const QString action = QString::fromUtf8(udev_device_get_action(dev));
    const QString syspath = QString::fromUtf8(udev_device_get_syspath(dev));

    if (action == QLatin1String("add")) {
        if (!findBySyspath(syspath)) {
            if (k9CdDrive *drive = driveFromUdev(dev)) {
                drive->num = drives.count();
                drives.append(drive);
                Q_EMIT deviceAdded(drive);
            }
        }
    } else if (action == QLatin1String("remove")) {
        if (k9CdDrive *d = findBySyspath(syspath)) {
            Q_EMIT deviceRemoved(d);
            drives.removeAll(d);
            d->deleteLater();
        }
    } else if (action == QLatin1String("change")) {
        // Media inserted or ejected.
        if (k9CdDrive *d = findBySyspath(syspath)) {
            QString label;
            if (udevFlag(dev, "ID_CDROM_MEDIA"))
                label = QString::fromUtf8(udev_device_get_property_value(dev, "ID_FS_LABEL") ?: "");
            Q_EMIT d->volumeChanged(d->device, label);
        }
    }
    udev_device_unref(dev);
}

void k9CdDrives::eject(const QString &device)
{
    k9Process process;
    process << "eject" << device;
    process.start();
    process.wait();
}

/** reads devices that were entered manually **/
void k9CdDrives::readConfig()
{
    k9Config config;
    const QStringList ldev = config.getDevices();
    const QStringList llabels = config.getDevicesLabels();
    const QStringList lIO = config.getDevicesIO();
    for (int i = 0; i < ldev.count(); ++i) {
        auto *drive = new k9CdDrive;
        drive->device = ldev.at(i);
        drive->name = llabels.value(i);
        const QString io = lIO.value(i);
        drive->canReadDVD = io.contains(QLatin1Char('I'));
        drive->canWriteCDR = drive->canWriteDVD = io.contains(QLatin1Char('O'));
        drive->num = drives.count();
        drives.append(drive);
        Q_EMIT deviceAdded(drive);
    }
}

int k9CdDrives::count() { return drives.count(); }

k9CdDrive *k9CdDrives::getDrive(int num) { return drives.at(num); }

k9CdDrive *k9CdDrives::getDeviceFromMountpoint(k9CdDrives *drives, const QString &mountpoint)
{
    QStorageInfo info(mountpoint);
    if (!info.isValid())
        return nullptr;
    const QString dev = QString::fromLocal8Bit(info.device());
    for (k9CdDrive *d : drives->drives)
        if (QDir::cleanPath(QFileInfo(d->device).canonicalFilePath()) == QDir::cleanPath(QFileInfo(dev).canonicalFilePath()))
            return d;
    return nullptr;
}
