/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QPointer>
#include <QAbstractProxyModel>
#include <QStringList>
#include <QStack>
#include "modelsconfig.h"
#include "modelitem.h"

namespace LC::Util
{
	/** Merges data from multiple source models into one resulting
	 * model and provides means to lookup models by row, get
	 * starting rows for a model etc.
	 *
	 * To add a new source model, one should use AddModel() as
	 * setSourceModel() throws an std::runtime_error exception.
	 *
	 * Currently it doesn't support hierarchical source models.
	 * Seems like it would never support it at least someone would
	 * try to implement it.
	 *
	 * @ingroup ModelUtil
	 */
	class UTIL_MODELS_API MergeModel : public QAbstractItemModel
	{
		Q_OBJECT

		mutable bool DefaultAcceptsRowImpl_ = false;
	protected:
		using models_t = QList<QPointer<QAbstractItemModel>>;
		models_t Models_;
	private:
		QStringList Headers_;

		ModelItem_ptr Root_;

		QStack<std::function<void ()>> RemovalRefreshers_;
	public:
		using iterator = models_t::iterator;
		using const_iterator = models_t::const_iterator;

		/** @brief Constructs the merge model.
		 *
		 * Sets the given \em headers and \em parent object.
		 *
		 * @param[in] headers The headers of the model.
		 * @param[in] parent The parent object of the model.
		 */
		explicit MergeModel (QStringList headers, QObject *parent = nullptr);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;

		/** @brief Returns the union of MIME types of the models.
		 *
		 * @return The union of all the MIME types.
		 */
		QStringList mimeTypes () const override;

		/** @brief Returns the MIME data for the given \em indices.
		 *
		 * This function queries the corresponding source model for
		 * each index of the \em indices, merging the URL list (if
		 * any) and using first obtained data of any other format.
		 *
		 * @param[in] indices The indices for which to return the
		 * MIME data.
		 * @return The MIME data.
		 */
		QMimeData* mimeData (const QModelIndexList& indices) const override;

		/** @brief Returns the model index in the MergeModel given the
		 * index from the source model.
		 *
		 * @param[in] index Source index.
		 * @return MergeModel's index.
		 */
		virtual QModelIndex mapFromSource (const QModelIndex& index) const;

		/** @brief Returns the source model index corresponding to the
		 * given index from the sorting filter model.
		 *
		 * @param[in] index MergeModel's index.
		 * @return Source index.
		 */
		virtual QModelIndex mapToSource (const QModelIndex& index) const;

		/** You shouldn't use this function because its semantics in
		 * the context of multiple source models aren't clearly
		 * defined. Calling this function results in
		 * std::runtime_error.
		 *
		 * @exception std::runtime_error No matter what, you'd get
		 * it.
		 */
		virtual void setSourceModel (QAbstractItemModel*);

		/** @brief Sets the new headers for this model.
		 *
		 * @param[in] headers The new headers.
		 */
		void SetHeaders (const QStringList& headers);

		/** @brief Adds a model to the list of source models.
		 *
		 * The newly added model is appended to the end.
		 *
		 * If the model already exists in the list, it is added
		 * again, and bad things would happen, as all the signals and
		 * slots would be connected and called twice. So it's your
		 * duty to ensure that you don't add the same model more than
		 * once.
		 *
		 * @param[in] model The model to append to the list.
		 */
		void AddModel (QAbstractItemModel *model);

		/** @brief Removes a model from the list of source models.
		 *
		 * If there is no such model, this function does nothing.
		 *
		 * @param[in] model The model to remove from the list.
		 */
		void RemoveModel (QAbstractItemModel *model);

		/** @brief Returns the number of child models in the merger.
		 *
		 * @return The number of child models.
		 */
		size_t Size () const;

		/** @brief Returns a const_iterator corresponding to the
		 * passed model, or one-past-end if no such model is found.
		 *
		 * @param[in] model The model to find.
		 * @return The iterator.
		 */
		const_iterator FindModel (const QAbstractItemModel *model) const;

