#ifndef FANCYPOPUP_H
#define FANCYPOPUP_H
#include <QFrame>
#include <QString>
#include "ui_fancypopup.h"

namespace Main
{
	class FancyPopup : public QFrame
	{
		Q_OBJECT

		Ui::FancyPopup Ui_;
	public:
		FancyPopup (const QString&, const QString&);
		virtual ~FancyPopup ();
	protected:
		virtual void mouseReleaseEvent (QMouseEvent*);
	signals:
		void clicked ();
	};
};

#endif

