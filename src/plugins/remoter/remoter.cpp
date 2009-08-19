/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "remoter.h"
#include <QTranslator>
#include <plugininterface/mergemodel.h>
#include <plugininterface/util.h>
#include "core.h"

using LeechCraft::Util::MergeModel;

void Remoter::Init ()
{
	LeechCraft::Util::InstallTranslator ("remoter");

    IsShown_ = false;
    Ui_.setupUi (this);
}

void Remoter::Release ()
{
    Core::Instance ().Release ();
}

QString Remoter::GetName () const
{
    return tr ("Remoter");
}

QString Remoter::GetInfo () const
{
    return tr ("Server providing remote access to other plugins."); 
}

QStringList Remoter::Provides () const
{
    return QStringList ("remoteaccess");
}

QStringList Remoter::Needs () const
{
    return QStringList ("*")
		<< "services::historyModel"
		<< "services::downloadersModel";
}

QStringList Remoter::Uses () const
{
    return QStringList ("remoteable");
}

void Remoter::SetProvider (QObject *provider, const QString& feature)
{
    Core::Instance ().AddObject (provider, feature);
}

QIcon Remoter::GetIcon () const
{
    return windowIcon ();
}

void Remoter::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Remoter::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void Remoter::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void Remoter::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

void Remoter::pushHistoryModel (MergeModel *model) const
{
	Core::Instance ().SetHistoryModel (model);
}

void Remoter::pushDownloadersModel (MergeModel *model) const
{
	Core::Instance ().SetDownloadersModel (model);
}

Q_EXPORT_PLUGIN2 (leechcraft_remoter, Remoter);

