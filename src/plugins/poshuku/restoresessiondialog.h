#ifndef PLUGINS_POSHUKU_RESTORESESSIONDIALOG_H
#define PLUGINS_POSHUKU_RESTORESESSIONDIALOG_H
#include <QDialog>
#include "ui_restoresessiondialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class RestoreSessionDialog : public QDialog
			{
				Q_OBJECT

				Ui::RestoreSessionDialog Ui_;
			public:
				RestoreSessionDialog (QWidget* = 0);
				virtual ~RestoreSessionDialog ();

				void AddPair (const QString&, const QString&);
				QStringList GetSelectedURLs () const;
			private slots:
				void on_SelectAll__released ();
				void on_SelectNone__released ();
			};
		};
	};
};

#endif

