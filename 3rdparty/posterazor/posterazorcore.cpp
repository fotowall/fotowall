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

#include "posterazorcore.h"
#include "pdfwriter.h"
#if defined (FREEIMAGE_LIB)
#    include "imageloaderfreeimage.h"
#else
#    include "imageloaderqt.h"
#endif
#include <QSettings>
#include <QStringList>
#include <QBrush>
#include <math.h>

#if defined(__EMSCRIPTEN__)
#include <QFile>
#include <QFileDialog>
#include <iostream>
#endif

const QLatin1String defaultValue_PaperFormat(           "DIN A4");

const QLatin1String settingsKey_PosterSizeMode(         "PosterSizeMode");
const QLatin1String settingsKey_PosterDimension(        "PosterDimension");
const QLatin1String settingsKey_PosterDimensionIsWidth( "PosterDimensionIsWidth");
const QLatin1String settingsKey_PosterAlignment(        "PosterAlignment");
const QLatin1String settingsKey_PaperFormat(            "PaperFormat");
const QLatin1String settingsKey_PaperOrientation(       "PaperOrientation");
const QLatin1String settingsKey_PaperBorderTop(         "PaperBorderTop");
const QLatin1String settingsKey_PaperBorderRight(       "PaperBorderRight");
const QLatin1String settingsKey_PaperBorderBottom(      "PaperBorderBottom");
const QLatin1String settingsKey_PaperBorderLeft(        "PaperBorderLeft");
const QLatin1String settingsKey_CustomPaperWidth(       "CustomPaperWidth");
const QLatin1String settingsKey_CustomPaperHeight(      "CustomPaperHeight");
const QLatin1String settingsKey_UseCustomPaperSize(     "UsesCustomPaperSize");
const QLatin1String settingsKey_OverlappingWidth(       "OverlappingWidth");
const QLatin1String settingsKey_OverlappingHeight(      "OverlappingHeight");
const QLatin1String settingsKey_OverlappingPosition(    "OverlappingPosition");
const QLatin1String settingsKey_UnitOfLength(           "UnitOfLength");

PosteRazorCore::PosteRazorCore(ImageLoaderInterface *imageLoader, QObject *parent)
    : QObject(parent)
    , m_imageLoader(imageLoader)
    , m_posterSizeMode(Types::PosterSizeModePages)
    , m_posterDimension(2.0)
    , m_posterDimensionIsWidth(true)
    , m_posterAlignment(Qt::AlignCenter)
    , m_usesCustomPaperSize(false)
    , m_paperFormat(defaultValue_PaperFormat)
    , m_paperOrientation(QPageLayout::Portrait)
    , m_paperBorderTop(1.5)
    , m_paperBorderRight(1.5)
    , m_paperBorderBottom(1.5)
    , m_paperBorderLeft(1.5)
    , m_customPaperWidth(20)
    , m_customPaperHeight(20)
    , m_overlappingWidth(1.0)
    , m_overlappingHeight(1.0)
    , m_overlappingPosition(Qt::AlignBottom | Qt::AlignRight)
    , m_unitOfLength(Types::UnitOfLengthCentimeter)
{
    Q_ASSERT(m_imageLoader);
}

unsigned int PosteRazorCore::imageBitsPerLineCount(int widthPixels, int bitPerPixel)
{
    return (widthPixels * bitPerPixel);
}

unsigned int PosteRazorCore::imageBytesPerLineCount(int widthPixels, int bitPerPixel)
{
    return (int)(ceil((double)imageBitsPerLineCount(widthPixels, bitPerPixel)/8.0f));
}

unsigned int PosteRazorCore::imageBytesCount(const QSize &size, int bitPerPixel)
{
    return imageBytesPerLineCount(size.width(), bitPerPixel) * size.height();
}

