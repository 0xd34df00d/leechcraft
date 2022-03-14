/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/collectiontypes.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/media/idiscographyprovider.h>

namespace Media
{
	class IDiscographyProvider;
}

namespace LC::LMP::BrainSlugz
{
	class CheckModel;

	class Checker : public QObject
	{
		Q_OBJECT

		CheckModel * const Model_;
		Media::IDiscographyProvider * const Provider_;

		const QList<Media::ReleaseInfo::Type> Types_;

		Collection::Artists_t Artists_;
	public:
		Checker (CheckModel*, const QList<Media::ReleaseInfo::Type>&,
				const ICoreProxy_ptr&, QObject* = nullptr);

		int GetRemainingCount () const;
	private:
		void HandleReady ();
		void HandleDiscoReady (const Collection::Artist&, QList<Media::ReleaseInfo>);

		void RotateQueue ();
	signals:
		void finished ();
		void progress (int remaining);
	};
}
