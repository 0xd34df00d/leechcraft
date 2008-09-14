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


#include <QApplication>
#include <plugininterface/proxy.h>

#include "chatter.h"
#include "fsirc.h"

Chatter::~Chatter ()
{
}

void Chatter::Init ()
{
    setWindowTitle ("Chatter");
    IsShown_ = false;
	ircClient = new fsirc();
	connect(ircClient, SIGNAL(gotLink(QString)), this, SIGNAL(gotEntity(QString)));
    setCentralWidget (ircClient);
}

QString Chatter::GetName () const
{
    return "Chatter";
}

QString Chatter::GetInfo () const
{
    return tr("A fucking simple irc client");
}

QString Chatter::GetStatusbarMessage () const
{
    return tr("what the fuck is status?");
}

IInfo& Chatter::SetID (IInfo::ID_t id)
{
    ID_ = id;
    return *this;
}

IInfo::ID_t Chatter::GetID () const
{
    return ID_;
}

QStringList Chatter::Provides () const
{
    return QStringList ("irc");
}

QStringList Chatter::Uses () const
{
    return QStringList ();
}

QStringList Chatter::Needs () const
{
    return QStringList ();
}

void Chatter::SetProvider (QObject *obj, const QString& feature)
{
    Providers_ [feature] = obj;
}

void Chatter::PushMainWindowExternals (const MainWindowExternals&)
{

}

void Chatter::Release ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (GetName ());
    settings.beginGroup ("geometry");
    settings.setValue ("pos", pos ());
    settings.endGroup ();
    settings.endGroup ();

	fsirc::finalizeIrcList ();
	delete ircClient;

	IrcLayer::finalizeServers ();
}

QIcon Chatter::GetIcon () const
{
    return QIcon (":/fsirc/data/icon.svg");
}

void Chatter::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Chatter::ShowWindow ()
{
    IsShown_ ? hide () : show ();
    IsShown_ = 1 - IsShown_;
}

void Chatter::ShowBalloonTip ()
{
}

void Chatter::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void Chatter::handleHidePlugins ()
{
	IsShown_ = false;
	hide ();
}

Q_EXPORT_PLUGIN2 (leechcraft_chatter, Chatter);