void PosteRazorCore::readSettings(const QSettings *settings)
{
    m_posterSizeMode               = (Types::PosterSizeModes)settings->value(settingsKey_PosterSizeMode, (int)m_posterSizeMode).toInt();
    m_posterDimension              = settings->value(settingsKey_PosterDimension, m_posterDimension).toDouble();
    m_posterDimensionIsWidth       = settings->value(settingsKey_PosterDimensionIsWidth, m_posterDimensionIsWidth).toBool();
    m_posterAlignment              = (Qt::Alignment)settings->value(settingsKey_PosterAlignment, (int)m_posterAlignment).toInt();
    m_usesCustomPaperSize           = settings->value(settingsKey_UseCustomPaperSize, m_usesCustomPaperSize).toBool();
    m_paperFormat                  = settings->value(settingsKey_PaperFormat, m_paperFormat).toString();
    if (!Types::paperFormats().contains(m_paperFormat))
        m_paperFormat = QLatin1String(defaultValue_PaperFormat);
    m_paperOrientation             = (QPageLayout::Orientation)settings->value(settingsKey_PaperOrientation, (int)m_paperOrientation).toInt();
    m_paperBorderTop               = settings->value(settingsKey_PaperBorderTop, m_paperBorderTop).toDouble();
    m_paperBorderRight             = settings->value(settingsKey_PaperBorderRight, m_paperBorderRight).toDouble();
    m_paperBorderBottom            = settings->value(settingsKey_PaperBorderBottom, m_paperBorderBottom).toDouble();
    m_paperBorderLeft              = settings->value(settingsKey_PaperBorderLeft, m_paperBorderLeft).toDouble();
    m_customPaperWidth             = settings->value(settingsKey_CustomPaperWidth, m_customPaperWidth).toDouble();
    m_customPaperHeight            = settings->value(settingsKey_CustomPaperHeight, m_customPaperHeight).toDouble();
    m_overlappingWidth             = settings->value(settingsKey_OverlappingWidth, m_overlappingWidth).toDouble();
    m_overlappingHeight            = settings->value(settingsKey_OverlappingHeight, m_overlappingHeight).toDouble();
    m_overlappingPosition          = (Qt::Alignment)settings->value(settingsKey_OverlappingPosition, (int)m_overlappingPosition).toInt();
    m_unitOfLength                 = (Types::UnitsOfLength)settings->value(settingsKey_UnitOfLength, (int)m_unitOfLength).toInt();
}

void PosteRazorCore::writeSettings(QSettings *settings) const
{
    settings->setValue(settingsKey_PosterSizeMode, (int)m_posterSizeMode);
    settings->setValue(settingsKey_PosterDimension, m_posterDimension);
    settings->setValue(settingsKey_PosterDimensionIsWidth, m_posterDimensionIsWidth);
    settings->setValue(settingsKey_PosterAlignment, (int)m_posterAlignment);
    settings->setValue(settingsKey_UseCustomPaperSize, m_usesCustomPaperSize);
    settings->setValue(settingsKey_PaperFormat, m_paperFormat);
    settings->setValue(settingsKey_PaperOrientation, (int)m_paperOrientation);
    settings->setValue(settingsKey_PaperBorderTop, m_paperBorderTop);
    settings->setValue(settingsKey_PaperBorderRight, m_paperBorderRight);
    settings->setValue(settingsKey_PaperBorderBottom, m_paperBorderBottom);
    settings->setValue(settingsKey_PaperBorderLeft, m_paperBorderLeft);
    settings->setValue(settingsKey_CustomPaperWidth, m_customPaperWidth);
    settings->setValue(settingsKey_CustomPaperHeight, m_customPaperHeight);
    settings->setValue(settingsKey_OverlappingWidth, m_overlappingWidth);
    settings->setValue(settingsKey_OverlappingHeight, m_overlappingHeight);
    settings->setValue(settingsKey_OverlappingPosition, (int)m_overlappingPosition);
    settings->setValue(settingsKey_UnitOfLength, (int)m_unitOfLength);
}

double PosteRazorCore::convertDistanceToCm(double distance) const
{
    return Types::convertBetweenUnitsOfLength(distance, unitOfLength(), Types::UnitOfLengthCentimeter);
}

QSizeF PosteRazorCore::convertSizeToCm(const QSizeF &size) const
{
    return QSizeF(convertDistanceToCm(size.width()), convertDistanceToCm(size.height()));
}

double PosteRazorCore::convertCmToDistance(double cm) const
{
    return Types::convertBetweenUnitsOfLength(cm, Types::UnitOfLengthCentimeter, unitOfLength());
}

QSizeF PosteRazorCore::convertCmToSize(const QSizeF &sizeInCm) const
{
    return QSizeF(convertCmToDistance(sizeInCm.width()), convertCmToDistance(sizeInCm.height()));
}

bool PosteRazorCore::loadInputImage(const QString &imageFileName, QString &errorMessage)
{
    const bool success = m_imageLoader->loadInputImage(imageFileName, errorMessage);
    if (success)
        createPreviewImage();
    return success;
}

bool PosteRazorCore::isImageLoaded() const
{
    return m_imageLoader->isImageLoaded();
}

const QVector<QPair<QStringList, QString> > &PosteRazorCore::imageFormats() const
{
    return m_imageLoader->imageFormats();
}

const QString PosteRazorCore::imageIOLibraryName() const
{
    return m_imageLoader->libraryName();
}

const QString PosteRazorCore::imageIOLibraryAboutText() const
{
    return m_imageLoader->libraryAboutText();
}

QSize PosteRazorCore::inputImageSizePixels() const
{
    return m_imageLoader->sizePixels();
}

