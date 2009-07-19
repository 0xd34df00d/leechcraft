#ifndef PLUGINS_CSTP_ADDTASK_H
#define PLUGINS_CSTP_ADDTASK_H
#include <QDialog>
#include <QUrl>
#include "ui_addtask.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class AddTask : public QDialog
			{
				Q_OBJECT

				Ui::AddTask Ui_;
				bool UserModifiedFilename_;
			public:
				AddTask (QWidget* = 0);
				AddTask (const QUrl&, const QString&, QWidget* = 0);
				virtual ~AddTask ();

				struct Task
				{
					QUrl URL_;
					QString LocalPath_;
					QString Filename_;
					QString Comment_;

					Task (const QUrl&,
							const QString&,
							const QString&,
							const QString&);
				};

				Task GetTask () const;
			public slots:
				virtual void accept ();
			private slots:
				void on_URL__textEdited (const QString&);
				void on_LocalPath__textChanged ();
				void on_Filename__textEdited ();
				void on_BrowseButton__released ();
			private:
				void CheckOK ();
			};
		};
	};
};

#endif

