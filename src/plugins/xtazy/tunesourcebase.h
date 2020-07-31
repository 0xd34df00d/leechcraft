/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantMap>

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace Xtazy
{
	class TuneSourceBase : public QObject
	{
		Q_OBJECT

		const QByteArray SourceName_;
	public:
		TuneSourceBase (const QByteArray&, QObject* = nullptr);

		const QByteArray& GetSourceName () const;
	protected:
		void EmitChange (const Media::AudioInfo&);

		Media::AudioInfo FromMPRISMap (const QVariantMap&);
	signals:
		void tuneInfoChanged (const Media::AudioInfo&, TuneSourceBase*);
	};
}
}
