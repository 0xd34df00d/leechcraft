#ifndef CHATTER_H
#define CHATTER_H
/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <interfaces/interfaces.h>
#include <QMainWindow>

class fsirc;

class Chatter : public QMainWindow
                 , public IInfo
              , public IWindow
{
    Q_OBJECT;
	Q_INTERFACES (IInfo IWindow);

    ID_t ID_;
    bool IsShown_;
public:
    virtual void Init ();
    QMap<QString, QObject*> Providers_;
    virtual QString GetName () const;
    virtual QString GetInfo () const;
    virtual QString GetStatusbarMessage () const;
    virtual IInfo& SetID (ID_t);
    virtual ID_t GetID () const;
    virtual QStringList Provides () const;
    virtual QStringList Uses () const;
    virtual QStringList Needs () const;
    virtual void SetProvider (QObject*, const QString&);
    virtual void PushMainWindowExternals (const MainWindowExternals&);
    virtual void Release ();
    virtual QIcon GetIcon () const;
    virtual void SetParent (QWidget*);
    virtual void ShowWindow ();
    virtual void ShowBalloonTip ();
protected:
	virtual void closeEvent (QCloseEvent*);
	fsirc * ircClient;
public slots:
	void handleHidePlugins ();
signals:
	void gotEntity(QString);
};

#endif
