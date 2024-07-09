/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "defaultbackendmanager.h"
#include <numeric>
#include <QStandardItemModel>
#include <QSettings>
#include <QApplication>
#include <util/sll/prelude.h>
#include <util/sll/scopeguards.h>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "choosebackenddialog.h"

namespace LC
{
namespace Monocle
{
	DefaultBackendManager::DefaultBackendManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Backends"), tr ("Choice") });
	}

	void DefaultBackendManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		for (const auto& key : settings.childKeys ())
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
		auto ids = Util::Map (loaders, [] (auto backend) { return qobject_cast<IInfo*> (backend)->GetUniqueID (); });
		std::sort (ids.begin (), ids.end ());
		const auto& key = std::accumulate (ids.begin (), ids.end (), QByteArray (),
				[] (const QByteArray& left, const QByteArray& right) { return left + '|' + right; });

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Monocle");
		const auto guard = Util::BeginGroup (settings, "BackendChoices");

		if (ids.contains (settings.value (key).toByteArray ()))
		{
			const auto& id = settings.value (key).toByteArray ();
			const auto pos = std::find_if (loaders.begin (), loaders.end (),
					[&id] (auto backend) { return qobject_cast<IInfo*> (backend)->GetUniqueID () == id; });
			return pos == loaders.end () ? nullptr : *pos;
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
		auto set = key.split ('|');
		set.removeAll ({});

		auto pm = GetProxyHolder ()->GetPluginsManager ();
		auto getName = [pm] (const QByteArray& id)
		{
			auto plugin = pm->GetPluginByID (id);
			return plugin ? qobject_cast<IInfo*> (plugin)->GetName () : QString ();
		};
		const auto& names = Util::Map (set, getName);

		QList<QStandardItem*> row;
		row << new QStandardItem (names.join ("; "));
		row << new QStandardItem (getName (choice));
		Model_->appendRow (row);

		row.first ()->setData (key, Roles::KeyID);
	}

	void DefaultBackendManager::removeRequested (const QString&, const QModelIndexList& indices)
	{
		QList<QPersistentModelIndex> pidxs;
		QSettings settings (QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Monocle");
		settings.beginGroup ("BackendChoices");
		for (const auto& idx : indices)
		{
			settings.remove (idx.sibling (idx.row (), 0).data (Roles::KeyID).toByteArray ());
			pidxs << idx;
		}
		settings.endGroup ();

		for (const auto& pidx : pidxs)
			Model_->removeRow (pidx.row ());
	}
}
}
