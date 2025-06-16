/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "expansionstatemanager.h"
#include <QTimer>
#include <QTreeView>
#include <QtDebug>
#include "roles.h"
#include "sortfilterproxymodel.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth
{
	namespace
	{
		QString BuildPath (QModelIndex index)
		{
			if (!index.isValid ())
				return {};

			QStringList path { index.data ().toString () };
			while ((index = index.parent ()).isValid ())
				path.prepend (index.data ().toString ());

			return "CLTreeExpanded/" + path.join ('/');
		}

		void SetExpanded (const QModelIndex& idx, bool expanded)
		{
			const auto type = idx.data (CLREntryType).value<CLEntryType> ();
			if (type != CLETCategory)
				return;

			const auto& path = BuildPath (idx);
			if (path.isEmpty ())
				return;

			XmlSettingsManager::Instance ().setProperty (path.toUtf8 (), expanded);
		}
	}

	ExpansionStateManager::ExpansionStateManager (SortFilterProxyModel& model, QTreeView& view, QObject *parent)
	: QObject { parent }
	, Model_ { model }
	, View_ { view }
	{
		qRegisterMetaType<QPersistentModelIndex> ("QPersistentModelIndex");

		connect (&model,
				&QAbstractItemModel::rowsInserted,
				this,
				&ExpansionStateManager::HandleRowsInserted);
		QTimer::singleShot (0, Qt::VeryCoarseTimer, this, &ExpansionStateManager::ReexpandTree);

		connect (&model,
				&QAbstractItemModel::rowsRemoved,
				this,
				&ExpansionStateManager::ReexpandTree);
		connect (&model,
				&QAbstractItemModel::modelReset,
				this,
				&ExpansionStateManager::ReexpandTree);

		QTimer::singleShot (0, Qt::VeryCoarseTimer, this, [=, this] { View_.expandToDepth (0); });

		connect (&view,
				&QTreeView::collapsed,
				this,
				[] (const QModelIndex& idx) { SetExpanded (idx, false); });
		connect (&view,
				&QTreeView::expanded,
				this,
				[] (const QModelIndex& idx) { SetExpanded (idx, true); });

		connect (&model,
				&SortFilterProxyModel::mucMode,
				&view,
				&QTreeView::expandAll);
	}

	void ExpansionStateManager::SetMucMode (bool mucMode)
	{
		if (mucMode)
			SaveWholeModeExpansions ();

		Model_.SetMUCMode (mucMode);

		if (!mucMode)
			RestoreWholeModeExpansions( );
	}

	void ExpansionStateManager::SaveWholeModeExpansions ()
	{
		FstLevelExpands_.clear ();
		SndLevelExpands_.clear ();

		for (int i = 0, rc = Model_.rowCount (); i < rc; ++i)
		{
			const auto& accIdx = Model_.index (i, 0);
			const auto& name = accIdx.data ().toString ();
			FstLevelExpands_ [name] = View_.isExpanded (accIdx);

			auto& groups = SndLevelExpands_ [name];
			for (int j = 0, grc = Model_.rowCount (accIdx); j < grc; ++j)
			{
				const auto& grpIdx = Model_.index (j, 0, accIdx);
				groups [grpIdx.data ().toString ()] = View_.isExpanded (grpIdx);
			}
		}
	}

	void ExpansionStateManager::RestoreWholeModeExpansions ()
	{
		if (FstLevelExpands_.isEmpty () || SndLevelExpands_.isEmpty ())
			return;

		for (int i = 0, rc = Model_.rowCount (); i < rc; ++i)
		{
			const auto& accIdx = Model_.index (i, 0);
			const auto& name = accIdx.data ().toString ();
			if (!FstLevelExpands_.contains (name))
				continue;

			View_.setExpanded (accIdx, FstLevelExpands_ [name]);

			const auto& groups = SndLevelExpands_ [name];
			for (int j = 0, grc = Model_.rowCount (accIdx); j < grc; ++j)
			{
				const auto& grpIdx = Model_.index (j, 0, accIdx);
				View_.setExpanded (grpIdx, groups [grpIdx.data ().toString ()]);
			}
		}

		FstLevelExpands_.clear ();
		SndLevelExpands_.clear ();
	}

	void ExpansionStateManager::ReexpandTree ()
	{
		if (const auto rc = Model_.rowCount ())
			HandleRowsInserted ({}, 0, rc - 1);
	}

	void ExpansionStateManager::HandleRowsInserted (const QModelIndex& parent, int begin, int end)
	{
		for (int i = begin; i <= end; ++i)
		{
			const auto& index = Model_.index (i, 0, parent);

			const auto type = index.data (CLREntryType).value<CLEntryType> ();
			if (type == CLETCategory)
			{
				const auto& path = BuildPath (index);

				const bool expanded = Model_.IsMUCMode () ||
						XmlSettingsManager::Instance ().Property (path.toStdString (), true).toBool ();
				if (expanded)
					ExpandLater (index);
			}
			else if (type == CLETAccount)
				ExpandLater (index);

			if (const auto rc = Model_.rowCount (index))
				HandleRowsInserted (index, 0, rc - 1);
		}
	}

	void ExpansionStateManager::ExpandLater (const QModelIndex& idx)
	{
		QPersistentModelIndex pIdx { idx };
		QTimer::singleShot (0, Qt::VeryCoarseTimer, this, [=, this] { ExpandIndex (pIdx); });
	}

	void ExpansionStateManager::ExpandIndex (const QPersistentModelIndex& pIdx)
	{
		if (pIdx.isValid ())
			View_.expand (pIdx);
	}
}
