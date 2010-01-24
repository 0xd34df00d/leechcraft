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

#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QDesktopWidget>
#include <QTextBrowser>

class QTextBrowser;
class NotificationWidget : public QTextBrowser
{
    Q_OBJECT
public:
    NotificationWidget(const QString &styleSheet, const QString &content);
    QSize setData(const QString& title, const QString& body, const QString &imagePath); //size of textbrowser
    void setTheme(const QString &styleSheet, const QString &content);
    virtual void mouseReleaseEvent ( QMouseEvent* ev );
private:
    //QString style_sheet;
    QString content;
signals:
    void action1Activated();
    void action2Activated();
};

#endif // NOTIFICATIONWIDGET_H
