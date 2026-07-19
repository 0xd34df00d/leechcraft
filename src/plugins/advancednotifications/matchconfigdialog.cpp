/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "matchconfigdialog.h"
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/xpc/stdanfields.h>
#include <interfaces/iinfo.h>
#include "fieldmatch.h"

namespace LC::AdvancedNotifications
{
	MatchConfigDialog::MatchConfigDialog (const QHash<QObject*, QList<AN::FieldData>>& map, QWidget *parent)
	: QDialog (parent)
	, FieldsMap_ (map)
	{
		Ui_.setupUi (this);
		connect (Ui_.SourcePlugin_,
				&QComboBox::currentIndexChanged,
				this,
				&MatchConfigDialog::ShowPluginFields);
		connect (Ui_.FieldName_,
				&QComboBox::activated,
				this,
				[this] (int idx) { ShowField (idx); });

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

	std::optional<FieldMatch> MatchConfigDialog::GetFieldMatch () const
	{
		const int fieldIdx = Ui_.FieldName_->currentIndex ();
		const int sourceIdx = Ui_.SourcePlugin_->currentIndex ();
		if (fieldIdx == -1 || sourceIdx == -1)
			return {};

		return Util::Visit (CurrentConfigWidget_,
				[&] (const IConfigWidget_ptr& configWidget) -> std::optional<FieldMatch>
				{
					if (!configWidget)
						return {};

					const auto& data = Ui_.FieldName_->itemData (fieldIdx).value<AN::FieldData> ();

					FieldMatch match
					{
						.Name_ = data.ID_,
						.Type_ = data.Type_,
						.Matcher_ = configWidget->GetConfiguredMatcher (),
					};
					if (const auto plugin = Ui_.SourcePlugin_->itemData (sourceIdx).value<QObject*> ())
						match.PluginID_ = qobject_cast<IInfo*> (plugin)->GetUniqueID ();
					return match;
				},
				[] (const QLabel&) { return std::optional<FieldMatch> {}; });
	}

	void MatchConfigDialog::SetFieldMatch (const FieldMatch& match)
	{
		const int fieldIdx = SelectPlugin (match.PluginID_.toLatin1 (), match.Name_);
		if (fieldIdx == -1)
			return;

		Ui_.FieldName_->setCurrentIndex (fieldIdx);
		ShowField (fieldIdx, match.Matcher_);
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
		if (const auto curField = Ui_.FieldName_->currentIndex ();
			curField >= 0)
			ShowField (curField);
	}

	void MatchConfigDialog::ShowField (int idx, const std::optional<ValueMatcherOrData>& matcher)
	{
		const auto& data = Ui_.FieldName_->itemData (idx).value<AN::FieldData> ();
		Ui_.DescriptionLabel_->setText (data.Description_);

		const auto lay = Ui_.ConfigWidget_->layout ();

		auto configWidget = CreateMatcherConfigWidget (data, matcher.and_then (Util::Visitor {
					[&data] (const QVariantMap&)
					{
						qWarning () << "no matcher for" << data.Name_ << data.ID_;
						// TODO disable or reset the edit widget
						return std::optional<AN::ValueMatcher> {};
					},
					[] (const AN::ValueMatcher& matcher)
					{
						// TODO enable edit widget
						return std::optional { matcher };
					}
				}));
		if (configWidget)
		{
			CurrentConfigWidget_ = configWidget;
			lay->addWidget (&configWidget->GetWidget ());
		}
		else
		{
			auto& label = CurrentConfigWidget_.emplace<QLabel> (tr ("Invalid or mismatching matcher type %1.")
						.arg (QMetaType { data.Type_ }.name ()));
			lay->addWidget (&label);
		}
	}
}
