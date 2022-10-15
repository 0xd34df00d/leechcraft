/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <QCache>
#include <QPointer>
#include <QSet>
#include <QImage>
#include <interfaces/core/ihookproxy.h>

class QStandardItem;

namespace LC
{
namespace Azoth
{
	class IProxyObject;
	class ICLEntry;
	struct EntryStatus;
	class AvatarsManager;

	class CLTooltipManager : public QObject
	{
		Q_OBJECT

		AvatarsManager * const AvatarsManager_;

		typedef QHash<ICLEntry*, QList<QStandardItem*>> Entry2Items_t;
		Entry2Items_t& Entry2Items_;

		QCache<ICLEntry*, QString> Avatar2TooltipSrcCache_;

		QSet<ICLEntry*> DirtyTooltips_;
	public:
		CLTooltipManager (AvatarsManager*, Entry2Items_t&);

		void AddEntry (ICLEntry*);
		void RemoveEntry (ICLEntry*);

		QString MakeTooltipString (ICLEntry*);

		void RebuildTooltip (ICLEntry *entry);
	private:
		QString MakeTooltipString (ICLEntry*, QString);
	private slots:
		void remakeTooltipForSender ();
		void handleAvatarChanged (QObject*);

		void handleCacheSizeChanged ();
		void handleAvatarsSizeChanged ();
	signals:
		void hookTooltipBeforeVariants (LC::IHookProxy_ptr proxy,
				QObject *entry) const;

		void rebuiltTooltip ();
	};
}
}
