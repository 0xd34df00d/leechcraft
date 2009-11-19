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

#include "nufella.h"
#include <QIcon>
#include <plugininterface/util.h>

void Nufella::Init (ICoreProxy_ptr)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("nufella"));
}

void Nufella::SecondInit ()
{
}

void Nufella::Release ()
{
}

QString Nufella::GetName () const
{
	return "Nufella";
}

QString Nufella::GetInfo () const
{
	return tr ("Just another Gnutella implementation");
}

QStringList Nufella::Provides () const
{
	return QStringList ("gnutella");
}

QStringList Nufella::Needs () const
{
	return QStringList ();
}

QStringList Nufella::Uses () const
{
	return QStringList ();
}

void Nufella::SetProvider (QObject*, const QString&)
{
}

QIcon Nufella::GetIcon () const
{
	return QIcon ();
}

qint64 Nufella::GetDownloadSpeed () const
{
	return 0;
}

qint64 Nufella::GetUploadSpeed () const
{
	return 0;
}

void Nufella::StartAll ()
{
}

void Nufella::StopAll ()
{
}

bool Nufella::CouldDownload (const LeechCraft::DownloadEntity& entity) const
{
	return false;
}

int Nufella::AddJob (LeechCraft::DownloadEntity)
{
	return -1;
}

QAbstractItemModel* Nufella::GetRepresentation () const
{
	return 0;
}

QWidget* Nufella::GetControls () const
{
	return 0;
}

QWidget* Nufella::GetAdditionalInfo () const
{
	return 0;
}

Q_EXPORT_PLUGIN2 (leechcraft_nufella, Nufella);

