/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "matchconfigdialog.h"
#include <interfaces/ianemitter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "typedmatchers.h"
#include "fieldmatch.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	MatchConfigDialog::MatchConfigDialog (const QStringList& types, QWidget *parent)
	: QDialog (parent)
	, Types_ (QSet<QString>::fromList (types))
	{
		Ui_.setupUi (this);

		const QObjectList& emitters = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IANEmitter*> ();
		Q_FOREACH (QObject *pObj, emitters)
		{
			IInfo *ii = qobject_cast<IInfo*> (pObj);
			if (!ii)
			{
				qWarning () << Q_FUNC_INFO
						<< pObj
						<< "doesn't implement IInfo";
				continue;
			}

			Ui_.SourcePlugin_->addItem (ii->GetIcon (),
					ii->GetName (), ii->GetUniqueID ());
		}

		if (!emitters.isEmpty ())
			on_SourcePlugin__activated (0);
	}

	FieldMatch MatchConfigDialog::GetFieldMatch () const
	{
		const int fieldIdx = Ui_.FieldName_->currentIndex ();
		const int sourceIdx = Ui_.SourcePlugin_->currentIndex ();
		if (fieldIdx == -1 || sourceIdx == -1)
			return FieldMatch ();

		CurrentMatcher_->SyncToWidget ();

		const ANFieldData& data = Ui_.FieldName_->
				itemData (fieldIdx).value<ANFieldData> ();

		FieldMatch result (data.Type_, CurrentMatcher_);
		result.SetPluginID (Ui_.SourcePlugin_->
					itemData (sourceIdx).toByteArray ());
		result.SetFieldName (data.ID_);

		return result;
	}

	void MatchConfigDialog::on_SourcePlugin__activated (int idx)
	{
		Ui_.FieldName_->clear ();

		const QByteArray& id = Ui_.SourcePlugin_->
				itemData (idx).toByteArray ();
		QObject *pObj = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetPluginByID (id);
		IANEmitter *iae = qobject_cast<IANEmitter*> (pObj);
		if (!iae)
		{
			qWarning () << Q_FUNC_INFO
					<< "plugin for ID"
					<< id
					<< "doesn't implement IANEmitter:"
					<< pObj;
			return;
		}

		const QList<ANFieldData> fields = iae->GetANFields ();
		Q_FOREACH (const ANFieldData& data, fields)
		{
			if (!Types_.isEmpty () &&
					QSet<QString>::fromList (data.EventTypes_).intersect (Types_).isEmpty ())
				continue;

			Ui_.FieldName_->addItem (data.Name_,
					QVariant::fromValue<ANFieldData> (data));
		}

		if (!fields.isEmpty ())
			on_FieldName__activated (0);
	}

	void MatchConfigDialog::on_FieldName__activated (int idx)
	{
		const ANFieldData& data = Ui_.FieldName_->
				itemData (idx).value<ANFieldData> ();
		Ui_.DescriptionLabel_->setText (data.Description_);

		QLayout *lay = Ui_.ConfigWidget_->layout ();
		QLayoutItem *oldItem = 0;
		while ((oldItem = lay->takeAt (0)) != 0)
		{
			if (oldItem->widget ())
				delete oldItem->widget ();
			delete oldItem;
		}

		CurrentMatcher_ = TypedMatcherBase::Create (data.Type_);
		if (CurrentMatcher_)
			lay->addWidget (CurrentMatcher_->GetConfigWidget ());
		else
			lay->addWidget (new QLabel (tr ("Invalid matcher type %1.")
						.arg (QVariant::typeToName (data.Type_))));
	}
}
}
