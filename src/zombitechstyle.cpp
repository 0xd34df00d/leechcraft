#include "zombitechstyle.h"
#include <QPushButton>
#include <QComboBox>
#include <QtDebug>
#include <QVector>
#include <QPoint>
#include <QPainter>

ZombiTechStyle::ZombiTechStyle ()
: White_ (0xff, 0xff, 0xff)
, MidOrange_ (0x80, 0x33, 0x00)
, TextOrange_ (0xa8, 0x43, 0x00)
, DarkOrange_ (0x2b, 0x11, 0x00)
, Background_ (0x09, 0x09, 0x09)
, LightOrange_ (0xaa, 0x44, 0x00)
, BorderOrange_ (0x6d, 0x2d, 0x02)
, DarkTextOrange_ (0x55, 0x22, 0x00)
{
}

ZombiTechStyle::~ZombiTechStyle ()
{
}

void ZombiTechStyle::polish (QPalette& palette)
{
	// Button
	palette.setBrush (QPalette::Active, QPalette::Button, DarkOrange_);
	palette.setBrush (QPalette::Inactive, QPalette::Button, DarkOrange_);
	palette.setBrush (QPalette::Disabled, QPalette::Button, DarkOrange_);

	palette.setBrush (QPalette::Active, QPalette::ButtonText, White_);
	palette.setBrush (QPalette::Inactive, QPalette::ButtonText, TextOrange_);
	palette.setBrush (QPalette::Disabled, QPalette::ButtonText, TextOrange_);

	palette.setBrush (QPalette::Active, QPalette::Light, MidOrange_.lighter (150));
	palette.setBrush (QPalette::Inactive, QPalette::Light, DarkOrange_.lighter (150));
	palette.setBrush (QPalette::Disabled, QPalette::Light, DarkOrange_.lighter (150));

	palette.setBrush (QPalette::Active, QPalette::Midlight, MidOrange_.lighter (130));
	palette.setBrush (QPalette::Inactive, QPalette::Midlight, DarkOrange_.lighter (130));
	palette.setBrush (QPalette::Disabled, QPalette::Midlight, DarkOrange_.lighter (130));

	palette.setBrush (QPalette::Active, QPalette::Mid, MidOrange_.darker (130));
	palette.setBrush (QPalette::Inactive, QPalette::Mid, DarkOrange_.darker (130));
	palette.setBrush (QPalette::Disabled, QPalette::Mid, DarkOrange_.darker (130));

	palette.setBrush (QPalette::Active, QPalette::Dark, MidOrange_.darker (150));
	palette.setBrush (QPalette::Inactive, QPalette::Dark, DarkOrange_.darker (150));
	palette.setBrush (QPalette::Disabled, QPalette::Dark, DarkOrange_.darker (150));

	// Window
	palette.setBrush (QPalette::Base, Background_);
	palette.setBrush (QPalette::AlternateBase, Background_.lighter (130));

	palette.setBrush (QPalette::Text, TextOrange_);

	palette.setBrush (QPalette::Window, Background_);

	palette.setBrush (QPalette::Active, QPalette::WindowText, TextOrange_);
	palette.setBrush (QPalette::Inactive, QPalette::WindowText, DarkTextOrange_);
	palette.setBrush (QPalette::Disabled, QPalette::WindowText, DarkTextOrange_);

	palette.setBrush (QPalette::BrightText, BorderOrange_);
}

void ZombiTechStyle::polish (QWidget *widget)
{
	if (qobject_cast<QPushButton*> (widget) ||
			qobject_cast<QComboBox*> (widget))
		widget->setAttribute (Qt::WA_Hover, true);
}

void ZombiTechStyle::unpolish (QWidget *widget)
{
	if (qobject_cast<QPushButton*> (widget) ||
			qobject_cast<QComboBox*> (widget))
		widget->setAttribute (Qt::WA_Hover, false);
}

int ZombiTechStyle::pixelMetric (PixelMetric metric,
		const QStyleOption *option,
		const QWidget *widget) const
{
	switch (metric)
	{
		case PM_ButtonMargin:
			return QPlastiqueStyle::pixelMetric (metric, option, widget);
		default:
			return QPlastiqueStyle::pixelMetric (metric, option, widget);
	}
}

int ZombiTechStyle::styleHint (StyleHint hint, const QStyleOption* option,
		 const QWidget* widget,
		 QStyleHintReturn *returnData) const
{
	switch (hint)
	{
		case SH_DitherDisabledText:
			return false;
		case SH_EtchDisabledText:
			return true;
		default:
			return QPlastiqueStyle::styleHint (hint, option, widget, returnData);
	}
}

void ZombiTechStyle::drawPrimitive (PrimitiveElement element,
		const QStyleOption* option,
		QPainter* painter,
		const QWidget* widget) const
{
	switch (element)
	{
		case PE_PanelButtonCommand:
			{
				int x, y, width, height;
				option->rect.getRect (&x, &y, &width, &height);

				painter->save ();

				painter->setRenderHint (QPainter::Antialiasing, true);

				QVector<QPoint> points;
				points << QPoint (x + 10, y)
					<< QPoint (x + width, y)
					<< QPoint (x + width, y + height - 10)
					<< QPoint (x + width - 10, y + height)
					<< QPoint (x, y + height)
					<< QPoint (x, y + 10);
				painter->setPen (QPen (QBrush (BorderOrange_), 2.5));
				painter->drawLine (points [points.size () - 1],
						points [0]);
				for (int i = 1; i < points.size (); ++i)
					painter->drawLine (points [i - 1], points [i]);

				painter->restore ();
				break;
			}
		default:
			QPlastiqueStyle::drawPrimitive (element, option, painter, widget);
	}
}

