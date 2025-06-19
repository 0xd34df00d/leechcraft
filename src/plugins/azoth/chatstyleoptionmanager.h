/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "sourcetrackingmodel.h"

class QStandardItemModel;

namespace LC
{
namespace Util
{
	class MergeModel;
}

namespace Azoth
{
	class IChatStyleResourceSource;

	class ChatStyleOptionManager : public QObject
	{
		Q_OBJECT

		const QByteArray OptionName_;

		SourceTrackingModel<IChatStyleResourceSource> * const CoreStylesModel_;
		QStandardItemModel * const EmptyOptModel_;
		Util::MergeModel * const OptionsModel_;

		QStandardItemModel * const VariantModel_;
	public:
		ChatStyleOptionManager (const QByteArray& optionName, QObject* = 0);

		void AddEmptyVariant ();

		QAbstractItemModel* GetStyleModel () const;
		QAbstractItemModel* GetVariantModel () const;

		void AddChatStyleResourceSource (IChatStyleResourceSource*);

		void StyleSelected (const QString&);
	};
}
}
