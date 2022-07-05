/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ppl.h"
#include <QIcon>
#include <QAction>
#include <QFileDialog>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/lmp/ilmpproxy.h>
#include "loghandler.h"

namespace LC::LMP::PPL
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("lmp_ppl");

		ActionSync_ = new QAction { tr ("Sync scrobbling log"), this };
		connect (ActionSync_,
				&QAction::triggered,
				this,
				[this]
				{
					QFileDialog dia
					{
						nullptr,
						tr ("Select .scrobbler.log"),
						QDir::homePath (),
						tr ("Scrobbler log (*.scrobbler.log)")
					};
					dia.setFilter (QDir::AllEntries | QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
					dia.setAcceptMode (QFileDialog::AcceptOpen);
					if (dia.exec () != QDialog::Accepted)
						return;

					const auto& path = dia.selectedFiles ().value (0);
					if (path.isEmpty ())
						return;

					new LogHandler { path, LMPProxy_->GetLocalCollection (), Proxy_->GetPluginsManager (), this };
				});
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.PPL";
	}

	QString Plugin::GetName () const
	{
		return "LMP PPL";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Synchronizes the logs of songs played on portable devices with LMP and scrobbling services like Last.FM.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.LMP.General" };
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return
		{
			{
				"LMP",
				{ ActionSync_ }
			}
		};
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_ppl, LC::LMP::PPL::Plugin);
