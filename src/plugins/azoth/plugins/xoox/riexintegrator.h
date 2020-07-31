/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QList>

namespace LC::Azoth
{
	struct RIEXItem;
}

namespace LC::Azoth::Xoox
{
	class RIEXManager;
	class GlooxAccount;

	class RIEXIntegrator : public QObject
	{
		RIEXManager& Mgr_;
		GlooxAccount& Acc_;

		QHash<QString, QList<RIEXItem>> AwaitingRIEXItems_;
	public:
		RIEXIntegrator (RIEXManager&, GlooxAccount&, QObject* = nullptr);

		void SuggestItems (const QList<RIEXItem>&, QObject*, const QString&);
	private:
		void HandleRIEX (const QString&, const QList<RIEXItem>&, const QString&);
	};
}
