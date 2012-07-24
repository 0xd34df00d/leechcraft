/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "mp3tunes.h"
#include <QIcon>

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.MP3Tunes";
	}

	QString Plugin::GetName () const
	{
		return "LMP MP3tunes";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for the MP3tunes.com service, including its playlist and locker facilities.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CloudStorage";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy*)
	{
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QString Plugin::GetCloudName () const
	{
		return "MP3tunes";
	}

	QIcon Plugin::GetCloudIcon () const
	{
		return QIcon ();
	}

	QStringList Plugin::GetSupportedFileFormats () const
	{
		return { "m4a", "mp3", "mp4", "ogg" };
	}

	void Plugin::Upload (const QString& localPath, const QString& account)
	{
	}

	QStringList Plugin::GetAccounts () const
	{
		return QStringList ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mp3tunes, LeechCraft::LMP::MP3Tunes::Plugin);
