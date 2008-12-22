#ifndef FANCYPOPUPMANAGER_H
#define FANCYPOPUPMANAGER_H
#include <deque>
#include <map>
#include <QObject>
#include <QDateTime>
#include <QPoint>

class QSystemTrayIcon;

namespace LeechCraft
{
	class FancyPopupManager : public QObject
	{
		Q_OBJECT

		typedef std::deque<QString> popups_t;
		popups_t Popups_;

		typedef std::map<QDateTime, QString> dates_t;
		dates_t Dates_;

		QSystemTrayIcon *TrayIcon_;
	public:
		FancyPopupManager (QSystemTrayIcon*,QObject* = 0);
		~FancyPopupManager ();

		void ShowMessage (const QString&);
	public slots:
		void timerTimeout ();
	private:
		void UpdateMessage ();
	};
};

#endif

