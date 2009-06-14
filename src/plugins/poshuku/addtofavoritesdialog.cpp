#include "addtofavoritesdialog.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			using LeechCraft::Util::TagsCompleter;
			using LeechCraft::Util::TagsCompletionModel;
			
			AddToFavoritesDialog::AddToFavoritesDialog (const QString& title,
					const QString& url,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.URLLabel_->setText (url);
				Ui_.TitleEdit_->setText (title);
				Ui_.TagsEdit_->setText (tr ("untagged"));
			
				TagsCompleter_.reset (new TagsCompleter (Ui_.TagsEdit_));
				Ui_.TagsEdit_->AddSelector ();
			}
			
			AddToFavoritesDialog::~AddToFavoritesDialog ()
			{
			}
			
			QString AddToFavoritesDialog::GetTitle () const
			{
				return Ui_.TitleEdit_->text ();
			}
			
			QStringList AddToFavoritesDialog::GetTags () const
			{
				return Core::Instance ().GetProxy ()->
					GetTagsManager ()->Split (Ui_.TagsEdit_->text ());
			}
		};
	};
};