double PosteRazorCore::inputImageHorizontalDpi() const
{
    return m_imageLoader->horizontalDotsPerUnitOfLength(Types::UnitOfLengthInch);
}

double PosteRazorCore::inputImageVerticalDpi() const
{
    return m_imageLoader->verticalDotsPerUnitOfLength(Types::UnitOfLengthInch);
}

QSizeF PosteRazorCore::inputImageSize() const
{
    return m_imageLoader->size(m_unitOfLength);
}

int PosteRazorCore::inputImageBitsPerPixel() const
{
    return m_imageLoader->bitsPerPixel();
}

Types::ColorTypes PosteRazorCore::inputImageColorType() const
{
    return m_imageLoader->colorDataType();
}

void PosteRazorCore::setUnitOfLength(Types::UnitsOfLength unit)
{
    m_unitOfLength = unit;
}

Types::UnitsOfLength PosteRazorCore::unitOfLength() const
{
    return m_unitOfLength;
}

void PosteRazorCore::setPaperFormat(const QString &format)
{
    m_paperFormat = format;
}

void PosteRazorCore::setPaperOrientation(QPageLayout::Orientation orientation)
{
    m_paperOrientation = orientation;
}

void PosteRazorCore::setPaperBorderTop(double border)
{
    m_paperBorderTop = convertDistanceToCm(border);
}

void PosteRazorCore::setPaperBorderRight(double border)
{
    m_paperBorderRight = convertDistanceToCm(border);
}

void PosteRazorCore::setPaperBorderBottom(double border)
{
    m_paperBorderBottom = convertDistanceToCm(border);
}

void PosteRazorCore::setPaperBorderLeft(double border)
{
    m_paperBorderLeft = convertDistanceToCm(border);
}

const QString PosteRazorCore::paperFormat() const
{
    return m_paperFormat;
}

QPageLayout::Orientation PosteRazorCore::paperOrientation() const
{
    return m_paperOrientation;
}

double PosteRazorCore::paperBorderTop() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_paperBorderTop), (qreal)maximalHorizontalPaperBorder());
}

double PosteRazorCore::paperBorderRight() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_paperBorderRight), (qreal)maximalVerticalPaperBorder());
}

double PosteRazorCore::paperBorderBottom() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_paperBorderBottom), (qreal)maximalHorizontalPaperBorder());
}

double PosteRazorCore::paperBorderLeft() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_paperBorderLeft), (qreal)maximalVerticalPaperBorder());
}

double PosteRazorCore::maximalVerticalPaperBorder() const
{
    return paperSize().width() / 2.0 - convertCmToDistance(1.0);
}

double PosteRazorCore::maximalHorizontalPaperBorder() const
{
    return paperSize().height() / 2.0 - convertCmToDistance(1.0);
}

void PosteRazorCore::setCustomPaperWidth(double width)
{
    m_customPaperWidth = convertDistanceToCm(width);
}

void PosteRazorCore::setCustomPaperHeight(double height)
{
    m_customPaperHeight = convertDistanceToCm(height);
}

QSizeF PosteRazorCore::customPaperSize() const
{
    const double minimalPaperWidth = 4.0;
    const double minimalPaperHeight = minimalPaperWidth;
    const double maximalPaperWidth = 500.0; // 5 meter = maximum of a PDF page.
    const double maximalPaperHeight = maximalPaperWidth;
    return QSizeF(
        convertCmToDistance(qBound(minimalPaperWidth, m_customPaperWidth, maximalPaperWidth)),
        convertCmToDistance(qBound(minimalPaperHeight, m_customPaperHeight, maximalPaperHeight))
    );
}

void PosteRazorCore::setUseCustomPaperSize(bool useIt)
{
    m_usesCustomPaperSize = useIt;
}

bool PosteRazorCore::usesCustomPaperSize() const
{
    return m_usesCustomPaperSize;
}

QSizeF PosteRazorCore::paperSize() const
{
    return usesCustomPaperSize()?customPaperSize()
        :Types::paperSize(paperFormat(), paperOrientation(), m_unitOfLength);
}

QSizeF PosteRazorCore::printablePaperAreaSize() const
{
    return QSizeF(
        paperSize().width() - paperBorderLeft() - paperBorderRight(),
        paperSize().height() - paperBorderTop() - paperBorderBottom()
    );
}

