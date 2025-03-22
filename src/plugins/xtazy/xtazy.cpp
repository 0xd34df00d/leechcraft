/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xtazy.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sll/unreachable.h>
#include "xmlsettingsmanager.h"
#include "tunesourcebase.h"
#include "filesource.h"
#include "lcsource.h"

#ifdef HAVE_DBUS
#include "mprissource.h"
#endif

namespace LC
{
namespace Xtazy
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
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
					SIGNAL (tuneInfoChanged (Media::AudioInfo, TuneSourceBase*)),
					this,
					SLOT (publish (Media::AudioInfo, TuneSourceBase*)));
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

	bool Plugin::SupportsFeature (Feature feature) const
	{
		switch (feature)
		{
		case Feature::Backdating:
			return false;
		}

		Util::Unreachable ();
	}

	QString Plugin::GetServiceName () const
	{
		return GetName ();
	}

	void Plugin::NowPlaying (const Media::AudioInfo& audio)
	{
		LCSource_->NowPlaying (audio);
	}

	void Plugin::SendBackdated (const BackdatedTracks_t&)
	{
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

	Media::AudioInfo Plugin::GetCurrentSong () const
	{
		return Previous_;
	}

	void Plugin::publish (const Media::AudioInfo& info, TuneSourceBase *base)
	{
		if (info == Previous_)
			return;

		const auto& propName = "Enable" + base->GetSourceName () + "Source";
		if (!XmlSettingsManager::Instance ().property (propName).toBool ())
			return;

		Previous_ = info;
		emit currentSongChanged (info);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_xtazy, LC::Xtazy::Plugin);
