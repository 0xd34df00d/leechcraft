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

#include "deadlyrics.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "sitessearcher.h"

namespace LeechCraft
{
namespace DeadLyrics
{
	void DeadLyRicS::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("deadlyrics");

		Proxy_ = proxy;

		Searchers_ << Searcher_ptr (new SitesSearcher (":/deadlyrics/resources/sites.xml", proxy));

		SettingsDialog_.reset (new Util::XmlSettingsDialog ());
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"deadlyricssettings.xml");
	}

	void DeadLyRicS::SecondInit ()
	{
	}

	void DeadLyRicS::Release ()
	{
		Searchers_.clear ();
	}

	QByteArray DeadLyRicS::GetUniqueID () const
	{
		return "org.LeechCraft.DeadLyrics";
	}

	QString DeadLyRicS::GetName () const
	{
		return "DeadLyRicS";
	}

	QString DeadLyRicS::GetInfo () const
	{
		return tr ("Lyrics Searcher");
	}

	QIcon DeadLyRicS::GetIcon () const
	{
		return QIcon (":/deadlyrics/resources/images/deadlyrics.svg");
	}

	Util::XmlSettingsDialog_ptr DeadLyRicS::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void DeadLyRicS::RequestLyrics (const Media::LyricsQuery& query, Media::QueryOptions)
	{
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_deadlyrics, LeechCraft::DeadLyrics::DeadLyRicS);