double PosteRazorCore::convertBetweenAbsoluteAndPagesPosterDimension(double dimension, bool pagesToAbsolute, bool width) const
{
    double posterDimension = dimension;

    const QSizeF printablePaperAreaSize = this->printablePaperAreaSize();
    const double printablePaperAreaDimension = convertDistanceToCm(width?printablePaperAreaSize.width():printablePaperAreaSize.height());
    const double overlappingDimension = convertDistanceToCm(width?overlappingWidth():overlappingHeight());

    if (pagesToAbsolute) {
        double posterDimensionAbsolute = 0;
        if (posterDimension >= 1.0) {
            posterDimension -= 1.0;
            posterDimensionAbsolute += printablePaperAreaDimension;
        } else {
            posterDimensionAbsolute = posterDimension * printablePaperAreaDimension;
            posterDimension = 0;
        }

        if (posterDimension > 0)
            posterDimensionAbsolute += (posterDimension * (printablePaperAreaDimension - overlappingDimension));

        posterDimension = posterDimensionAbsolute;
    } else {
        double posterDimensionPages = 0;
        if (posterDimension >= printablePaperAreaDimension) {
            posterDimension -= printablePaperAreaDimension;
            posterDimensionPages += 1.0;
        } else if (posterDimension < printablePaperAreaDimension) {
            posterDimensionPages = posterDimension / printablePaperAreaDimension;
            posterDimension = 0;
        }

        if (posterDimension > 0)
            posterDimensionPages += (posterDimension / (printablePaperAreaDimension - overlappingDimension));

        posterDimension = posterDimensionPages;
    }

    return posterDimension;
}

// Since m_posterDimension holds only height or width of the poster,
// here we calculate the other dimension considering the aspect ratio.
double PosteRazorCore::calculateOtherPosterDimension() const
{
    double otherDimension = 0;

    if (posterSizeMode() != Types::PosterSizeModePercentual) {
        const QSizeF inputImageSize = this->inputImageSize();
        const double sourceReference = m_posterDimensionIsWidth?inputImageSize.width():inputImageSize.height();
        const double targetReference = m_posterDimensionIsWidth?inputImageSize.height():inputImageSize.width();
        const double aspectRatio = sourceReference/targetReference;

        if (posterSizeMode() != Types::PosterSizeModePages) {
            otherDimension = m_posterDimension / aspectRatio;
        } else {
            const double sourceAbsolute = convertBetweenAbsoluteAndPagesPosterDimension(m_posterDimension, true, m_posterDimensionIsWidth);
            const double targetAbsolute = sourceAbsolute/aspectRatio;
            otherDimension = convertBetweenAbsoluteAndPagesPosterDimension(targetAbsolute, false, !m_posterDimensionIsWidth);
        }
    }
    else
        otherDimension = m_posterDimension;

    return otherDimension;
}

void PosteRazorCore::setPosterDimension(Types::PosterSizeModes mode, double dimension, bool dimensionIsWidth)
{
    setPosterSizeMode(mode);

    if (posterSizeMode() == Types::PosterSizeModeAbsolute)
        dimension = convertDistanceToCm(dimension);

    m_posterDimension = dimension;
    m_posterDimensionIsWidth = dimensionIsWidth;
}

void PosteRazorCore::setOverlappingWidth(double width)
{
    m_overlappingWidth = convertDistanceToCm(width);
}

void PosteRazorCore::setOverlappingHeight(double height)
{
    m_overlappingHeight = convertDistanceToCm(height);
}

double PosteRazorCore::overlappingWidth() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_overlappingWidth), (qreal)maximalOverLappingWidth());
}

double PosteRazorCore::overlappingHeight() const
{
    return qBound((qreal).0, (qreal)convertCmToDistance(m_overlappingHeight), (qreal)maximalOverLappingHeight());
}

double PosteRazorCore::maximalOverLappingWidth() const
{
    return paperSize().width() - paperBorderLeft() - paperBorderRight() - convertCmToDistance(1.0);
}

double PosteRazorCore::maximalOverLappingHeight() const
{
    return paperSize().height() - paperBorderTop() - paperBorderBottom() - convertCmToDistance(1.0);
}

void PosteRazorCore::setOverlappingPosition(Qt::Alignment position)
{
    m_overlappingPosition = position;
}

Qt::Alignment PosteRazorCore::overlappingPosition() const
{
    return m_overlappingPosition;
}

void PosteRazorCore::setPosterWidth(Types::PosterSizeModes mode, double width)
{
    setPosterDimension(mode, width, true);
}

void PosteRazorCore::setPosterHeight(Types::PosterSizeModes mode, double height)
{
    setPosterDimension(mode, height, false);
}

void PosteRazorCore::setPosterSizeMode(Types::PosterSizeModes mode)
{
    m_posterSizeMode = mode;
}

