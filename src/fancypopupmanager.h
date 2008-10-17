#ifndef FANCYPOPUPMANAGER_H
#define FANCYPOPUPMANAGER_H
#include <vector>
#include <map>
#include <QObject>
#include <QDateTime>
#include <QPoint>

namespace Main
{
	class FancyPopup;

	class FancyPopupManager : public QObject
	{
		Q_OBJECT

		typedef std::vector<FancyPopup*> popups_t;
		popups_t Popups_;

		typedef std::map<QDateTime, FancyPopup*> dates_t;
		dates_t Dates_;

		struct ValueFinder 
		{
			Main::FancyPopup *Popup_;
			
			ValueFinder (Main::FancyPopup *p)
			: Popup_ (p)
			{
			}

			bool operator() (Main::FancyPopupManager::dates_t::value_type i)
			{
				return i.second == Popup_;
			}
		};
	public:
		FancyPopupManager (QObject* = 0);
		~FancyPopupManager ();

		void ShowMessage (const QString&, const QString&);
	public slots:
		void timerTimeout ();
		void popupClicked ();
	private:
		void RecalculatePositions ();
		QPoint CalculatePosition (FancyPopup*);
		QPoint CalculatePosition (FancyPopup*, int);
	};
};

#endif