		/** @brief This is an overloaded function provided for convenience.
		 * Non-const and returns a non-const iterator.
		 *
		 * @param[in] model The model to find.
		 * @return The iterator.
		 */
		iterator FindModel (const QAbstractItemModel *model);

		/** @brief Finds starting row for the model pointed by \em it.
		 *
		 * Returns the row in the resulting MergeModel from which do
		 * begin rows which belong to the model corresponding to the
		 * given const_iterator.
		 *
		 * @param[in] it The iterator corresponding to the model.
		 * @return The starting row.
		 */
		int GetStartingRow (const_iterator it) const;

		/** @brief Finds starting row for the model pointed by \em it.
		 *
		 * Returns the row in the resulting MergeModel from which do
		 * begin rows which belong to the model corresponding to the
		 * given const_iterator.
		 *
		 * @param[in] it The iterator corresponding to the model.
		 * @return The starting row.
		 */
		int GetStartingRow (iterator it);

		/** @brief Returns the model for the given \em row.
		 *
		 * Returns the model that corresponds to the given row. If
		 * there is no such model, throws std::runtime_error. If
		 * starting is not null, it also calculates and returns the
		 * starting row for the returned model. This allows one to avoid
		 * calling GetStartingRow() after this function and thus
		 * speed things up.
		 *
		 * @param[in] row The row that should be identified.
		 * @param[in,out] starting The pointer to variable that will
		 * store the starting row, if not null.
		 * @return Iterator associated with the model.
		 *
		 * @exception std::runtime_error Throws if there is no model
		 * for such row.
		 */
		const_iterator GetModelForRow (int row, int *starting = nullptr) const;

		/** @brief This is an overloaded function provided for
		 * convenience.
		 *
		 * @param[in] row The row that should be identified.
		 * @param[in,out] starting The pointer to variable that will
		 * store the starting row, if not null.
		 * @return Iterator associated with the model.
		 *
		 * @exception std::runtime_error Throws if there is no model
		 * for such row.
		 */
		iterator GetModelForRow (int row, int *starting = nullptr);

		/** @brief Returns all models intalled into this one.
		 *
		 * Only those models that are not null (and, thus, haven't
		 * been destroyed) are returned in the list. This list is
		 * guaranteed to contain only valid objects.
		 *
		 * @return The list of models.
		 */
		QList<QAbstractItemModel*> GetAllModels () const;
	public Q_SLOTS:
		virtual void handleColumnsAboutToBeInserted (const QModelIndex&, int, int);
		virtual void handleColumnsAboutToBeRemoved (const QModelIndex&, int, int);
		virtual void handleColumnsInserted (const QModelIndex&, int, int);
		virtual void handleColumnsRemoved (const QModelIndex&, int, int);
		virtual void handleDataChanged (const QModelIndex&, const QModelIndex&);
		virtual void handleRowsAboutToBeInserted (const QModelIndex&, int, int);
		virtual void handleRowsAboutToBeRemoved (const QModelIndex&, int, int);
		virtual void handleRowsInserted (const QModelIndex&, int, int);
		virtual void handleRowsRemoved (const QModelIndex&, int, int);
		virtual void handleModelAboutToBeReset ();
		virtual void handleModelReset ();
	protected:
		/** @brief Allows to filter rows from the resulting model.
		 *
		 * This virtual function could be overridden to provide
		 * custom filtering facilities. If the row in the model
		 * should be merged into the resulting model, this function
		 * should return true, otherwise if it returns false the row
		 * would be filtered out.
		 *
		 * @param[in] model The source model the \em row belongs.
		 * @param[in] row The row index in the source \em model.
		 * @return Whether the given \em row should be displayed.
		 */
		virtual bool AcceptsRow (QAbstractItemModel *model, int row) const;
	private:
		int RowCount (QAbstractItemModel*) const;
	};
}
