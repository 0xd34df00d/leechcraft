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
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <QMainWindow>

class fsirc;

class Chatter : public QMainWindow
              , public IInfo
              , public IEmbedTab
{
    Q_OBJECT;
	Q_INTERFACES (IInfo IEmbedTab);

    bool IsShown_;
public:
	virtual ~Chatter ();
    virtual void Init ();
    virtual QString GetName () const;
    virtual QString GetInfo () const;
    virtual QStringList Provides () const;
    virtual QStringList Uses () const;
    virtual QStringList Needs () const;
    virtual void SetProvider (QObject*, const QString&);
    virtual void Release ();
    virtual QIcon GetIcon () const;
	virtual QWidget* GetTabContents ();
	fsirc * ircClient;
signals:
	void gotEntity(QString);
	void bringToFront ();
};

#endif
