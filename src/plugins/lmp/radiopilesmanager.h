/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class IPluginsManager;

namespace Media
{
	class IAudioPile;
}

namespace LC
{
namespace LMP
{
	class RadioPilesManager : public QObject
	{
		QStandardItemModel * const PilesModel_;
	public:
		RadioPilesManager (const IPluginsManager*, QObject* = nullptr);

		QAbstractItemModel* GetModel () const;
	private:
		void FillModel (const IPluginsManager*);
		void HandlePile (QStandardItem*, Media::IAudioPile*);
	};
}
}
