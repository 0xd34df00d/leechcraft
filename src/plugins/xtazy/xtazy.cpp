/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Georg Rudoy
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

#include "xtazy.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

#include "tunesourcebase.h"
#include "filesource.h"
#include "lcsource.h"

#ifdef HAVE_DBUS
#include "mprissource.h"
#endif

namespace LeechCraft
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"xtazysettings.xml");

		LCSource_ = new LCSource (this);

#ifdef HAVE_DBUS
		TuneSources_ << new MPRISSource (this);
#endif
		TuneSources_ << new FileSource (this);
		TuneSources_ << LCSource_;
	}

	void Plugin::SecondInit ()
	{
		for (auto base : TuneSources_)
			connect (base,
					SIGNAL (tuneInfoChanged (Media::AudioInfo)),
					this,
					SLOT (publish (Media::AudioInfo)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Xtazy";
	}

	void Plugin::Release ()
	{
		qDeleteAll (TuneSources_);
	}

	QString Plugin::GetName () const
	{
		return "Xtazy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Keeps track of the currently playing song and provides it to other plugins.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QString Plugin::GetServiceName () const
	{
		return GetName ();
	}

	void Plugin::NowPlaying (const Media::AudioInfo& audio)
	{
		LCSource_->NowPlaying (audio);
	}

	void Plugin::PlaybackStopped ()
	{
		LCSource_->Stopped ();
	}

	void Plugin::LoveCurrentTrack ()
	{
	}

	void Plugin::BanCurrentTrack ()
	{
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	Media::AudioInfo Plugin::GetCurrentSong () const
	{
		return Previous_;
	}

	void Plugin::publish (const Media::AudioInfo& info)
	{
		if (info == Previous_)
			return;

		Previous_ = info;
		emit currentSongChanged (info);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_xtazy, LeechCraft::Xtazy::Plugin);
