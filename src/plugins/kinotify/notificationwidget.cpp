#include "notificationwidget.h"
#include <QVBoxLayout>
#include "notificationsmanager.h"
#include <QMouseEvent>
#include <QDebug>
#include <QLabel>
#include <QBuffer>
#include <QPainter>
#include <QWebFrame>
#include <QTextDocument>

NotificationWidget::NotificationWidget ( const QString &content )
{
    setTheme ( content );
	setStyleSheet ("background: transparent");
    //init browser
    this->page ()->mainFrame ()->setScrollBarPolicy ( Qt::Horizontal, Qt::ScrollBarAlwaysOff );
    this->page ()->mainFrame ()->setScrollBarPolicy ( Qt::Vertical, Qt::ScrollBarAlwaysOff );
    //this->setFrameShape ( QFrame::NoFrame );
    this->setWindowFlags(NotificationsManager::self()->widgetFlags);
    
    //init transparent
	QPalette pal = palette();
	pal.setBrush(QPalette::Base, Qt::transparent);
	page()->setPalette(pal);
	setAttribute(Qt::WA_OpaquePaintEvent, false);
    this->setAttribute(Qt::WA_TranslucentBackground);

	resize (NotificationsManager::self()->defaultSize);
	setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Preferred);
	page ()->setPreferredContentsSize (size ());
}

QByteArray NotificationWidget::MakeImage (const QString& path)
{
	QBuffer iconBuffer;
	iconBuffer.open (QIODevice::ReadWrite);
	QPixmap pixmap (path);
	pixmap.save (&iconBuffer, "PNG");
	return QByteArray ("data:image/png;base64,") + iconBuffer.buffer ().toBase64 ();
}


QSize NotificationWidget::setData ( const QString& title, const QString& body, const QString& imagePath )
{
    QString data = content;
    data.replace ( "{title}", title );
    data.replace ( "{body}", Qt::escape (body) );
    data.replace ( "{imagepath}", MakeImage (imagePath));
    setHtml(data);
    int width = size ().width();
    int height = size ().height();
	QSize contents = page ()->mainFrame ()->contentsSize ();
	int cheight = contents.height ();
	if (cheight > height ||
			(cheight > 0 && cheight < height))
		height = cheight;

    return QSize(width,height);
}


void NotificationWidget::setTheme ( const QString& content )
{
    this->content = content;
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
