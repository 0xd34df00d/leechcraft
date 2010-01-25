#include "notificationwidget.h"
#include <QVBoxLayout>
#include "notificationsmanager.h"
#include <QMouseEvent>
#include <QDebug>
#include <QLabel>
#include <QBuffer>
#include <QPainter>

NotificationWidget::NotificationWidget ( const QString &styleSheet, const QString &content )
{
    setTheme ( styleSheet,content );
    //init browser
    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
    this->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    this->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff);
    this->setFrameShape ( QFrame::NoFrame );
    this->setWindowFlags(NotificationsManager::self()->widgetFlags);
    //this->resize(NotificationsManager::self()->defaultSize);
    
    //init transparent
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_NoSystemBackground, false);
    this->ensurePolished(); // workaround Oxygen filling the background
    this->setAttribute(Qt::WA_StyledBackground, false);
}

QByteArray NotificationWidget::MakeImage (const QString& path)
{
	QBuffer iconBuffer;
	iconBuffer.open (QIODevice::ReadWrite);
	QPixmap pixmap (path);
	pixmap.save (&iconBuffer, "PNG");
	return iconBuffer.buffer ().toBase64 ();
}


QSize NotificationWidget::setData ( const QString& title, const QString& body, const QString& imagePath )
{
    QString data = content;
    data.replace ( "{title}", title );
    data.replace ( "{body}", body );
    data.replace ( "{imagepath}", MakeImage (imagePath));
    this->document()->setHtml(data);
    this->document()->setTextWidth(NotificationsManager::self()->defaultSize.width());    
    int width = NotificationsManager::self()->defaultSize.width();
    int height = this->document()->size().height();

    return QSize(width,height);
}


void NotificationWidget::setTheme ( const QString& styleSheet, const QString& content )
{
    this->content = content;
    this->setStyleSheet ( styleSheet );
}


void NotificationWidget::mouseReleaseEvent ( QMouseEvent* ev )
{
    if (ev->button() == NotificationsManager::self()->action1Trigger) {
        emit action1Activated();
    }
//    else if (ev->button() == NotificationsManager::self()->action2Trigger)
//        emit action2Activated();
    else
        return;
    disconnect(SIGNAL(action1Activated()));
    disconnect(SIGNAL(action2Activated()));
}
