#include "notificationsmanager.h"
#include "kineticnotification.h"
#include <QDebug>
#include <QFile>
#include <QTextDocument>
#include <QApplication>
#include <QMouseEvent>
#include <QFileInfo>
#include <QTimer>
#include "notificationwidget.h"

// the universe's only Notification manager
NotificationsManager *NotificationsManager::instance = 0;

NotificationsManager::NotificationsManager()
{
    loadSettings();
}

KineticNotification *NotificationsManager::getById ( const QString& id ) const
{
    int number = getNumber(id);
    return (number == -1) ? 0 : active_notifications.value(number);
}

int NotificationsManager::getNumber ( const QString& id ) const
{
    for (int i=0;i!=active_notifications.count();i++) {
        if (active_notifications.value(i)->getId() == id)
            return i;
    }
    return -1;
}

KineticNotification* NotificationsManager::getByNumber ( const int& number ) const
{
    return active_notifications.value(number);
}

QRect NotificationsManager::insert ( KineticNotification* notification )
{
    if (getNumber(notification->getId()) != -1)
    {
        qWarning() << "Trying to overwrite exist notification";
        return QRect();
    }
    active_notifications.append(notification);
    int number = active_notifications.count()-1;
    if (number == 0) {
        QRect geom = QApplication::desktop()->availableGeometry(QCursor::pos());
        geom = QRect(geom.bottomRight(),notification->geometry().size());
        geom.moveLeft(geom.x() - margin - notification->geometry().width());
        return geom;
    }
    else
	{
		QRect geom = active_notifications.at(number-1)->geometry();
		geom.setHeight (notification->geometry ().height ());
		return geom;
	}
}

void NotificationsManager::remove ( const QString& id )
{
    int number = getNumber(id);
    active_notifications.removeAt(number);
}

void NotificationsManager::updateGeometry()
{
    if (updatePosition) {
        QRect geom = QApplication::desktop()->availableGeometry(QCursor::pos());
        int y = geom.bottom();
        geom.moveLeft(geom.right() - margin - defaultSize.width());
        for (int i=0;i!=active_notifications.count();i++) {
            y -= margin + active_notifications.value(i)->geometry().height();
            geom.moveTop(y);
            geom.setSize(active_notifications.value(i)->geometry().size());
            active_notifications.value(i)->update(geom);
        }
    }
}

NotificationsManager* NotificationsManager::self()
{
    if ( !instance )
        instance = new NotificationsManager();
    return instance;
}

void NotificationsManager::loadSettings()
{
    animationDuration = 1000;
    defaultSize = QSize(350,70);
    margin = 10;
    themePath = ":/plugins/kinotify/resources/notification/commie";
    content = loadContent(themePath + "/tmp.html");
    updatePosition = true;
    animation = true;
    easingCurve = QEasingCurve::OutSine;
    widgetFlags = Qt::ToolTip | Qt::FramelessWindowHint;
	action1Trigger = Qt::LeftButton;
	action2Trigger = Qt::RightButton;
}

QString NotificationsManager::loadContent ( const QString& path )
{
    QFile content (path);
    QString output;
    if (content.open(QIODevice::ReadOnly))
	{
        output = content.readAll();
		QStringList elements;
		elements << "back";
		Q_FOREACH (QString elem, elements)
			output.replace (QString ("{%1}").arg (elem),
					NotificationWidget::MakeImage (QString (themePath + "/img/%1.png").arg (elem)));
        content.close();
    }
    return output;
}

