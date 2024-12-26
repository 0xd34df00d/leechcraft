/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "matchconfigdialog.h"
#include <QtDebug>
#include <util/xpc/stdanfields.h>
#include <interfaces/iinfo.h>
#include "typedmatchers.h"
#include "fieldmatch.h"

namespace LC::AdvancedNotifications
{
	MatchConfigDialog::MatchConfigDialog (const QHash<QObject*, QList<AN::FieldData>>& map, QWidget *parent)
	: QDialog (parent)
	, FieldsMap_ (map)
	{
		Ui_.setupUi (this);
		connect (Ui_.SourcePlugin_,
				qOverload<int> (&QComboBox::currentIndexChanged),
				this,
				&MatchConfigDialog::ShowPluginFields);
		connect (Ui_.FieldName_,
				qOverload<int> (&QComboBox::currentIndexChanged),
				this,
				&MatchConfigDialog::ShowField);

		if (!FieldsMap_ [nullptr].isEmpty ())
			Ui_.SourcePlugin_->addItem (tr ("Standard fields"));

		for (auto i = FieldsMap_.begin (); i != FieldsMap_.end (); ++i)
		{
			if (!i.key ())
				continue;

			auto ii = qobject_cast<IInfo*> (i.key ());
			Ui_.SourcePlugin_->addItem (ii->GetIcon (),
					ii->GetName (), QVariant::fromValue (i.key ()));
		}
	}

	FieldMatch MatchConfigDialog::GetFieldMatch () const
	{
		const int fieldIdx = Ui_.FieldName_->currentIndex ();
		const int sourceIdx = Ui_.SourcePlugin_->currentIndex ();
		if (fieldIdx == -1 || sourceIdx == -1)
			return FieldMatch ();

		CurrentMatcher_->SyncToWidget ();

		const auto& data = Ui_.FieldName_->itemData (fieldIdx).value<AN::FieldData> ();

		FieldMatch result (data.Type_, CurrentMatcher_);
		if (const auto plugin = Ui_.SourcePlugin_->itemData (sourceIdx).value<QObject*> ())
			result.SetPluginID (qobject_cast<IInfo*> (plugin)->GetUniqueID ());
		result.SetFieldName (data.ID_);

		return result;
	}

	void MatchConfigDialog::SetFieldMatch (const FieldMatch& match)
	{
		if (!match.GetMatcher ())
			qWarning () << Q_FUNC_INFO
					<< "no matcher for"
					<< match.GetPluginID ()
					<< match.GetFieldName ();

		const int fieldIdx = SelectPlugin (match.GetPluginID ().toLatin1 (), match.GetFieldName ());
		if (fieldIdx == -1)
			return;

		Ui_.FieldName_->setCurrentIndex (fieldIdx);

		if (CurrentMatcher_)
		{
			CurrentMatcher_->SetValue (match.GetMatcher ()->GetValue ());
			CurrentMatcher_->SyncWidgetTo ();
		}
	}

	int MatchConfigDialog::SelectPlugin (const QByteArray& pluginId, const QString& fieldId)
	{
		int plugIdx = -1;
		if (!pluginId.isEmpty ())
			for (int i = 0; i < Ui_.SourcePlugin_->count (); ++i)
			{
				const auto plugin = Ui_.SourcePlugin_->itemData (i).value<QObject*> ();
				if (plugin && qobject_cast<IInfo*> (plugin)->GetUniqueID () == pluginId)
				{
					plugIdx = i;
					break;
				}
			}

		auto tryIdx = [this, &fieldId] (int idx)
		{
			const auto pObj = Ui_.SourcePlugin_->itemData (idx).value<QObject*> ();
			const auto& fields = FieldsMap_ [pObj];

			for (int i = 0; i < fields.size (); ++i)
				if (fields.at (i).ID_ == fieldId)
				{
					Ui_.SourcePlugin_->setCurrentIndex (idx);
					return i;
				}

			return -1;
		};

		if (plugIdx != -1)
		{
			const auto idx = tryIdx (plugIdx);
			if (idx != -1)
				return idx;
		}

		return tryIdx (0);
	}

	void MatchConfigDialog::AddFields (const QList<AN::FieldData>& fields)
	{
		for (const auto& data : fields)
			Ui_.FieldName_->addItem (data.Name_, QVariant::fromValue (data));
	}

	void MatchConfigDialog::ShowPluginFields (int idx)
	{
		Ui_.FieldName_->clear ();

		const auto pObj = Ui_.SourcePlugin_->itemData (idx).value<QObject*> ();
		AddFields (FieldsMap_ [pObj]);
	}

	void MatchConfigDialog::ShowField (int idx)
	{
		const auto& data = Ui_.FieldName_->itemData (idx).value<AN::FieldData> ();
		Ui_.DescriptionLabel_->setText (data.Description_);

		QLayout *lay = Ui_.ConfigWidget_->layout ();
		while (auto oldItem = lay->takeAt (0))
		{
			delete oldItem->widget ();
			delete oldItem;
		}

		CurrentMatcher_ = TypedMatcherBase::Create (data.Type_, data);
		if (CurrentMatcher_)
			lay->addWidget (CurrentMatcher_->GetConfigWidget ());
		else
			lay->addWidget (new QLabel (tr ("Invalid matcher type %1.")
						.arg (QVariant::typeToName (data.Type_))));
	}
}
