/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_SUMMARY_CORE_H
#define PLUGINS_SUMMARY_CORE_H
#include <QObject>
#include "interfaces/iinfo.h"
#include "plugininterface/mergemodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			class SummaryWidget;
			class TreeViewReemitter;

			class Core : public QObject
			{
				Q_OBJECT

				ICoreProxy_ptr Proxy_;

				boost::shared_ptr<Util::MergeModel> MergeModel_;
				SummaryWidget *Default_;
				SummaryWidget *Current_;
				QList<SummaryWidget*> Others_;
				TreeViewReemitter *Reemitter_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				void SecondInit ();

				TreeViewReemitter* GetTreeViewReemitter () const;
				SummaryWidget* GetDefaultTab () const;
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
				QAbstractItemModel* GetTasksModel (const QString& request) const;

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
			private:
				void MadeCurrent (SummaryWidget*);
				SummaryWidget* CreateSummaryWidget ();
			private slots:
				void handleCurrentTabChanged (int);
				void handleNewTabRequested ();
				void handleNeedToClose ();
				void handleFilterUpdated ();
			signals:
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
				void currentViewChanged (QTreeView*);
			};
		};
	};
};

#endif