double PosteRazorCore::posterDimension(Types::PosterSizeModes mode, bool width) const
{
    double posterDimension = (width==m_posterDimensionIsWidth)?m_posterDimension:calculateOtherPosterDimension();

    posterDimension = qMax(
        (mode == Types::PosterSizeModeAbsolute)?0.001
        :(mode == Types::PosterSizeModePages)?0.001
        :0.001
        , posterDimension
    );

    // anything to convert?
    if (posterSizeMode() != mode){
        // These are needed for conversion from and to PosterSizeModePercentual
        const QSizeF inputImageSize = this->inputImageSize();
        const double inputImageDimension = convertDistanceToCm(width?inputImageSize.width():inputImageSize.height());

        // First convert to absolute size mode (cm)
        if (posterSizeMode() == Types::PosterSizeModePages) {
            posterDimension = convertBetweenAbsoluteAndPagesPosterDimension(posterDimension, true, width);
        } else if (posterSizeMode() == Types::PosterSizeModePercentual) {
            posterDimension *= inputImageDimension;
            posterDimension /= 100.0;
        }

        // Then convert to the wanted size mode
        if (mode == Types::PosterSizeModePages) {
            posterDimension = convertBetweenAbsoluteAndPagesPosterDimension(posterDimension, false, width);
        } else if (mode == Types::PosterSizeModePercentual) {
            posterDimension /= inputImageDimension;
            posterDimension *= 100.0;
        }
    }

    if (mode == Types::PosterSizeModeAbsolute)
        posterDimension = convertCmToDistance(posterDimension);

    return posterDimension;
}

QSizeF PosteRazorCore::posterSize(Types::PosterSizeModes mode) const
{
    return QSizeF(posterDimension(mode, true), posterDimension(mode, false));
}

Types::PosterSizeModes PosteRazorCore::posterSizeMode() const
{
    return m_posterSizeMode;
}

void PosteRazorCore::setPosterAlignment(Qt::Alignment alignment)
{
    m_posterAlignment = alignment;
}

void PosteRazorCore::createPreviewImage()
{
    createPreviewImage(QSize(1024, 768));
}

Qt::Alignment PosteRazorCore::posterAlignment() const
{
    return m_posterAlignment;
}

QSizeF PosteRazorCore::previewSize(const QSizeF &imageSize, const QSize &boxSize, bool enlargeToFit) const
{
    QSizeF result(imageSize);

    QSizeF boundedBoxSize(boxSize);
    if (!enlargeToFit)
        boundedBoxSize = boundedBoxSize.boundedTo(imageSize);

    result.scale(boundedBoxSize, Qt::KeepAspectRatio);
    return result;
}

QSizeF PosteRazorCore::inputImagePreviewSize(const QSize &boxSize) const
{
    return previewSize(inputImageSizePixels(), boxSize, false);
}

void PosteRazorCore::createPreviewImage(const QSize &size) const
{
    const QImage previewImage = m_imageLoader->imageAsRGB(inputImagePreviewSize(size).toSize());
    emit previewImageChanged(previewImage);
}

void PosteRazorCore::paintImageOnCanvas(PaintCanvasInterface *paintCanvas) const
{
    if (isImageLoaded()) {
        const QSizeF canvasSize = paintCanvas->size();
        const QSize inputImageSize = inputImageSizePixels();
        const QSizeF boxSize = previewSize(inputImageSize, canvasSize.toSize(), false);
        QPointF offset((canvasSize.width() - boxSize.width()) / 2, (canvasSize.height() - boxSize.height()) / 2);

        // If the image is not downscaled, make sure that the coordinates are integers in order
        // to prevent unneeded blurring
        if (canvasSize.width() >= inputImageSize.width() && canvasSize.height() >= inputImageSize.height()) {
            offset.setX(floor(offset.x()));
            offset.setY(floor(offset.y()));
        }

        paintCanvas->drawImage(QRectF(offset, boxSize));
    }
}

