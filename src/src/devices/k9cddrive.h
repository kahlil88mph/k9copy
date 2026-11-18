//
// C++ Interface: k9cddrive
//
// Developer: Pauline123 <pauline123@users.sf.net>, (C) 2014
// Author: Jean-Michel PETIT <k9copy@free.fr>, (C) 2007
// 2026: Solid replaced by libudev (enumeration, hotplug, media changes).
//
// Copyright: See COPYING file that comes with this distribution
//
#ifndef K9CDDRIVE_H
#define K9CDDRIVE_H
#include "k9common.h"
#include "config.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

struct udev;
struct udev_device;
struct udev_monitor;
class QSocketNotifier;
class k9CdDrives;

class k9CdDrive : public QObject {
    Q_OBJECT
    friend class k9CdDrives;
public:
    k9CdDrive();
    ~k9CdDrive();
    int num;
    QString name;
    QString device;     // device node, e.g. /dev/sr0
    QString syspath;    // udev syspath; empty for manually configured drives
    bool canWriteCDR;
    bool canReadDVD;
    bool canWriteDVD;

    static void setReadAhead(const QString &_device, int _value);
    void setWriteSpeeds(const QList<int> &_value);
    QList<int> getWriteSpeeds() const;

private:
    QList<int> writeSpeeds;
Q_SIGNALS:
    void volumeChanged(const QString &device, const QString &volumeName);
};

class k9CdDrives : public QObject {
    Q_OBJECT
public:
    k9CdDrives(QObject *parent);
    ~k9CdDrives();
    int count();
    k9CdDrive *getDrive(int num);
    static void eject(const QString &device);
    void scanDrives();
    void clearDrives();
    static k9CdDrive *getDeviceFromMountpoint(k9CdDrives *drives, const QString &mountpoint);

Q_SIGNALS:
    void deviceAdded(k9CdDrive *drive);
    void deviceRemoved(k9CdDrive *drive);

private Q_SLOTS:
    void udevEvent();

private:
    void readConfig();
    k9CdDrive *driveFromUdev(udev_device *dev);
    k9CdDrive *findBySyspath(const QString &syspath) const;
    QList<k9CdDrive *> drives;
    udev *m_udev = nullptr;
    udev_monitor *m_monitor = nullptr;
    QSocketNotifier *m_notifier = nullptr;
};
#endif
