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

#include "defaultbackendmanager.h"
#include <memory>
#include <numeric>
#include <QStandardItemModel>
#include <QSettings>
#include <QApplication>
#include <interfaces/iinfo.h>
#include <interfaces/core/ipluginsmanager.h>
#include "choosebackenddialog.h"
#include "core.h"

namespace LeechCraft
{
namespace Monocle
{
	DefaultBackendManager::DefaultBackendManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Backends")) << tr ("Choice"));
	}

	void DefaultBackendManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		Q_FOREACH (const auto& key, settings.childKeys ())
		{
			const auto& utf8key = key.toUtf8 ();
			AddToModel (utf8key, settings.value (utf8key).toByteArray ());
		}
		settings.endGroup ();
	}

	QAbstractItemModel* DefaultBackendManager::GetModel () const
	{
		return Model_;
	}

	QObject* DefaultBackendManager::GetBackend (const QList<QObject*>& loaders)
	{
		QList<QByteArray> ids;
		Q_FOREACH (auto backend, loaders)
			ids << qobject_cast<IInfo*> (backend)->GetUniqueID ();
		std::sort (ids.begin (), ids.end ());
		const auto& key = std::accumulate (ids.begin (), ids.end (), QByteArray (),
				[] (const QByteArray& left, const QByteArray& right)
					{ return left + '|' + right; });

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		std::shared_ptr<void> guard (static_cast<void*> (0),
				[&settings] (void*) { settings.endGroup (); });

		if (ids.contains (settings.value (key).toByteArray ()))
		{
			const auto& id = settings.value (key).toByteArray ();
			Q_FOREACH (auto backend, loaders)
				if (qobject_cast<IInfo*> (backend)->GetUniqueID () == id)
					return backend;
			return 0;
		}

		ChooseBackendDialog dia (loaders);
		if (dia.exec () != QDialog::Accepted)
			return 0;

		auto backend = dia.GetSelectedBackend ();
		if (dia.GetRememberChoice ())
		{
			const auto& selectedId = qobject_cast<IInfo*> (backend)->GetUniqueID ();
			settings.setValue (key, selectedId);
			AddToModel (key, selectedId);
		}

		return backend;
	}

	void DefaultBackendManager::AddToModel (const QByteArray& key, const QByteArray& choice)
	{
		QList<QByteArray> set = key.split ('|');
		set.removeAll (QByteArray ());

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		auto getName = [pm] (const QByteArray& id) -> QString
		{
			auto plugin = pm->GetPluginByID (id);
			return plugin ? qobject_cast<IInfo*> (plugin)->GetName () : QString ();
		};
		QStringList names;
		std::transform (set.begin (), set.end (), std::back_inserter (names), getName);

		QList<QStandardItem*> row;
		row << new QStandardItem (names.join ("; "));
		row << new QStandardItem (getName (choice));
		Model_->appendRow (row);

		row.first ()->setData (key, Roles::KeyID);
	}

	void DefaultBackendManager::removeRequested (const QString&, const QModelIndexList& indices)
	{
		QList<QPersistentModelIndex> pidxs;
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		Q_FOREACH (const auto& idx, indices)
		{
			settings.remove (idx.sibling (idx.row (), 0).data (Roles::KeyID).toByteArray ());
			pidxs << idx;
		}
		settings.endGroup ();

		Q_FOREACH (const auto& pidx, pidxs)
			Model_->removeRow (pidx.row ());
	}
}
}