void PosteRazorCore::paintPaperOnCanvas(PaintCanvasInterface *paintCanvas, bool paintOverlapping) const
{
    const QSizeF canvasSize = paintCanvas->size();
    const QSizeF paperSize = this->paperSize();
    const QSizeF boxSize = previewSize(paperSize, canvasSize.toSize(), true);
    const QPointF offset((canvasSize.width() - boxSize.width()) / 2.0, (canvasSize.height() - boxSize.height()) / 2.0);
    const double UnitOfLengthToPixelfactor = boxSize.width()/paperSize.width();
    const double borderTop = paperBorderTop() * UnitOfLengthToPixelfactor;
    const double borderRight = paperBorderRight() * UnitOfLengthToPixelfactor;
    const double borderBottom = paperBorderBottom() * UnitOfLengthToPixelfactor;
    const double borderLeft = paperBorderLeft() * UnitOfLengthToPixelfactor;
    const QSizeF printableAreaSize(boxSize.width() - borderLeft - borderRight, boxSize.height() - borderTop - borderBottom);

    paintCanvas->drawFilledRect(QRectF(offset, boxSize), QColor(128, 128, 128));
    paintCanvas->drawFilledRect(QRectF(QPointF(borderLeft, borderTop) + offset, printableAreaSize), QColor(230, 230, 230));

    if (paintOverlapping) {
        const double overlappingWidth = this->overlappingWidth() * UnitOfLengthToPixelfactor;
        const double overlappingHeight = this->overlappingHeight() * UnitOfLengthToPixelfactor;
        const Qt::Alignment overlappingPosition = this->overlappingPosition();
        const double overlappingTop = (overlappingPosition & Qt::AlignTop)?
            borderTop:boxSize.height() - borderBottom - overlappingHeight;
        const double overlappingLeft = (overlappingPosition & Qt::AlignLeft)?
            borderLeft:boxSize.width() - borderRight - overlappingWidth;

        const QColor overlappingBrush(255, 128, 128);
        paintCanvas->drawFilledRect(QRectF(QPointF(borderLeft, overlappingTop) + offset, QSizeF(printableAreaSize.width(), overlappingHeight)), overlappingBrush);
        paintCanvas->drawFilledRect(QRectF(QPointF(overlappingLeft, borderTop) + offset, QSizeF(overlappingWidth, printableAreaSize.height())), overlappingBrush);
    }
}

void PosteRazorCore::paintPosterOnCanvasOverlapped(PaintCanvasInterface *paintCanvas) const
{
    const QSizeF canvasSize = paintCanvas->size();
    QSizeF pagePrintableAreaSize = printablePaperAreaSize();
    const QSizeF posterSizePages = posterSize(Types::PosterSizeModePages);
    const int pagesHorizontal = (int)ceil(posterSizePages.width());
    const int pagesVertical = (int)ceil(posterSizePages.height());
    const QSizeF posterSize(
        pagesHorizontal*pagePrintableAreaSize.width() - (pagesHorizontal-1)*overlappingWidth() + paperBorderLeft() + paperBorderRight(),
        pagesVertical*pagePrintableAreaSize.height() - (pagesVertical-1)*overlappingHeight() + paperBorderTop() + paperBorderBottom()
    );
    const QSizeF boxSize = previewSize(posterSize, canvasSize.toSize(), true);
    const QPointF offset((canvasSize.width() - boxSize.width()) / 2, (canvasSize.height() - boxSize.height()) / 2);
    const double UnitOfLengthToPixelfactor = boxSize.width()/posterSize.width();

    const double borderTop = paperBorderTop() * UnitOfLengthToPixelfactor;
    const double borderRight = paperBorderRight() * UnitOfLengthToPixelfactor;
    const double borderBottom = paperBorderBottom() * UnitOfLengthToPixelfactor;
    const double borderLeft = paperBorderLeft() * UnitOfLengthToPixelfactor;
    const QSizeF posterPrintableAreaSize(boxSize.width() - borderLeft - borderRight, boxSize.height() - borderTop - borderBottom);
    const QPointF posterPrintableAreaOrigin = QPointF(borderLeft, borderTop) + offset;
    const QRectF posterPrintableArea(posterPrintableAreaOrigin, posterPrintableAreaSize);

    paintCanvas->drawFilledRect(QRectF(offset, boxSize), QColor(128, 128, 128));
    paintCanvas->drawFilledRect(posterPrintableArea, QColor(230, 230, 230));

    const QSizeF posterSizeAbsolute = this->posterSize(Types::PosterSizeModeAbsolute);
    const QSizeF imageSize = posterSizeAbsolute * UnitOfLengthToPixelfactor;
    const Qt::Alignment alignment = posterAlignment();

    paintCanvas->drawImage(
        QRectF(
            QPointF(
                (
                    alignment & Qt::AlignLeft?borderLeft
                    :alignment & Qt::AlignHCenter?qBound((qreal)borderLeft, (boxSize.width() - imageSize.width()) / 2, (qreal)borderLeft + (qreal)posterPrintableAreaSize.width() - (qreal)imageSize.width())
                    :(borderLeft + posterPrintableAreaSize.width() - imageSize.width())
                ) + offset.x(),
                (
                    alignment & Qt::AlignTop?borderTop
                    :alignment & Qt::AlignVCenter?qBound((qreal)borderTop, (boxSize.height() - imageSize.height()) / 2, (qreal)borderTop + (qreal)posterPrintableAreaSize.height() - (qreal)imageSize.height())
                    :(borderTop + posterPrintableAreaSize.height() - imageSize.height())
                ) + offset.y()
            ),
            imageSize
        )
    );

    const double overlappingHeight = this->overlappingHeight() * UnitOfLengthToPixelfactor;
    const double overlappingWidth = this->overlappingWidth() * UnitOfLengthToPixelfactor;
    pagePrintableAreaSize *= UnitOfLengthToPixelfactor;

    const QColor overlappingColor(255, 128, 128, 128);
    double overlappingRectangleYPosition = borderTop;
    for (int pagesRow = 0; pagesRow < pagesVertical - 1; pagesRow++) {
        overlappingRectangleYPosition += pagePrintableAreaSize.height() - overlappingHeight;
        paintCanvas->drawFilledRect(QRectF(QPointF(0, overlappingRectangleYPosition) + offset, QSizeF(boxSize.width(), overlappingHeight)), overlappingColor);
    }

    double overlappingRectangleXPosition = borderLeft;
    for (int pagesColumn = 0; pagesColumn < pagesHorizontal - 1; pagesColumn++) {
        overlappingRectangleXPosition += pagePrintableAreaSize.width() - overlappingWidth;
        paintCanvas->drawFilledRect(QRectF(QPointF(overlappingRectangleXPosition, 0) + offset, QSizeF(overlappingWidth, boxSize.height())), overlappingColor);
    }

    const int fontSize = int(qMin(pagePrintableAreaSize.width() / 2.5, pagePrintableAreaSize.height() / 1.5));
    for (int pagesRow = 0; pagesRow < pagesVertical; ++pagesRow) {
        for (int pagesColumn = 0; pagesColumn < pagesHorizontal; ++pagesColumn) {
            const QPointF pagePrintableAreaOrigin = posterPrintableAreaOrigin + QPointF(
                pagesColumn * (pagePrintableAreaSize.width() - overlappingWidth),
                pagesRow * (pagePrintableAreaSize.height() - overlappingHeight)
            );
            const QRectF pageNumberArea = QRectF(pagePrintableAreaOrigin, pagePrintableAreaSize).adjusted(
                pagesColumn == 0 ? 0 : overlappingWidth,
                pagesRow == 0 ? 0 : overlappingHeight,
                pagesColumn == pagesHorizontal - 1 ? 0 : -overlappingWidth,
                pagesRow == pagesVertical - 1 ? 0 : -overlappingHeight
            );
            const int pageNumber = pagesRow * pagesHorizontal + pagesColumn + 1;
            paintCanvas->drawOverlayText(pageNumberArea.center(), Qt::AlignCenter, fontSize, QString::number(pageNumber));
        }
    }
}

