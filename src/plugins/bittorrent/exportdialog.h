#ifndef PLUGINS_BITTORRENT_EXPORTDIALOG_H
#define PLUGINS_BITTORRENT_EXPORTDIALOG_H
#include <QDialog>
#include "ui_exportdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class ExportDialog : public QDialog
			{
				Q_OBJECT

				Ui::ExportDialog Ui_;
			public:
				ExportDialog (QWidget* = 0);
				virtual ~ExportDialog ();

				QString GetLocation () const;
				bool GetSettings () const;
				bool GetActive () const;
			private slots:
				void on_BrowseButton__released ();
			};
		};
	};
};

#endif

