/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "checker.h"
#include <algorithm>
#include <cmath>
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilocalcollection.h>
#include "checkmodel.h"

namespace LC::LMP::BrainSlugz
{
	Checker::Checker (CheckModel *model,
			const QList<Media::ReleaseInfo::Type>& types,
			QObject *parent)
	: QObject { parent }
	, Model_ { model }
	, Provider_ { GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::IDiscographyProvider*> ().value (0) }
	, Types_ { types }
	, Artists_ { Model_->GetSelectedArtists () }
	{
		if (!Provider_)
		{
			qWarning () << Q_FUNC_INFO
					<< "no providers :(";
			deleteLater ();
			return;
		}

		RotateQueue ();
	}

	int Checker::GetRemainingCount () const
	{
		return Artists_.size ();
	}

	void Checker::HandleReady ()
	{
		emit finished ();
	}

	namespace
	{
		void CleanupPunctuation (QString& name)
		{
			for (auto c : { '(', ')', ',', '.', ':' })
				name.remove (c);
			for (const auto& str : { QStringLiteral ("—") })
				name.remove (str);
			name = name.trimmed ().simplified ();
		}

		void CleanupAlbumName (QString& name)
		{
			name.remove ("EP"_ql);
			name.remove (" the "_ql, Qt::CaseInsensitive);
			if (name.startsWith ("the "_ql, Qt::CaseInsensitive))
				name = name.mid (4);
			name = name.trimmed ().simplified ();
		}

		const int MaxYearDiff = 4;

		bool IsSameRelease (const Collection::Album_ptr& albumPtr, const Media::ReleaseInfo& release)
		{
			auto name1 = albumPtr->Name_.toLower ();
			auto name2 = release.Name_.toLower ();

			CleanupPunctuation (name1);
			CleanupPunctuation (name2);

			if (name1 == name2)
				return true;

			CleanupAlbumName (name1);
			CleanupAlbumName (name2);

			if (albumPtr->Year_ == release.Year_ &&
					(name1.contains (name2) || name2.contains (name1)))
				return true;

			return std::abs (static_cast<double> (albumPtr->Year_ - release.Year_)) <= MaxYearDiff &&
					name1 == name2;
		}
	}

	void Checker::HandleDiscoReady (const Collection::Artist& artist, QList<Media::ReleaseInfo> releases)
	{
		releases.erase (std::remove_if (releases.begin (), releases.end (),
					[this] (const Media::ReleaseInfo& info)
					{
						return !Types_.contains (info.Type_);
					}),
				releases.end ());

		bool foundSome = false;

		for (const auto& albumPtr : artist.Albums_)
		{
			const auto pos = std::find_if (releases.begin (), releases.end (), std::bind_front (IsSameRelease, albumPtr));
			if (pos == releases.end ())
				continue;

			releases.erase (pos);
			foundSome = true;
		}

		if (!foundSome)
		{
			qWarning () << Q_FUNC_INFO
					<< "we probably found something different for"
					<< artist.Name_;
			for (const auto& release : releases)
				qWarning () << "\t" << release.Year_ << release.Name_;
			Model_->MarkNoNews (artist);
		}
		else if (releases.isEmpty ())
			Model_->MarkNoNews (artist);
		else
			Model_->SetMissingReleases (releases, artist);
	}

	void Checker::RotateQueue ()
	{
		emit progress (Artists_.size ());

		if (Artists_.isEmpty ())
		{
			HandleReady ();
			return;
		}

		auto artist = Artists_.takeFirst ();
		Util::Sequence (this, Provider_->GetDiscography (artist.Name_, {})) >>
				Util::Visitor
				{
					[=, this] (const QString& error)
					{
						qWarning () << Q_FUNC_INFO
								<< artist.Name_
								<< error;
						Model_->MarkNoNews (artist);
					},
					[=, this] (const auto& result) { HandleDiscoReady (artist, result); }
				}.Finally ([this] { RotateQueue (); });
	}
}