void PosteRazorCore::paintPosterOnCanvasDivided(PaintCanvasInterface *paintCanvas) const
{
    Q_UNUSED(paintCanvas)
}

void PosteRazorCore::paintPosterOnCanvasPageWise(PaintCanvasInterface *paintCanvas, int page) const
{
    Q_UNUSED(paintCanvas)
    Q_UNUSED(page)
}

void PosteRazorCore::paintPosterPageOnCanvas(PaintCanvasInterface *paintCanvas, int page) const
{
    const QSizeF posterSizePages = posterSize(Types::PosterSizeModePages);
    const int columsCount = (int)(ceil(posterSizePages.width()));
    const int rowsCount = (int)(ceil(posterSizePages.height()));
    const int column = page % columsCount;
    const int row = page / columsCount;

    const QSizeF posterSizeAbsolute = posterSize(Types::PosterSizeModeAbsolute);
    const QSizeF posterImageSizeCm = convertSizeToCm(posterSizeAbsolute);
    const QSizeF printablePaperAreaSizeCm = convertSizeToCm(printablePaperAreaSize());
    const double overlappingWidthCm = convertDistanceToCm(overlappingWidth());
    const double overlappingHeightCm = convertDistanceToCm(overlappingHeight());
    const QSizeF printablePosterAreaSizeCm(
        columsCount * printablePaperAreaSizeCm.width() - (columsCount - 1) * overlappingWidthCm,
        rowsCount * printablePaperAreaSizeCm.height() - (rowsCount - 1) * overlappingHeightCm
    );
    const double borderTopCm = convertDistanceToCm(paperBorderTop());
    const double borderRightCm = convertDistanceToCm(paperBorderRight());
    const double borderBottomCm = convertDistanceToCm(paperBorderBottom());
    const double borderLeftCm = convertDistanceToCm(paperBorderLeft());
    const QSizeF posterTotalSizeCm(
        printablePosterAreaSizeCm.width() + borderLeftCm + borderRightCm,
        printablePosterAreaSizeCm.height() + borderTopCm + borderBottomCm
    );
    const Qt::Alignment alignment = posterAlignment();
    double imageOffsetFromLeftPosterBorderCm = (
        alignment & Qt::AlignRight?posterTotalSizeCm.width() - posterImageSizeCm.width() - borderLeftCm
        :alignment & Qt::AlignHCenter?(posterTotalSizeCm.width() - posterImageSizeCm.width())/2 - borderLeftCm
        :-borderLeftCm
    );
    imageOffsetFromLeftPosterBorderCm = qBound((qreal).0, (qreal)imageOffsetFromLeftPosterBorderCm, (qreal)posterTotalSizeCm.width() - (qreal)posterImageSizeCm.width() - (qreal)borderLeftCm - (qreal)borderRightCm);
    double imageOffsetFromTopPosterBorderCm = (
        alignment & Qt::AlignBottom?posterTotalSizeCm.height() - posterImageSizeCm.height() - borderTopCm
        :alignment & Qt::AlignVCenter?(posterTotalSizeCm.height() - posterImageSizeCm.height())/2 - borderTopCm
        :-borderTopCm
    );
    imageOffsetFromTopPosterBorderCm = qBound((qreal).0, (qreal)imageOffsetFromTopPosterBorderCm, (qreal)posterTotalSizeCm.height() - (qreal)posterImageSizeCm.height() - (qreal)borderTopCm - (qreal)borderBottomCm);
    const QPointF pageOffsetToImageFromTopLeftCm(
        column * (printablePaperAreaSizeCm.width()- overlappingWidthCm) - imageOffsetFromLeftPosterBorderCm,
        row * (printablePaperAreaSizeCm.height() - overlappingHeightCm) - imageOffsetFromTopPosterBorderCm
    );
    paintCanvas->drawImage(QRectF(-pageOffsetToImageFromTopLeftCm, posterImageSizeCm));
}

