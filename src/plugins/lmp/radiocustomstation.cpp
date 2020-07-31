/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiocustomstation.h"
#include <QtDebug>
#include "radiocustomstreams.h"

namespace LC
{
namespace LMP
{
	RadioCustomStation::RadioCustomStation (const QList<QUrl>& tracks, RadioCustomStreams *parent)
	: QObject (parent)
	, RCS_ (parent)
	, TrackList_ (tracks)
	{
	}

	QObject* RadioCustomStation::GetQObject ()
	{
		return this;
	}

	void RadioCustomStation::RequestNewStream ()
	{
	}

	QString RadioCustomStation::GetRadioName () const
	{
		return tr ("Bookmarks");
	}

	void RadioCustomStation::AddItem (const QUrl& url, const QString& name)
	{
		RCS_->Add (url, name);
	}

	void RadioCustomStation::RemoveItem (const QModelIndex& index)
	{
		RCS_->Remove (index);
	}
}
}
