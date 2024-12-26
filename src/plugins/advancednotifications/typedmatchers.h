/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QString>
#include <QVariant>
#include <interfaces/an/ianemitter.h>

namespace Ui
{
	class BoolMatcherConfigWidget;
	class IntMatcherConfigWidget;
	class StringLikeMatcherConfigWidget;
}

namespace LC::AdvancedNotifications
{
	class TypedMatcherBase;

	using TypedMatcherBase_ptr = std::shared_ptr<TypedMatcherBase>;

	class TypedMatcherBase
	{
	protected:
		QWidget *CW_ = nullptr;
	public:
		static TypedMatcherBase_ptr Create (QMetaType::Type, const AN::FieldData& = {});

		virtual QVariantMap Save () const = 0;
		virtual void Load (const QVariantMap&) = 0;

		virtual void SetValue (const AN::FieldValue&) = 0;
		virtual void SetValue (const QVariant&) = 0;
		virtual AN::FieldValue GetValue () const = 0;

		virtual bool Match (const QVariant&) const = 0;

		virtual QString GetHRDescription () const = 0;

		virtual QWidget* GetConfigWidget () = 0;
		virtual void SyncToWidget () = 0;
		virtual void SyncWidgetTo () = 0;
	};

	class StringLikeMatcher : public TypedMatcherBase
	{
	protected:
		AN::StringFieldValue Value_ { {} };
		const QStringList Allowed_;

		std::shared_ptr<Ui::StringLikeMatcherConfigWidget> Ui_;
	public:
		explicit StringLikeMatcher (const QStringList& variants = {});

		QVariantMap Save () const override;
		void Load (const QVariantMap&) override;

		void SetValue (const AN::FieldValue&) override;
		void SetValue (const QVariant&) override;
		AN::FieldValue GetValue () const override;

		QWidget* GetConfigWidget () override;
		void SyncToWidget () override;
		void SyncWidgetTo () override;
	};

	class StringMatcher final : public StringLikeMatcher
	{
	public:
		using StringLikeMatcher::StringLikeMatcher;

		bool Match (const QVariant&) const override;

		QString GetHRDescription () const override;
	};

	class StringListMatcher final : public StringLikeMatcher
	{
	public:
		explicit StringListMatcher (const QStringList& variants = {});

		bool Match (const QVariant&) const override;

		QString GetHRDescription () const override;
	};

	class UrlMatcher final : public StringLikeMatcher
	{
	public:
		UrlMatcher () = default;

		bool Match (const QVariant&) const override;

		QString GetHRDescription () const override;
	};

	class BoolMatcher final : public TypedMatcherBase
	{
		AN::BoolFieldValue Value_ { false };

		const QString FieldName_;
		std::shared_ptr<Ui::BoolMatcherConfigWidget> Ui_;
	public:
		explicit BoolMatcher (const QString& fieldName);

		QVariantMap Save () const override;
		void Load (const QVariantMap&) override;

		void SetValue (const AN::FieldValue&) override;
		void SetValue (const QVariant&) override;
		AN::FieldValue GetValue () const override;

		bool Match (const QVariant&) const override;

		QString GetHRDescription () const override;
		QWidget* GetConfigWidget () override;
		void SyncToWidget () override;
		void SyncWidgetTo () override;
	};

	class IntMatcher final : public TypedMatcherBase
	{
		AN::IntFieldValue Value_ { 0, AN::IntFieldValue::OEqual };

		std::shared_ptr<Ui::IntMatcherConfigWidget> Ui_;
		QMap<AN::IntFieldValue::Operations, int> Ops2pos_;
	public:
		IntMatcher ();

		QVariantMap Save () const override;
		void Load (const QVariantMap&) override;

		void SetValue (const AN::FieldValue&) override;
		void SetValue (const QVariant&) override;
		AN::FieldValue GetValue () const override;

		bool Match (const QVariant&) const override;

		QString GetHRDescription () const override;
		QWidget* GetConfigWidget () override;
		void SyncToWidget () override;
		void SyncWidgetTo () override;
	};
}
