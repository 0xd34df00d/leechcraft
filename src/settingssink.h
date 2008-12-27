#ifndef SETTINGSSINK_H
#define SETTINGSSINK_H
#include <QDialog>
#include "ui_settingssink.h"

namespace LeechCraft
{
	namespace Util
	{
		class XmlSettingsDialog;
	};

	class SettingsSink : public QDialog
	{
		Q_OBJECT

		Ui::SettingsSink Ui_;
	public:
		SettingsSink (const QString&,
				Util::XmlSettingsDialog*,
				QWidget* = 0);
		virtual ~SettingsSink ();

		void AddDialog (const QObject*);
	private:
		void Add (const QString&, const QIcon&, QWidget*);
	};
};

#endif

