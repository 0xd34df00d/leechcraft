/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class IEntityManager;

namespace LC
{
namespace LMP
{
	class SourceObject;
	enum class SourceError;

	class SourceErrorHandler : public QObject
	{
		Q_OBJECT

		SourceObject * const Source_;
		IEntityManager * const IEM_;
	public:
		SourceErrorHandler (SourceObject*, IEntityManager*);
	private slots:
		void handleSourceError (const QString&, SourceError);
	signals:
		void nextTrack ();
	};
}
}
