/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "fsettings.h"
//#include <QtCore/QDebug>

fSettings::fSettings() : QSettings("NBL", "fsirc")
{

}

int fSettings::findValue(QVariant fValue, QString array, QString key)
{
	// finds value in current array
	// Why does QSettings not have it?
	int size=beginReadArray(array);
	int ret=-1;
	for(int i=0; i<size; i++)
	{
		setArrayIndex(i);
		if(value(key)==fValue) ret=i;
	}
	endArray();
	return ret;

}

int fSettings::appendValue(QVariant fValue, QString array, QString key, int unique)
{
	if((!unique)||(findValue(fValue,array,key)==-1))
	{
		int size=beginReadArray(array);
		endArray();
		beginWriteArray(array);
		setArrayIndex(size);
		setValue(key, fValue);
		endArray();
		return 1;
	}
	else
		return 0;
}

QStringList fSettings::toStringList(QString array, QString key)
{
	int size=beginReadArray(array);
	QStringList uList;
	for(int i=0; i<size; i++)
	{
		setArrayIndex(i);
		uList.append(value(key).toString());
	}
	endArray();
	return uList;
}

QVariant fSettings::lastItem(QString array, QString key)
{
	int size=beginReadArray(array);
	setArrayIndex(size-1);
	QVariant val=value(key);
	endArray();
	return val;
}
