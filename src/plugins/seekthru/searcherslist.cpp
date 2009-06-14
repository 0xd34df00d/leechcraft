#include "searcherslist.h"
#include <QInputDialog>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			SearchersList::SearchersList (QWidget* parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);
				Ui_.SearchersView_->setModel (&Core::Instance ());
				connect (Ui_.SearchersView_->selectionModel (),
						SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (handleCurrentChanged (const QModelIndex&)));
			}
			
			void SearchersList::handleCurrentChanged (const QModelIndex& current)
			{
				Ui_.ButtonRemove_->setEnabled (current.isValid ());
			
				Current_ = current;
			
				QString description = current.data (Core::RoleDescription).toString ();
				if (description.isEmpty ())
					Ui_.Description_->setText (tr ("No description"));
				else
					Ui_.Description_->setText (description);
			
				QString longName = current.data (Core::RoleLongName).toString ();
				if (longName.isEmpty ())
					Ui_.LongName_->setText (tr ("No long name"));
				else
					Ui_.LongName_->setText (longName);
			
				QStringList tags = current.data (Core::RoleTags).toStringList ();
				Ui_.Tags_->setText (tags.join (" "));
			
				QString contact = current.data (Core::RoleContact).toString ();
				if (contact.isEmpty ())
					Ui_.Contact_->setText (tr ("No contacts information"));
				else
					Ui_.Contact_->setText (contact);
			
				QString developer = current.data (Core::RoleDeveloper).toString ();
				if (developer.isEmpty ())
					Ui_.Developer_->setText (tr ("No developer information"));
				else
					Ui_.Developer_->setText (developer);
			
				QString attribution = current.data (Core::RoleAttribution).toString ();
				if (attribution.isEmpty ())
					Ui_.Attribution_->setText (tr ("No attribution information"));
				else
					Ui_.Attribution_->setText (attribution);
				
				QString right = current.data (Core::RoleRight).toString ();
				if (right.isEmpty ())
					Ui_.Right_->setText (tr ("No right information"));
				else
					Ui_.Right_->setText (right);
			
				bool adult = current.data (Core::RoleAdult).toBool ();
				Ui_.Adult_->setText (adult ? tr ("Yes") : tr ("No"));
			
				QStringList languages = current.data (Core::RoleLanguages).toStringList ();
			   	Ui_.Languages_->addItems (languages);	
			}
			
			void SearchersList::on_ButtonAdd__released ()
			{
				QString url = QInputDialog::getText (this,
						tr ("Adding a new searcher"),
						tr ("Enter the URL of the OpenSearch description"));
			
				if (url.isEmpty ())
					return;
			
				Core::Instance ().Add (url);
			}
			
			void SearchersList::on_ButtonRemove__released ()
			{
				Core::Instance ().Remove (Ui_.SearchersView_->selectionModel ()->currentIndex ());
			}
			
			void SearchersList::on_Tags__textEdited (const QString& text)
			{
				Core::Instance ().SetTags (Current_, text.split (' ', QString::SkipEmptyParts));
			}
			
		};
	};
};

