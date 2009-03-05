#include "searcherslist.h"
#include <QInputDialog>
#include "core.h"

SearchersList::SearchersList (QWidget* parent)
: QWidget (parent)
{
	Ui_.setupUi (this);
}

void SearchersList::on_ButtonAdd__released ()
{
	QString url = QInputDialog::getText (this,
			tr ("Adding a new searcher"),
			tr ("Enter the URL of the OpenSearch description"));

	Core::Instance ().Add (url);
}

