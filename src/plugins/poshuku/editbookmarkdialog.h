#ifndef PLUGINS_POSHUKU_EDITBOOKMARKDIALOG_H
#define PLUGINS_POSHUKU_EDITBOOKMARKDIALOG_H
#include <QDialog>
#include "ui_editbookmarkdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class EditBookmarkDialog : public QDialog
			{
				Q_OBJECT

				Ui::EditBookmarkDialog Ui_;
			public:
				EditBookmarkDialog (const QModelIndex&, QWidget* = 0);

				QString GetTitle () const;
				QStringList GetTags () const;
			};
		};
	};
};

#endif

