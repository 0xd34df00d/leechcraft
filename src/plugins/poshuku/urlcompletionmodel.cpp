#include "urlcompletionmodel.h"
#include <stdexcept>
#include <QUrl>
#include <QtDebug>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			URLCompletionModel::URLCompletionModel (QObject *parent)
			: QAbstractItemModel (parent)
			, Valid_ (false)
			{
			}
			
			URLCompletionModel::~URLCompletionModel ()
			{
			}
			
			int URLCompletionModel::columnCount (const QModelIndex&) const
			{
				return 1;
			}
			
			QVariant URLCompletionModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				if (role == Qt::DisplayRole)
					return Items_ [index.row ()].Title_ + " [" + Items_ [index.row ()].URL_ + "]";
				else if (role == Qt::DecorationRole)
					return Core::Instance ().GetIcon (QUrl (Items_ [index.row ()].URL_));
				else if (role == Qt::EditRole)
					return Base_ + index.row ();
				else if (role == RoleURL)
					return Items_ [index.row ()].URL_;
				else
					return QVariant ();
			}
			
			Qt::ItemFlags URLCompletionModel::flags (const QModelIndex&) const
			{
				return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
			}
			
			QVariant URLCompletionModel::headerData (int, Qt::Orientation, int) const
			{
				return QVariant ();
			}
			
			QModelIndex URLCompletionModel::index (int row, int column,
					const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex URLCompletionModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int URLCompletionModel::rowCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return 0;
			
				return Items_.size ();
			}
			
			void URLCompletionModel::setBase (const QString& str)
			{
				Valid_ = false;
				Base_ = str;
			
				Populate ();
			}
			
			void URLCompletionModel::handleItemAdded (const HistoryItem&)
			{
				Valid_ = false;
				Populate ();
			}
			
			void URLCompletionModel::Populate ()
			{
				if (!Valid_)
				{
					Valid_ = true;
			
					int size = Items_.size () - 1;
					if (size)
						beginRemoveRows (QModelIndex (), 0, size);
					Items_.clear ();
					if (size)
						endRemoveRows ();
			
					try
					{
						Core::Instance ().GetStorageBackend ()->LoadResemblingHistory (Base_, Items_);
					}
					catch (const std::runtime_error& e)
					{
						qWarning () << Q_FUNC_INFO << e.what ();
						Valid_ = false;
					}
			
					size = Items_.size () - 1;
					beginInsertRows (QModelIndex (), 0, size);
					endInsertRows ();
				}
			}
		};
	};
};

