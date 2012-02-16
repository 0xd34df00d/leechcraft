/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "summary.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include "core.h"
#include "summarywidget.h"

namespace LeechCraft
{
namespace Summary
{
	void Summary::Init (ICoreProxy_ptr proxy)
	{
		SummaryWidget::SetParentMultiTabs (this);

		Translator_.reset (Util::InstallTranslator ("summary"));

		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (addNewTab (const QString&, QWidget*)),
				this,
				SIGNAL (addNewTab (const QString&, QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (&Core::Instance (),
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
		connect (&Core::Instance (),
				SIGNAL (changeTooltip (QWidget*, QWidget*)),
				this,
				SIGNAL (changeTooltip (QWidget*, QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (statusBarChanged (QWidget*, const QString&)),
				this,
				SIGNAL (statusBarChanged (QWidget*, const QString&)));
		connect (&Core::Instance (),
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));

		TabClassInfo tabClass =
		{
			"Summary",
			tr ("Summary"),
			GetInfo (),
			GetIcon (),
			50,
			TabFeatures (TFOpenableByRequest | TFByDefault)
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
		Translator_.reset ();
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
		return QIcon (":/plugins/summary/resources/images/summary.svg");
	}

	QStringList Summary::Provides () const
	{
		return QStringList ();
	}

	QStringList Summary::Needs () const
	{
		return QStringList ();
	}

	QStringList Summary::Uses () const
	{
		return QStringList ();
	}

	void Summary::SetProvider (QObject*, const QString&)
	{
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

	EntityTestHandleResult Summary::CouldHandle (const LeechCraft::Entity& e) const
	{
		EntityTestHandleResult result;
		result.HandlePriority_ = Core::Instance ().CouldHandle (e) ? 1000 : 0;
		return result;
	}

	void Summary::Handle (LeechCraft::Entity e)
	{
		Core::Instance ().Handle (e);
	}

	QModelIndex Summary::MapToSource (const QModelIndex& index) const
	{
		return Core::Instance ().MapToSourceRecursively (index);
	}

	QTreeView* Summary::GetCurrentView () const
	{
		return Core::Instance ().GetCurrentView ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_summary, LeechCraft::Summary::Summary);
