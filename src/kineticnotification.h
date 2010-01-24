//   Modern notifications, which use new Qt Statemachine and Animation framework
//   (c) by Sidorov "Sauron" Aleksey, sauron@citadelspb.com, 2009
//   This library is free software; you can redistribute it and/or
//   modify it under the terms of the GNU Library General Public
//   License version 2 or later as published by the Free Software Foundation.
//
//   This library is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   Library General Public License for more details.
//
//   You should have received a copy of the GNU Library General Public License
//   along with this library; see the file COPYING.LIB.  If not, write to
//   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//   Boston, MA 02110-1301, USA.

#ifndef KINETICNOTIFICATION_H
#define KINETICNOTIFICATION_H

#include <QObject>
#include <QStateMachine>
#include <QRect>

class NotificationWidget;
class KineticNotification : public QObject
{
    Q_OBJECT
public:
    KineticNotification(const QString &id, uint timeOut = 0); //0 - persistant
    virtual ~KineticNotification();
    void setMessage(const QString &title, const QString &body = NULL, const QString &imagePath = NULL);
    void appendMessage(const QString &message);
    void setActionsText(const QString &action1,const QString &action2);
    //for short message you can use only title
    void send();
    void update(QRect geom);
    QString getId() const;
    QRect geometry() const; //Show state geometry
signals:
    void finished(const QString &id);
    void updated();
    void timeoutReached();
    void action1Activated();
    void action2Activated();
private:
    NotificationWidget *notification_widget;
    void updateGeometry(const QRect &newGeometry);
    QString title;
    QString body;
    QString id;
    QString image_path;
    uint timeOut;
    QRect show_geometry; //Don't use direct, change by UpdateGeometry!
    QStateMachine machine;
    QState *show_state;
    QState *hide_state;
    virtual void timerEvent ( QTimerEvent* );
};

#endif // KINETICNOTIFICATION_H
