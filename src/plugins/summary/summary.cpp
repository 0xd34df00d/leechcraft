/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summary.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "summarywidget.h"

namespace LC
{
namespace Summary
{
	void Summary::Init (ICoreProxy_ptr proxy)
	{
		SummaryWidget::SetParentMultiTabs (this);

		Core::Instance ().SetProxy (proxy);

		TabClassInfo tabClass =
		{
			"Summary",
			tr ("Summary"),
			GetInfo (),
			GetIcon (),
			50,
			TFOpenableByRequest | TFByDefault | TFSuggestOpening
		};
		TabClasses_ << tabClass;
	}

	void Summary::SecondInit ()
	{
		Core::Instance ().SecondInit ();
	}

	void Summary::Release ()
	{
		Core::Instance ().Release ();
	}

	QByteArray Summary::GetUniqueID () const
	{
		return "org.LeechCraft.Summary";
	}

	QString Summary::GetName () const
	{
		return "Summary";
	}

	QString Summary::GetInfo () const
	{
		return tr ("Summary of downloads and recent events");
	}

	QIcon Summary::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Summary::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Summary::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Summary")
			Core::Instance ().handleNewTabRequested ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	QModelIndex Summary::MapToSource (const QModelIndex& index) const
	{
		return Core::Instance ().MapToSourceRecursively (index);
	}

	void Summary::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		Core::Instance ().RecoverTabs (infos);
	}

	bool Summary::HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const
	{
		return true;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_summary, LC::Summary::Summary);
