/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef TYPES_H
#define TYPES_H

#include <QSizeF>
#include <QHash>
#include <QString>
#include <QPrinter>


class Types
{
public:
    enum PosterSizeModes {
        PosterSizeModeAbsolute,
        PosterSizeModePercentual,
        PosterSizeModePages,
        PosterSizeModeNone
    };

    enum ColorTypes {
        ColorTypeMonochrome,
        ColorTypeGreyscale,
        ColorTypePalette,
        ColorTypeRGB,
        ColorTypeRGBA,
        ColorTypeCMYK
    };

    enum UnitsOfLength {
        UnitOfLengthMeter,
        UnitOfLengthMillimeter,
        UnitOfLengthCentimeter,
        UnitOfLengthInch,
        UnitOfLengthFeet,
        UnitOfLengthPoints
    };

    static const QHash<UnitsOfLength, QPair<QString, double> > &unitsOfLength();
    static double convertBetweenUnitsOfLength(double distance, UnitsOfLength sourceUnit, UnitsOfLength targetUnit);
    static QSizeF convertBetweenUnitsOfLength(const QSizeF &size, UnitsOfLength sourceUnit, UnitsOfLength targetUnit);
    static UnitsOfLength unitOfLenthFromString(const QString &string);
    static const QHash<QString, QSizeF> &paperFormats();
    static QSizeF paperSize(const QString &format, QPageLayout::Orientation orientation, UnitsOfLength unit);

    // These two functions help to format help text. I know, they do not belong, here.
    static QString cleanString(const QString &dirtyString);
    static QString newlineToParagraph(const QString &nlText);
};

#endif // TYPES_H
