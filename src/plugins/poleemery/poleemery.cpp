/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "poleemery.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/prelude.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "operationstab.h"
#include "accountstab.h"
#include "graphstab.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "poleemerysettings.xml");

		XSD_->SetDataSource ("CurrenciesView",
				Core::Instance ().GetCurrenciesManager ()->GetSettingsModel ());

		Core::Instance ().SetCoreProxy (proxy);

		TabClasses_.append ({
				{
					GetUniqueID () + "/Operations",
					tr ("Finances operations"),
					tr ("All operations on personal finances."),
					QIcon (),
					2,
					TFOpenableByRequest
				},
				[this] (const TabClassInfo& tc) { MakeTab<OperationsTab> (tc); }
			});
		TabClasses_.append ({
				{
					GetUniqueID () + "/Accounts",
					tr ("Finances accounts"),
					tr ("Finances accounts management tab."),
					QIcon (),
					1,
					TFOpenableByRequest
				},
				[this] (const TabClassInfo& tc) { MakeTab<AccountsTab> (tc); }
			});
		TabClasses_.append ({
				{
					GetUniqueID () + "/Graphs",
					tr ("Spending graphs"),
					tr ("Tab with various graphs helping to analyze spendings."),
					QIcon (),
					1,
					TFOpenableByRequest
				},
				[this] (const TabClassInfo& tc) { MakeTab<GraphsTab> (tc); }
			});
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poleemery";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Poleemery";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("The personal finances manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return Util::Map (TabClasses_, Util::Fst);
	}

	void Plugin::TabOpenRequested (const QByteArray& tc)
	{
		const auto pos = std::find_if (TabClasses_.begin (), TabClasses_.end (),
				[&tc] (const auto& pair) { return pair.first.TabClass_ == tc; });
		if (pos == TabClasses_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
			return;
		}

		pos->second (pos->first);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	template<typename T>
	void Plugin::MakeTab (const TabClassInfo& tc)
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tc.VisibleName_, new T { tc, this });
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_poleemery, LC::Poleemery::Plugin);
