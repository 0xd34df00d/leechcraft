#include "editbookmarkdialog.h"
#include <plugininterface/tagscompleter.h>
#include "favoritesmodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			EditBookmarkDialog::EditBookmarkDialog (const QModelIndex& index,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				new Util::TagsCompleter (Ui_.Tags_);
				Ui_.Tags_->AddSelector ();

				QString url = index.sibling (index.row (), FavoritesModel::ColumnURL)
					.data ().toString ();
				if (url.size () > 100)
					url = QString ("%1...").arg (url.left (97));
				Ui_.Label_->setText (url);

				Ui_.Title_->setText (index.sibling (index.row (), FavoritesModel::ColumnTitle)
						.data ().toString ());

				Ui_.Tags_->setText (index.sibling (index.row (), FavoritesModel::ColumnTags)
						.data ().toString ());
			}

			QString EditBookmarkDialog::GetTitle () const
			{
				return Ui_.Title_->text ();
			}

			QStringList EditBookmarkDialog::GetTags () const
			{
				return Core::Instance ().GetProxy ()->GetTagsManager ()->
					Split (Ui_.Tags_->text ());
			}
		};
	};
};

