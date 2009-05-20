#ifndef APPSTYLER_H
#define APPSTYLER_H
#include <QComboBox>

namespace LeechCraft
{
	class AppStyler : public QComboBox
	{
		Q_OBJECT
	public:
		AppStyler (QWidget* = 0);
	public slots:
		void accept ();
		void reject ();
	};
};

#endif

