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
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "summarywidget.h"

namespace LC::Summary
{
	void Summary::Init (ICoreProxy_ptr)
	{
	}

	void Summary::SecondInit ()
	{
	}

	void Summary::Release ()
	{
		if (Current_)
		{
			delete Current_;
			Current_ = nullptr;
		}
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
		return SummaryWidget::GetStaticTabClassInfo ().Description_;
	}

	QIcon Summary::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	TabClasses_t Summary::GetTabClasses () const
	{
		return { SummaryWidget::GetStaticTabClassInfo () };
	}

	void Summary::TabOpenRequested (const QByteArray&)
	{
		if (Current_)
			emit Current_->raiseTab ();
		else
		{
			Current_ = new SummaryWidget { *this };
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Summary"), Current_);
		}
	}

	void Summary::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		if (infos.isEmpty () || Current_)
			return;

		const auto& info = infos.first ();

		Current_ = new SummaryWidget { *this };
		for (const auto& [name, value] : info.DynProperties_)
			Current_->setProperty (name, value);

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Summary"), Current_);
	}

	bool Summary::HasSimilarTab (const QByteArray&, const QList<QByteArray>&) const
	{
		return true;
	}
}

LC_EXPORT_PLUGIN (leechcraft_summary, LC::Summary::Summary);
