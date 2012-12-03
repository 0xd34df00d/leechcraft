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

#include "monocle.h"
#include <QIcon>
#include <qurl.h>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "documenttab.h"
#include "xmlsettingsmanager.h"
#include "defaultbackendmanager.h"

namespace LeechCraft
{
namespace Monocle
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("monocle");

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "monoclesettings.xml");

		Core::Instance ().SetProxy (proxy);

		XSD_->SetDataSource ("DefaultBackends",
				Core::Instance ().GetDefaultBackendManager ()->GetModel ());

		DocTabInfo_ =
		{
			GetUniqueID () + "_Document",
			"Monocle",
			GetInfo (),
			GetIcon (),
			55,
			TFOpenableByRequest | TFSuggestOpening
		};
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Modular document viewer for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/monocle/resources/images/monocle.svg");
		return icon;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (!(e.Parameters_ & FromUserInitiated))
			return EntityTestHandleResult ();

		if (!e.Entity_.canConvert<QUrl> ())
			return EntityTestHandleResult ();

		const auto& url = e.Entity_.toUrl ();
		if (url.scheme () != "file")
			return EntityTestHandleResult ();

		const auto& local = url.toLocalFile ();
		if (!QFile::exists (local))
			return EntityTestHandleResult ();

		return Core::Instance ().CanLoadDocument (local) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		auto tab = new DocumentTab (DocTabInfo_, this);
		tab->SetDoc (e.Entity_.toUrl ().toLocalFile ());
		EmitTab (tab);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { DocTabInfo_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == DocTabInfo_.TabClass_)
			EmitTab (new DocumentTab (DocTabInfo_, this));
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< id;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	void Plugin::AddPlugin (QObject *pluginObj)
	{
		Core::Instance ().AddPlugin (pluginObj);
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		Q_FOREACH (const auto& info, infos)
		{
			auto tab = new DocumentTab (DocTabInfo_, this);
			Q_FOREACH (const auto& pair, info.DynProperties_)
				tab->setProperty (pair.first, pair.second);

			EmitTab (tab);

			tab->RecoverState (info.Data_);
		}
	}

	void Plugin::EmitTab (DocumentTab *tab)
	{
		emit addNewTab (DocTabInfo_.VisibleName_, tab);
		emit changeTabIcon (tab, DocTabInfo_.Icon_);
		emit raiseTab (tab);

		connect (tab,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (tab,
				SIGNAL (changeTabName (QWidget*, QString)),
				this,
				SIGNAL (changeTabName (QWidget*, QString)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle, LeechCraft::Monocle::Plugin);

