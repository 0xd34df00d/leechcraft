/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <util/models/mergemodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihaverecoverabletabs.h>

class QSortFilterProxyModel;
class QTreeView;
class QToolBar;

namespace LC
{
namespace Summary
{
	class SummaryWidget;
	class TreeViewReemitter;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		/** Default merge model for the Downloads category with
		 * all the downloaders and such stuff.
		 */
		std::shared_ptr<Util::MergeModel> MergeModel_;
		QPointer<SummaryWidget> Current_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		void SecondInit ();

		QTreeView* GetCurrentView () const;

		/** Returns true if both indexes belong to the same model. If
			* both indexes are invalid, true is returned.
			*
			* The passed indexes shouldn't be mapped to source from filter
			* model or merge model, Core will do it itself.
			*
			* @param[in] i1 The first index.
			* @param[in] i2 The second index.
			* @return Whether the indexes belong to the same model.
			*/
		bool SameModel (const QModelIndex& i1, const QModelIndex& i2) const;

		/** Returns controls for the model with a given index. The
			* return value can't be NULL.
			*
			* The passed index shouldn't be mapped to source from filter
			* model, Core will do it itself.
			*
			* @param[in] index Unmapped index for which the widget should
			* be returned.
			* @return Toolbar with controls.
			*
			* @sa GetAdditionalInfo
			*/
		QToolBar* GetControls (const QModelIndex& index) const;

		/** Returns additional info for the model with a given index, or
			* NULL if the model doesn't provide it.
			*
			* The passed index shouldn't be mapped to source from filter
			* model, Core will do it itself.
			*
			* @param[in] index Unmapped index for which the widget should
			* be returned.
			* @return Widget with additional info/controls.
			*
			* @sa GetControls
			*/
		QWidget* GetAdditionalInfo (const QModelIndex& index) const;

		/** Creates a new model for the given request and returns a
			* pointer to it. Ownership is transferred to the caller.
			*
			* For example, this is used in the Summary.
			*/
		QSortFilterProxyModel* GetTasksModel () const;

		/** Returns list of tags for a given row using given model. It's
			* assumed that the passed model is actually a MergeModel.
			*
			* @param[in] row The row in the merge model for which the tags
			* should be retrieved.
			* @param[in] model The MergeModel which contains the row.
			* @return Tags for the row.
			*/
		QStringList GetTagsForIndex (int row, QAbstractItemModel *model) const;

		/** Maps totally unmapped index to the plugin's source model
			* through merge model and filter model. If the index doesn't
			* belong this plugin, returns an invalid QModelIndex().
			*
			* @param[in] index The original unmapped index.
			* @return Mapped index from the plugin's model.
			*
			* @exception std::runtime_error Throws if the required model
			* could not be found.
			*/
		QModelIndex MapToSourceRecursively (QModelIndex index) const;

		void RecoverTabs (const QList<TabRecoverInfo>&);
	public slots:
		void handleNewTabRequested ();
	private:
		template<typename F>
		SummaryWidget* CreateSummaryWidget (F&& f);
	private slots:
		void handleCurrentTabChanged (int);
		void handleWindow (int);
		void handlePluginInjected (QObject*);
	signals:
		void currentViewChanged (QTreeView*);
	};
}
}
