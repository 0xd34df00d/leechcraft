#include "searchtext.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			SearchText::SearchText (const QString& text, QWidget *parent)
			: QDialog (parent)
			, Text_ (text)
			{
				Ui_.setupUi (this);
				Ui_.Label_->setText (tr ("Search %1 with:").arg (text));

				QStringList categories = Core::Instance ().GetProxy ()->GetSearchCategories ();
				Q_FOREACH (QString cat, categories)
					new QTreeWidgetItem (Ui_.Tree_, QStringList (cat));

				on_MarkAll__released ();

				connect (this,
						SIGNAL (accepted ()),
						this,
						SLOT (doSearch ()));
			}

			void SearchText::doSearch ()
			{
				QStringList selected;

				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					if (Ui_.Tree_->topLevelItem (i)->checkState (0) == Qt::Checked)
						selected << Ui_.Tree_->topLevelItem (i)->text (0);

				if (!selected.size ())
					return;

				QString query = Text_;

				if (selected.size () == 1)
					query = QString ("%1 ca:%2").arg (Text_).arg (selected.at (0));
				else
				{
					query += " (";
					query += QString ("ca:%1").arg (selected.at (0));
					for (int i = 1; i < selected.size (); ++i)
						query += QString (" OR ca:%1").arg (selected.at (i));
					query += ")";
				}

				Core::Instance ().GetProxy ()->OpenSummary (query);
			}

			void SearchText::on_MarkAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Checked);
			}

			void SearchText::on_UnmarkAll__released ()
			{
				for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
					Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Unchecked);
			}
		};
	};
};

