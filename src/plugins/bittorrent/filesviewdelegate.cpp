#include <QModelIndex>
#include <QSpinBox>
#include <QLineEdit>
#include <QtDebug>
#include <plugininterface/treeitem.h>
#include "filesviewdelegate.h"
#include "torrentfilesmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			namespace
			{
				bool HasChildren (const QModelIndex& index)
				{
					return index.model ()->rowCount (index.sibling (index.row (), 0));
				}
			};

			using LeechCraft::Util::TreeItem;
			
			FilesViewDelegate::FilesViewDelegate (QObject *parent)
			: QItemDelegate (parent)
			{
			}
			
			FilesViewDelegate::~FilesViewDelegate ()
			{
			}
			
			QWidget* FilesViewDelegate::createEditor (QWidget *parent,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				if (index.column () == 2 &&
						!HasChildren (index))
				{
					QSpinBox *box = new QSpinBox (parent);
					box->setRange (0, 7);
					return box;
				}
				else if (index.column () == 0 &&
						!HasChildren (index))
					return new QLineEdit (parent);
				else
					return QItemDelegate::createEditor (parent, option, index);
			}
			
			void FilesViewDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				QItemDelegate::paint (painter, option, index);
			}
			
			void FilesViewDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
			{
				if (index.column () == 2 &&
						!HasChildren (index))
					qobject_cast<QSpinBox*> (editor)->
						setValue (static_cast<TreeItem*> (index.internalPointer ())->
								Data (2).toInt ());
				else if (index.column () == 0 &&
						!HasChildren (index))
				{
					QVariant data = static_cast<TreeItem*> (index.internalPointer ())->
						Data (0, TorrentFilesModel::RawDataRole);
					qobject_cast<QLineEdit*> (editor)->setText (data.toString ());
				}
				else
					QItemDelegate::setEditorData (editor, index);
			}
			
			void FilesViewDelegate::setModelData (QWidget *editor, QAbstractItemModel *model,
					const QModelIndex& index) const
			{
				if (index.column () == 2)
				{
					int value = qobject_cast<QSpinBox*> (editor)->value ();
					model->setData (index, value);
				}
				else if (index.column () == 0)
				{
					model->setData (index,
							qobject_cast<QLineEdit*> (editor)->text ());
				}
				else
					QItemDelegate::setModelData (editor, model, index);
			}
			
		};
	};
};

