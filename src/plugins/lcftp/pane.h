#ifndef PLUGINS_LCFTP_PANE_H
#define PLUGINS_LCFTP_PANE_H
#include <QWidget>
#include "ui_pane.h"
#include "structures.h"

class QDirModel;
class QSortFilterProxyModel;
class QStandardItemModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			/** The pane can be either local or remote. Local pane is
			 * the one that browses through the local filesystem, has
			 * the directory completion features etc. Remote on browses
			 * on remote FTP hosts.
			 */
			class Pane : public QWidget
			{
				Q_OBJECT

				Ui::Pane Ui_;

				QSortFilterProxyModel *StaticSource_;
				QDirModel *DirModel_;
				QStandardItemModel *RemoteModel_;
				int JobID_;
			public:
				/** Initializes, constructs the interface, sets up the
				 * pane as the remote one.
				 */
				Pane (QWidget* = 0);
				virtual ~Pane ();

				/** Makes the pane browse the given url. If the pane is
				 * in local mode, it switches it to the remote mode.
				 */
				void SetURL (const QUrl& url);
				/** Makes the pane browse the given local location. If
				 * the pane is not in local mode, it switches it to the
				 * local mode.
				 */
				void Navigate (const QString& location);
			private:
				bool IsLocal () const;
			private slots:
				void on_Address__returnPressed ();
				void handleFetchedEntry (const FetchedEntry&);
			};
		};
	};
};

#endif

