/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Azoth
{
	class CLTooltipManager;

	class CLModel : public QStandardItemModel
	{
		Q_OBJECT

		CLTooltipManager * const TooltipManager_;
	public:
		CLModel (CLTooltipManager*, QObject* = 0);

		QVariant data (const QModelIndex&, int) const;

		QStringList mimeTypes () const;
		QMimeData* mimeData (const QModelIndexList&) const;
		bool dropMimeData (const QMimeData*, Qt::DropAction,
				int, int, const QModelIndex&);
		Qt::DropActions supportedDropActions () const;
	private:
		void CheckRequestUpdateTooltip (const QModelIndex&, int) const;

		bool PerformHooks (const QMimeData*, int, const QModelIndex&);
		bool CheckHookDnDEntry2Entry (const QMimeData*, int, const QModelIndex&);

		bool TryInvite (const QMimeData*, int, const QModelIndex&);
		bool TryDropContact (const QMimeData*, int, const QModelIndex&);
		bool TryDropFile (const QMimeData*, const QModelIndex&);
	signals:
		void hookDnDEntry2Entry (LC::IHookProxy_ptr,
				QObject*, QObject*);
		void rebuiltTooltip ();
	};
}
}
