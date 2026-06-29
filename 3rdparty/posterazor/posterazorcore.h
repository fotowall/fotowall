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

#ifndef POSTERAZORCORE_H
#define POSTERAZORCORE_H

#include "types.h"
#include "paintcanvasinterface.h"
#include <QObject>

class QSettings;
class ImageLoaderInterface;

class PosteRazorCore: public QObject
{
    Q_OBJECT

public:
    PosteRazorCore(ImageLoaderInterface *imageLoader, QObject *parent = 0);

    static unsigned int imageBitsPerLineCount(int widthPixels, int bitPerPixel);
    static unsigned int imageBytesPerLineCount(int widthPixels, int bitPerPixel);
    static unsigned int imageBytesCount(const QSize &size, int bitPerPixel);

    void readSettings(const QSettings *settings);
    void writeSettings(QSettings *settings) const;
    bool loadInputImage(const QString &imageFileName, QString &errorMessage);
    int savePoster(const QString &fileName) const;

    QSize inputImageSizePixels() const;
    double inputImageHorizontalDpi() const;
    double inputImageVerticalDpi() const;
    QSizeF inputImageSize() const;
    int inputImageBitsPerPixel() const;
    Types::ColorTypes inputImageColorType() const;
    Types::UnitsOfLength unitOfLength() const;
    const QString paperFormat() const;
    QPageLayout::Orientation paperOrientation() const;
    double paperBorderTop() const;
    double paperBorderRight() const;
    double paperBorderBottom() const;
    double paperBorderLeft() const;
    QSizeF customPaperSize() const;
    bool usesCustomPaperSize() const;
    QSizeF paperSize() const;
    QSizeF printablePaperAreaSize() const;
    double overlappingWidth() const;
    double overlappingHeight() const;
    Qt::Alignment overlappingPosition() const;
    QSizeF posterSize(Types::PosterSizeModes mode) const;
    Types::PosterSizeModes posterSizeMode() const;
    Qt::Alignment posterAlignment() const;
    bool isImageLoaded() const;
    const QVector<QPair<QStringList, QString> > &imageFormats() const;
    const QString imageIOLibraryName() const;
    const QString imageIOLibraryAboutText() const;

    void setUnitOfLength(Types::UnitsOfLength unit);
    void setPaperFormat(const QString &format);
    void setPaperOrientation(QPageLayout::Orientation orientation);
    void setPaperBorderTop(double border);
    void setPaperBorderRight(double border);
    void setPaperBorderBottom(double border);
    void setPaperBorderLeft(double border);
    void setCustomPaperWidth(double width);
    void setCustomPaperHeight(double height);
    void setUseCustomPaperSize(bool useIt);
    void setOverlappingWidth(double width);
    void setOverlappingHeight(double height);
    void setOverlappingPosition(Qt::Alignment position);
    void setPosterWidth(Types::PosterSizeModes mode, double width);
    void setPosterHeight(Types::PosterSizeModes mode, double height);
    void setPosterSizeMode(Types::PosterSizeModes mode);
    void setPosterAlignment(Qt::Alignment alignment);
    void createPreviewImage();

public slots:
    void paintOnCanvas(PaintCanvasInterface *paintCanvas, const QVariant &options) const;

private:
    double convertDistanceToCm(double distance) const;
    QSizeF convertSizeToCm(const QSizeF &size) const;
    double convertCmToDistance(double cm) const;
    QSizeF convertCmToSize(const QSizeF &sizeInCm) const;
    void createPreviewImage(const QSize &boxSize) const;
    double maximalVerticalPaperBorder() const;
    double maximalHorizontalPaperBorder() const;
    double convertBetweenAbsoluteAndPagesPosterDimension(double dimension, bool pagesToAbsolute, bool width) const;
    double calculateOtherPosterDimension() const;
    void setPosterDimension(Types::PosterSizeModes mode, double dimension, bool dimensionIsWidth);
    double maximalOverLappingWidth() const;
    double maximalOverLappingHeight() const;
    double posterDimension(Types::PosterSizeModes mode, bool width) const;
    QSizeF previewSize(const QSizeF &imageSize, const QSize &boxSize, bool enlargeToFit) const;
    QSizeF inputImagePreviewSize(const QSize &boxSize) const;
    void paintImageOnCanvas(PaintCanvasInterface *paintCanvas) const;
    void paintPaperOnCanvas(PaintCanvasInterface *paintCanvas, bool paintOverlapping) const;
    void paintPosterOnCanvasOverlapped(PaintCanvasInterface *paintCanvas) const;
    void paintPosterOnCanvasDivided(PaintCanvasInterface *paintCanvas) const;
    void paintPosterOnCanvasPageWise(PaintCanvasInterface *paintCanvas, int page) const;
    void paintPosterPageOnCanvas(PaintCanvasInterface *paintCanvas, int page) const;

signals:
    void previewImageChanged(const QImage &image) const;

private:
    ImageLoaderInterface*    m_imageLoader;
    Types::PosterSizeModes   m_posterSizeMode;
    double                   m_posterDimension;
    bool                     m_posterDimensionIsWidth;
    Qt::Alignment            m_posterAlignment;
    bool                     m_usesCustomPaperSize;
    QString                  m_paperFormat;
    QPageLayout::Orientation    m_paperOrientation;
    double                   m_paperBorderTop;
    double                   m_paperBorderRight;
    double                   m_paperBorderBottom;
    double                   m_paperBorderLeft;
    double                   m_customPaperWidth;
    double                   m_customPaperHeight;
    double                   m_overlappingWidth;
    double                   m_overlappingHeight;
    Qt::Alignment            m_overlappingPosition;
    Types::UnitsOfLength     m_unitOfLength;
};

#endif // POSTERAZORCORE_H