void PosteRazorCore::paintOnCanvas(PaintCanvasInterface *paintCanvas, const QVariant &options) const
{
    const QString state = options.toString();

    if (state == QLatin1String("image")) {
        paintImageOnCanvas(paintCanvas);
    } else if (state == QLatin1String("paper") || state == QLatin1String("overlapping")) {
        paintPaperOnCanvas(paintCanvas, state == QLatin1String("overlapping"));
    } else if (state.startsWith(QLatin1String("posterpage"))) {
        const int page = state.split(' ').last().toInt();
        paintPosterPageOnCanvas(paintCanvas, page);
    } else if (state.startsWith(QLatin1String("poster"))) {
        const QStringList options = state.split(QLatin1Char(' '));
        if (options.at(1) == QLatin1String("overlapped")) {
            paintPosterOnCanvasOverlapped(paintCanvas);
        } else if (options.at(1) == QLatin1String("divided")) {
            paintPosterOnCanvasDivided(paintCanvas);
        } else if (options.at(1) == QLatin1String("pagewise")) {
            const int page = options.at(2).toInt();
            paintPosterOnCanvasPageWise(paintCanvas, page);
        } else {
            qFatal("Unimplemented poster mode in PosteRazorCore::paintOnCanvas().");
        }
    } else {
        qFatal("Unimplemented state in PosteRazorCore::paintOnCanvas().");
    }
}

int PosteRazorCore::savePoster(const QString &fileName) const
{
    int err = 0;

    const QSizeF posterSizePages = posterSize(Types::PosterSizeModePages);
    const QSizeF sizeCm = convertSizeToCm(printablePaperAreaSize());
    const int pagesCount = (int)(ceil(posterSizePages.width())) * (int)(ceil(posterSizePages.height()));
    const QSize imageSize = m_imageLoader->sizePixels();
    const QByteArray imageData = m_imageLoader->bits();

    PDFWriter pdfWriter;
    err = pdfWriter.startSaving(fileName, pagesCount, sizeCm.width(), sizeCm.height());
    if (!err) {
        if (m_imageLoader->isJpeg())
            err = pdfWriter.saveJpegImage(m_imageLoader->fileName(), imageSize, m_imageLoader->colorDataType());
        else
            err = pdfWriter.saveImage(imageData, imageSize, m_imageLoader->bitsPerPixel(), m_imageLoader->colorDataType(), m_imageLoader->colorTable());
    }

    if (!err) {
        for (int page = 0; page < pagesCount; page++) {
            pdfWriter.startPage();
            paintOnCanvas(&pdfWriter, QString(QLatin1String("posterpage %1")).arg(page));
            pdfWriter.finishPage();
        }
        err = pdfWriter.finishSaving();
    }

#if defined(__EMSCRIPTEN__)
  QFile pdfFile("fotowall_poster.pdf"); // Read PDF stored in local browser storage
  pdfFile.open(QIODevice::ReadOnly);
  QByteArray content = pdfFile.readAll();
  std::cout << "Content size is " << content.size() << std::endl;
  pdfFile.remove();

  QFileDialog::saveFileContent(content, "fotowall_poster.pdf"); // download pdf
#endif

    return err;
}
