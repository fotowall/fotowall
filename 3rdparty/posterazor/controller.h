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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QSettings>
#include <QAction>
#include "types.h"

class PosteRazorCore;
class WizardController;
class QTranslator;

class Controller: public QObject
{
    Q_OBJECT

public:
    Controller(PosteRazorCore *posteRazorCore, QWidget *view, QObject *parent = 0);

    void updateDialog();
    void updatePreview();

public slots:
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
    void setPosterWidthAbsolute(double width);
    void setPosterHeightAbsolute(double height);
    void setPosterWidthPages(double width);
    void setPosterHeightPages(double height);
    void setPosterSizePercentual(double percent);
    void setPosterSizeMode(Types::PosterSizeModes mode);
    void setPosterAlignment(Qt::Alignment alignment);
    void setLaunchPDFApplication(bool launch);
    void readSettings(const QSettings *settings);
    void writeSettings(QSettings *settings) const;
    void loadInputImage();
    bool loadInputImage(const QString &fileName);
    bool loadInputImage(const QString &fileName, QString &errorMessage);
    int savePoster(const QString &fileName) const;
    void savePoster() const;
    void loadTranslation(const QString &localeName);
    void setUnitOfLength(const QString &unit);
    void openPosteRazorWebsite();
    void setImageLoadingAvailable(bool available);
    void setPosterSizeModeAvailable(Types::PosterSizeModes mode, bool available);

signals:
    void previewChanged() const;
    void imageFileNameChanged(const QString &fileName) const;
    void imageInfoChanged(int imageWidthInPixels, int imageHeightInPixels, double imageWidth,
        double imageHeight, Types::UnitsOfLength unitOfLength, double verticalDpi, double horizontalDpi,
        Types::ColorTypes colorType, int bitsPerPixel) const;

    /* Privately used signals */
    void setPaperFormatSignal(const QString &format);
    void setPaperOrientationSignal(QPageLayout::Orientation orientation);
    void setPaperBorderTopSignal(double border);
    void setPaperBorderRightSignal(double border);
    void setPaperBorderBottomSignal(double border);
    void setPaperBorderLeftSignal(double border);
    void setCustomPaperSizeSignal(const QSizeF &size);
    void setUseCustomPaperSizeSignal(bool useIt);
    void setOverlappingWidthSignal(double width);
    void setOverlappingHeightSignal(double height);
    void setOverlappingPositionSignal(Qt::Alignment position);
    void setPosterWidthAbsoluteSignal(double width);
    void setPosterHeightAbsoluteSignal(double height);
    void setPosterWidthPagesSignal(double width);
    void setPosterHeightPagesSignal(double height);
    void setPosterSizePercentualSignal(double percent);
    void setPosterSizeModeSignal(Types::PosterSizeModes mode);
    void setPosterAlignmentSignal(Qt::Alignment alignment);
    void setLaunchPDFApplicationSignal(bool launch);
    void updatePreviewSignal();
    void showImageFileNameSignal(const QString &fileName);
    void updateImageInfoFieldsSignal(const QSize &inputImageSizeInPixels, const QSizeF &imageSize, double verticalDpi, double horizontalDpi, Types::ColorTypes colorType, int bitsPerPixel);
    void setCurrentTranslationSignal(const QString &translation);
    void addAboutDialogActionSignal(QAction *action);
    void readSettingsSignal(const QSettings *settings);
    void writeSettingsSignal(QSettings *settings) const;
    void setCurrentUnitOfLengthSignal(const QString &unit);
    void setPosterSavingEnabledSignal(bool enabled);
    void setPosterSizeModeAvailableSignal(Types::PosterSizeModes mode, bool available);

private slots:
    void showExtraAboutDialog();

protected:
    PosteRazorCore *m_posteRazorCore;
    QWidget *m_view;
    bool m_launchPDFApplication;
    WizardController *m_wizardController;
    QTranslator *m_translator;
    QString m_translationName;

    void setDialogSaveOptions();
    void setDialogPosterSizeMode();
    void setDialogPosterOptions();
    void setDialogPosterDimensions(Types::PosterSizeModes excludedMode, bool widthExcluded);
    void setDialogPaperOptions();
    void setDialogPaperBorders();
    void setDialogCustomPaperDimensions();
    void setDialogImageInfoFields();
    void setDialogOverlappingDimensions();
    void setDialogOverlappingOptions();
};

#endif // CONTROLLER_H
