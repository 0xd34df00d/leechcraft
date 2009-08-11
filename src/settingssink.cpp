#include "settingssink.h"
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <QtDebug>

Q_DECLARE_METATYPE (LeechCraft::Util::XmlSettingsDialog*);

namespace LeechCraft
{
	using namespace Util;

	SettingsSink::SettingsSink (const QString& name,
			XmlSettingsDialog* dialog,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		// Because Qt Designer inserts one.
		Ui_.Dialogs_->removeWidget (Ui_.Dialogs_->currentWidget ());

		Add (name, windowIcon (), dialog);
	}

	SettingsSink::~SettingsSink ()
	{
	}

	void SettingsSink::AddDialog (const QObject *object)
	{
		IInfo *info = qobject_cast<IInfo*> (object);
		IHaveSettings *ihs = qobject_cast<IHaveSettings*> (object);

		Add (info->GetName (), info->GetIcon (), ihs->GetSettingsDialog ().get ());

		Ui_.Dialogs_->setCurrentIndex (0);
	}

	void SettingsSink::Add (const QString& name, const QIcon& wicon,
			XmlSettingsDialog *widget)
	{
		Ui_.Dialogs_->addWidget (widget);
		adjustSize ();

		QTreeWidgetItem *parent = new QTreeWidgetItem (Ui_.Tree_, QStringList (name));
		parent->setData (0, RDialog, QVariant::fromValue<XmlSettingsDialog*> (widget));
		parent->setIcon (0, wicon);
		if (widget->GetPages ().size () > 1)
			Q_FOREACH (QString page, widget->GetPages ())
				new QTreeWidgetItem (parent, QStringList (page));

		connect (this,
				SIGNAL (accepted ()),
				widget,
				SLOT (accept ()));
		connect (this,
				SIGNAL (rejected ()),
				widget,
				SLOT (reject ()));

		Ui_.Tree_->expandAll ();
	}

	void SettingsSink::on_Tree__currentItemChanged (QTreeWidgetItem *current)
	{
		if (current == Ui_.Tree_->invisibleRootItem ())
			return;

		int pindex = 0;
		XmlSettingsDialog *dialog = 0;

		if (current->parent ())
		{
			QTreeWidgetItem *parent = current->parent ();
			dialog = parent->data (0, RDialog).value<XmlSettingsDialog*> ();
			pindex = parent->indexOfChild (current);
		}
		else
			dialog = current->data (0, RDialog).value<XmlSettingsDialog*> ();

		Ui_.Dialogs_->setCurrentWidget (dialog);
		dialog->SetPage (pindex);
	}
};

