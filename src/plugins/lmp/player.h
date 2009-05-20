#ifndef PLUGINS_LMP_PLAYER_H
#define PLUGINS_LMP_PLAYER_H
#include <memory>
#include <QDialog>
#include <QStandardItemModel>
#include <Phonon>
#include "ui_player.h"

class QStatusBar;
class QToolBar;
class QAction;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class Player : public QDialog
			{
				Q_OBJECT

				Ui::Player Ui_;
				QStatusBar *StatusBar_;
				std::auto_ptr<QAction> Play_;
				std::auto_ptr<QAction> Pause_;
				std::auto_ptr<QAction> ViewerSettings_;
				std::auto_ptr<QStandardItemModel> QueueModel_;
				enum
				{
					SourceRole = Qt::UserRole + 100
				};
			public:
				Player (QWidget* = 0);
				void Enqueue (Phonon::MediaSource*);
			private:
				QToolBar* SetupToolbar ();
				void ApplyVideoSettings (qreal, qreal, qreal, qreal);
			public slots:
				void handleStateUpdated (const QString&);
				void handleError (const QString&);
			private slots:
				void changeViewerSettings ();
				void handleSourceChanged (const Phonon::MediaSource&);
				void on_Queue__activated (const QModelIndex&);
			};
		};
	};
};

#endif

