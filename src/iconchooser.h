#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H
#include <QComboBox>

namespace LeechCraft
{
	class IconChooser : public QComboBox
	{
		Q_OBJECT

		QStringList Sets_;
	public:
		IconChooser (const QStringList&, QWidget* = 0);
	public slots:
		void accept ();
		void reject ();
	signals:
		void requestNewIconSet ();
	};
};

#endif

