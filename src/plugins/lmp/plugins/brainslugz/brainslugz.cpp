/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "brainslugz.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "checker.h"
#include "checktab.h"
#include "progressmodelmanager.h"

namespace LC::LMP::BrainSlugz
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		ProgressModelManager_ = new ProgressModelManager { this };

		CheckTC_ = TabClassInfo
		{
			GetUniqueID () + ".CheckTab",
			GetName (),
			GetInfo (),
			GetIcon (),
			0,
			TFOpenableByRequest | TFSingle
		};
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.BrainSlugz";
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("LMP BrainSlugz");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Check if your collection misses some albums or EPs!");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { CheckTC_ };
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.LMP.General" };
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return ProgressModelManager_->GetModel ();
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LmpProxy_ = proxy;
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc != CheckTC_.TabClass_)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
			return;
		}

		if (!OpenedTab_)
		{
			OpenedTab_ = new CheckTab { LmpProxy_, CheckTC_, this };
			connect (OpenedTab_,
					&CheckTab::checkStarted,
					ProgressModelManager_,
					&ProgressModelManager::AddChecker);
		}
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (GetName (), OpenedTab_);
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_brainslugz, LC::LMP::BrainSlugz::Plugin)
