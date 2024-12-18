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

#include "types.h"
#include <QtDebug>
#if QT_VERSION < 0x060000
#include <QRegExp>
#else
#include <QRegularExpression>
#endif

const QHash<Types::UnitsOfLength, QPair<QString, double> > &Types::unitsOfLength()
{
    static QHash<UnitsOfLength, QPair<QString, double> > units;
    if (units.empty()) {
        static const struct
        {
            Types::UnitsOfLength unit;
            QString name;
            double cm;
        } unitsOfLength[] = {
            {Types::UnitOfLengthMeter,      QLatin1String("m"),        100.00},
            {Types::UnitOfLengthMillimeter, QLatin1String("mm"),         0.10},
            {Types::UnitOfLengthCentimeter, QLatin1String("cm"),         1.00},
            {Types::UnitOfLengthInch,       QLatin1String("in"),         2.54},
            {Types::UnitOfLengthFeet,       QLatin1String("ft"), 2.54 * 12.00},
            {Types::UnitOfLengthPoints,     QLatin1String("pt"), 2.54 / 72.00}
        };
        static const int unitsOfLengthCount = sizeof(unitsOfLength)/sizeof(unitsOfLength[0]);
        for (int i = 0; i < unitsOfLengthCount; i++)
            units.insert(unitsOfLength[i].unit, QPair<QString, double> (unitsOfLength[i].name, unitsOfLength[i].cm));
    }
    return units;
}

double Types::convertBetweenUnitsOfLength(double distance, UnitsOfLength sourceUnit, UnitsOfLength targetUnit)
{
    double convertedDistance = distance;
    if (sourceUnit != targetUnit) {
        if (sourceUnit != UnitOfLengthCentimeter)
            convertedDistance *= unitsOfLength().value(sourceUnit).second;
        if (targetUnit != UnitOfLengthCentimeter)
            convertedDistance /= unitsOfLength().value(targetUnit).second;
    }
    return convertedDistance;
}

QSizeF Types::convertBetweenUnitsOfLength(const QSizeF &size, UnitsOfLength sourceUnit, UnitsOfLength targetUnit)
{
    return QSizeF(
        convertBetweenUnitsOfLength(size.width(), sourceUnit, targetUnit),
        convertBetweenUnitsOfLength(size.height(), sourceUnit, targetUnit)
    );
}

Types::UnitsOfLength Types::unitOfLenthFromString(const QString &string)
{
    static QHash<QString, UnitsOfLength> hash;
    if (hash.isEmpty()) {
        const QList<UnitsOfLength> units = unitsOfLength().keys();
        foreach (UnitsOfLength unit, units)
            hash.insert(unitsOfLength().value(unit).first, unit);
    }
    return hash.value(string);
}

const QHash<QString, QSizeF> &Types::paperFormats()
{
    static QHash<QString, QSizeF> formats;
    if (formats.empty()) {
        static const struct {
            QString name;
            double width;
            double height;
        } paperFormats[] = {
            {QLatin1String("DIN A4"),    21.0, 29.7},
            {QLatin1String("DIN A3"),    29.7, 42.0},
            {QLatin1String("Legal"),     21.6, 35.6},
            {QLatin1String("Letter"),    21.6, 27.9},
            {QLatin1String("Tabloid"),   27.9, 43.2}
        };
        static const int paperFormatsCount = (int)sizeof(paperFormats)/sizeof(paperFormats[0]);
        for (int i = 0; i < paperFormatsCount; i++)
            formats.insert(paperFormats[i].name, QSizeF(paperFormats[i].width, paperFormats[i].height));
    }
    return formats;
}

QSizeF Types::paperSize(const QString &format, QPageLayout::Orientation orientation, UnitsOfLength unit)
{
    QSizeF result = paperFormats().value(format);
    if (orientation == QPageLayout::Landscape)
        result.transpose();
    return convertBetweenUnitsOfLength(result, UnitOfLengthCentimeter, unit);
}

QString Types::cleanString(const QString &dirtyString)
{
    QString result = dirtyString;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    result.replace(QRegularExpression("[:&]"), "");
    result.replace(QRegularExpression("\\(.*\\)"), "");
#else
    result.replace(QRegExp("[:&]"), "");
    result.replace(QRegExp("\\(.*\\)"), "");
#endif
    result.replace('\n', ' ');
    return result.trimmed();
}

QString Types::newlineToParagraph(const QString &nlText)
{
    QString result = nlText;
    result.replace('\n', "</p><p>");
    return "<p>" + result + "</p>";
}
