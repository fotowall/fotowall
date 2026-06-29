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

#include "controller.h"
#include "posterazorcore.h"
#include "wizardcontroller.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>
#include <QUrl>
#include <QStandardPaths>

const QLatin1String settingsKey_LaunchPDFApplication("LaunchPDFApplication");
const QLatin1String settingsKey_TranslationName("TranslationName");
const QLatin1String settingsKey_ImageLoadPath("ImageLoadPath");
const QLatin1String settingsKey_PosterSavePath("PosterSavePath");

Controller::Controller(PosteRazorCore *posteRazorCore, QWidget *view, QObject *parent)
    : QObject(parent)
    , m_posteRazorCore(posteRazorCore)
    , m_view(view)
    , m_launchPDFApplication(true)
{
    connect(m_view, SIGNAL(paperFormatChanged(const QString&)), SLOT(setPaperFormat(const QString&)));
    connect(m_view, SIGNAL(paperOrientationChanged(QPageLayout::Orientation)), SLOT(setPaperOrientation(QPageLayout::Orientation)));
    connect(m_view, SIGNAL(paperBorderTopChanged(double)), SLOT(setPaperBorderTop(double)));
    connect(m_view, SIGNAL(paperBorderRightChanged(double)), SLOT(setPaperBorderRight(double)));
    connect(m_view, SIGNAL(paperBorderBottomChanged(double)), SLOT(setPaperBorderBottom(double)));
    connect(m_view, SIGNAL(paperBorderLeftChanged(double)), SLOT(setPaperBorderLeft(double)));
    connect(m_view, SIGNAL(useCustomPaperSizeChanged(bool)), SLOT(setUseCustomPaperSize(bool)));
    connect(m_view, SIGNAL(paperCustomWidthChanged(double)), SLOT(setCustomPaperWidth(double)));
    connect(m_view, SIGNAL(paperCustomHeightChanged(double)), SLOT(setCustomPaperHeight(double)));
    connect(m_view, SIGNAL(overlappingWidthChanged(double)), SLOT(setOverlappingWidth(double)));
    connect(m_view, SIGNAL(overlappingHeightChanged(double)), SLOT(setOverlappingHeight(double)));
    connect(m_view, SIGNAL(overlappingPositionChanged(Qt::Alignment)), SLOT(setOverlappingPosition(Qt::Alignment)));
    connect(m_view, SIGNAL(posterWidthAbsoluteChanged(double)), SLOT(setPosterWidthAbsolute(double)));
    connect(m_view, SIGNAL(posterHeightAbsoluteChanged(double)), SLOT(setPosterHeightAbsolute(double)));
    connect(m_view, SIGNAL(posterWidthPagesChanged(double)), SLOT(setPosterWidthPages(double)));
    connect(m_view, SIGNAL(posterHeightPagesChanged(double)), SLOT(setPosterHeightPages(double)));
    connect(m_view, SIGNAL(posterSizePercentualChanged(double)), SLOT(setPosterSizePercentual(double)));
    connect(m_view, SIGNAL(posterAlignmentChanged(Qt::Alignment)), SLOT(setPosterAlignment(Qt::Alignment)));
    connect(m_view, SIGNAL(savePosterSignal()), SLOT(savePoster()));
    connect(m_view, SIGNAL(launchPDFApplicationChanged(bool)), SLOT(setLaunchPDFApplication(bool)));
    connect(m_view, SIGNAL(loadImageSignal()), SLOT(loadInputImage()));
    connect(m_view, SIGNAL(translationChanged(const QString&)), SLOT(loadTranslation(const QString&)));
    connect(m_view, SIGNAL(unitOfLengthChanged(const QString&)), SLOT(setUnitOfLength(const QString&)));
    connect(m_view, SIGNAL(openPosteRazorWebsiteSignal()), SLOT(openPosteRazorWebsite()));
    connect(m_view, SIGNAL(needsPaint(PaintCanvasInterface*, const QVariant&)), m_posteRazorCore, SLOT(paintOnCanvas(PaintCanvasInterface*, const QVariant&)));
    connect(m_posteRazorCore, SIGNAL(previewImageChanged(const QImage&)), m_view, SLOT(setPreviewImage(const QImage&)));

    static const struct {
        const char *signal;
        const char *slot;
    } signalsToViewSlotsConnections[] = {
        {SIGNAL(setPaperFormatSignal(const QString&)),              SLOT(setPaperFormat(const QString&))},
        {SIGNAL(setPaperFormatSignal(const QString&)),              SLOT(setPaperFormat(const QString&))},
        {SIGNAL(setPaperOrientationSignal(QPageLayout::Orientation)),  SLOT(setPaperOrientation(QPageLayout::Orientation))},
        {SIGNAL(setPaperBorderTopSignal(double)),                   SLOT(setPaperBorderTop(double))},
        {SIGNAL(setPaperBorderRightSignal(double)),                 SLOT(setPaperBorderRight(double))},
        {SIGNAL(setPaperBorderBottomSignal(double)),                SLOT(setPaperBorderBottom(double))},
        {SIGNAL(setPaperBorderLeftSignal(double)),                  SLOT(setPaperBorderLeft(double))},
        {SIGNAL(setCustomPaperSizeSignal(const QSizeF&)),           SLOT(setCustomPaperSize(const QSizeF&))},
        {SIGNAL(setUseCustomPaperSizeSignal(bool)),                 SLOT(setUseCustomPaperSize(bool))},
        {SIGNAL(setOverlappingWidthSignal(double)),                 SLOT(setOverlappingWidth(double))},
        {SIGNAL(setOverlappingHeightSignal(double)),                SLOT(setOverlappingHeight(double))},
        {SIGNAL(setOverlappingPositionSignal(Qt::Alignment)),       SLOT(setOverlappingPosition(Qt::Alignment))},
        {SIGNAL(setPosterWidthAbsoluteSignal(double)),              SLOT(setPosterWidthAbsolute(double))},
        {SIGNAL(setPosterHeightAbsoluteSignal(double)),             SLOT(setPosterHeightAbsolute(double))},
        {SIGNAL(setPosterWidthPagesSignal(double)),                 SLOT(setPosterWidthPages(double))},
        {SIGNAL(setPosterHeightPagesSignal(double)),                SLOT(setPosterHeightPages(double))},
        {SIGNAL(setPosterSizePercentualSignal(double)),             SLOT(setPosterSizePercentual(double))},
        {SIGNAL(setPosterSizeModeSignal(Types::PosterSizeModes)),   SLOT(setPosterSizeMode(Types::PosterSizeModes))},
        {SIGNAL(setPosterAlignmentSignal(Qt::Alignment)),           SLOT(setPosterAlignment(Qt::Alignment))},
        {SIGNAL(setLaunchPDFApplicationSignal(bool)),               SLOT(setLaunchPDFApplication(bool))},
        {SIGNAL(updatePreviewSignal()),                             SLOT(updatePreview())},
        {SIGNAL(showImageFileNameSignal(const QString&)),           SLOT(showImageFileName(const QString&))},
        {SIGNAL(updateImageInfoFieldsSignal(const QSize&, const QSizeF&, double, double, Types::ColorTypes, int)),
                                                                    SLOT(updateImageInfoFields(const QSize&, const QSizeF&, double, double, Types::ColorTypes, int))},
        {SIGNAL(setCurrentTranslationSignal(const QString&)),       SLOT(setCurrentTranslation(const QString&))},
        {SIGNAL(addAboutDialogActionSignal(QAction*)),              SLOT(addAboutDialogAction(QAction*))},
        {SIGNAL(readSettingsSignal(const QSettings*)),              SLOT(readSettings(const QSettings*))},
        {SIGNAL(writeSettingsSignal(QSettings*)),                   SLOT(writeSettings(QSettings*))},
        {SIGNAL(setCurrentUnitOfLengthSignal(const QString&)),      SLOT(setCurrentUnitOfLength(const QString&))},
        {SIGNAL(setPosterSavingEnabledSignal(bool)),                SLOT(setPosterSavingEnabled(bool))},
        {SIGNAL(setPosterSizeModeAvailableSignal(Types::PosterSizeModes, bool)),
                                                                    SLOT(setPosterSizeModeAvailable(Types::PosterSizeModes, bool))}
    };
    static const int signalsToViewSlotsConnectionsCount =
        int(sizeof(signalsToViewSlotsConnections)/sizeof(signalsToViewSlotsConnections[0]));
    for (int i = 0; i < signalsToViewSlotsConnectionsCount; ++i)
//        if (m_view->metaObject()->indexOfSlot(signalsToViewSlotsConnections[i].slot) != -1)
            connect(this, signalsToViewSlotsConnections[i].signal, m_view, signalsToViewSlotsConnections[i].slot);

    m_wizardController = new WizardController(m_view, this);

    if (!m_posteRazorCore->imageIOLibraryAboutText().isEmpty()) {
        QAction *aboutAction = new QAction(m_view);
        aboutAction->setText(QLatin1String("A&bout ") + m_posteRazorCore->imageIOLibraryName());
        connect (aboutAction, SIGNAL(triggered()), SLOT(showExtraAboutDialog()));
        emit addAboutDialogActionSignal(aboutAction);
    }

    updateDialog();
    setDialogPosterSizeMode();

    if (m_posteRazorCore->isImageLoaded()) {
        m_posteRazorCore->createPreviewImage();
        m_wizardController->handleImageLoaded();
    }

    m_translator = new QTranslator(this);
    loadTranslation(QString());
}

void Controller::updateDialog()
{
    const QString unitOfLength = Types::unitsOfLength().value(m_posteRazorCore->unitOfLength()).first;
    emit setCurrentUnitOfLengthSignal(unitOfLength);
    setDialogPaperOptions();
    setDialogPosterOptions();
    setDialogOverlappingOptions();
    setDialogImageInfoFields();
    setDialogSaveOptions();
    updatePreview();
}

void Controller::updatePreview()
{
    emit updatePreviewSignal();
}

void Controller::setPosterSizeModeAvailable(Types::PosterSizeModes mode, bool available)
{
    emit setPosterSizeModeAvailableSignal(mode, available);
}

void Controller::setUnitOfLength(Types::UnitsOfLength unit)
{
    m_posteRazorCore->setUnitOfLength(unit);
    updateDialog();
}

void Controller::setPaperFormat(const QString &format)
{
    m_posteRazorCore->setPaperFormat(format);
    setDialogPosterOptions();
    setDialogPaperBorders();
    updatePreview();
}

void Controller::setPaperOrientation(QPageLayout::Orientation orientation)
{
    m_posteRazorCore->setPaperOrientation(orientation);
    setDialogPosterOptions();
    setDialogPaperBorders();
    updatePreview();
}

void Controller::setPaperBorderTop(double border)
{
    m_posteRazorCore->setPaperBorderTop(border);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setPaperBorderRight(double border)
{
    m_posteRazorCore->setPaperBorderRight(border);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setPaperBorderBottom(double border)
{
    m_posteRazorCore->setPaperBorderBottom(border);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setPaperBorderLeft(double border)
{
    m_posteRazorCore->setPaperBorderLeft(border);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setCustomPaperWidth(double width)
{
    m_posteRazorCore->setCustomPaperWidth(width);
    setDialogPosterOptions();
    setDialogPaperBorders();
    updatePreview();
}

void Controller::setCustomPaperHeight(double height)
{
    m_posteRazorCore->setCustomPaperHeight(height);
    setDialogPosterOptions();
    setDialogPaperBorders();
    updatePreview();
}

void Controller::setUseCustomPaperSize(bool useIt)
{
    m_posteRazorCore->setUseCustomPaperSize(useIt);
    setDialogPosterOptions();
    setDialogPaperBorders();
    updatePreview();
}

void Controller::setOverlappingWidth(double width)
{
    m_posteRazorCore->setOverlappingWidth(width);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setOverlappingHeight(double height)
{
    m_posteRazorCore->setOverlappingHeight(height);
    setDialogPosterOptions();
    updatePreview();
}

void Controller::setOverlappingPosition(Qt::Alignment position)
{
    m_posteRazorCore->setOverlappingPosition(position);
    updatePreview();
}

void Controller::setPosterWidthAbsolute(double width)
{
    m_posteRazorCore->setPosterWidth(Types::PosterSizeModeAbsolute, width);
    setDialogPosterDimensions(Types::PosterSizeModeAbsolute, true);
}

void Controller::setPosterHeightAbsolute(double height)
{
    m_posteRazorCore->setPosterHeight(Types::PosterSizeModeAbsolute, height);
    setDialogPosterDimensions(Types::PosterSizeModeAbsolute, false);
}

void Controller::setPosterWidthPages(double width)
{
    m_posteRazorCore->setPosterWidth(Types::PosterSizeModePages, width);
    setDialogPosterDimensions(Types::PosterSizeModePages, true);
}

void Controller::setPosterHeightPages(double height)
{
    m_posteRazorCore->setPosterHeight(Types::PosterSizeModePages, height);
    setDialogPosterDimensions(Types::PosterSizeModePages, false);
}

void Controller::setPosterSizePercentual(double percent)
{
    m_posteRazorCore->setPosterHeight(Types::PosterSizeModePercentual, percent);
    setDialogPosterDimensions(Types::PosterSizeModePercentual, false);
}

void Controller::setPosterSizeMode(Types::PosterSizeModes mode)
{
    m_posteRazorCore->setPosterSizeMode(mode);
}

void Controller::setPosterAlignment(Qt::Alignment alignment)
{
    m_posteRazorCore->setPosterAlignment(alignment);
    updatePreview();
}

void Controller::setLaunchPDFApplication(bool launch)
{
    m_launchPDFApplication = launch;
    setDialogSaveOptions();
}

void Controller::setDialogSaveOptions()
{
    emit setLaunchPDFApplicationSignal(m_launchPDFApplication);
}

void Controller::setDialogPosterSizeMode()
{
    emit setPosterSizeModeSignal(m_posteRazorCore->posterSizeMode());
}

void Controller::setDialogPosterOptions()
{
    setDialogPosterDimensions(Types::PosterSizeModeNone, true);
    emit setPosterAlignmentSignal(m_posteRazorCore->posterAlignment());
    emit setPosterSizeModeSignal(m_posteRazorCore->posterSizeMode());
}

void Controller::setDialogPosterDimensions(Types::PosterSizeModes excludedMode, bool widthExcluded)
{
    const QSizeF posterSizeAbsolute = m_posteRazorCore->posterSize(Types::PosterSizeModeAbsolute);
    const QSizeF posterSizePages = m_posteRazorCore->posterSize(Types::PosterSizeModePages);
    const QSizeF posterSizePercentual = m_posteRazorCore->posterSize(Types::PosterSizeModePercentual);
    if (excludedMode != Types::PosterSizeModeAbsolute || !widthExcluded)
        emit setPosterWidthAbsoluteSignal(posterSizeAbsolute.width());
    if (excludedMode != Types::PosterSizeModeAbsolute || widthExcluded)
        emit setPosterHeightAbsoluteSignal(posterSizeAbsolute.height());
    if (excludedMode != Types::PosterSizeModePages || !widthExcluded)
        emit setPosterWidthPagesSignal(posterSizePages.width());
    if (excludedMode != Types::PosterSizeModePages || widthExcluded)
        emit setPosterHeightPagesSignal(posterSizePages.height());
    if (excludedMode != Types::PosterSizeModePercentual)
        emit setPosterSizePercentualSignal(posterSizePercentual.width());
    updatePreview();
}

void Controller::setDialogPaperOptions()
{
    setDialogPaperBorders();
    setDialogCustomPaperDimensions();
    emit setUseCustomPaperSizeSignal(m_posteRazorCore->usesCustomPaperSize());
    emit setPaperFormatSignal(m_posteRazorCore->paperFormat());
    emit setPaperOrientationSignal(m_posteRazorCore->paperOrientation());
}

void Controller::setDialogPaperBorders()
{
    emit setPaperBorderTopSignal(m_posteRazorCore->paperBorderTop());
    emit setPaperBorderRightSignal(m_posteRazorCore->paperBorderRight());
    emit setPaperBorderBottomSignal(m_posteRazorCore->paperBorderBottom());
    emit setPaperBorderLeftSignal(m_posteRazorCore->paperBorderLeft());
}

void Controller::setDialogCustomPaperDimensions()
{
    emit setCustomPaperSizeSignal(m_posteRazorCore->customPaperSize());
}

void Controller::setDialogImageInfoFields()
{
    if (m_posteRazorCore->isImageLoaded()) {
        emit updateImageInfoFieldsSignal(
            m_posteRazorCore->inputImageSizePixels(),
            m_posteRazorCore->inputImageSize(),
            m_posteRazorCore->inputImageVerticalDpi(),
            m_posteRazorCore->inputImageHorizontalDpi(),
            m_posteRazorCore->inputImageColorType(),
            m_posteRazorCore->inputImageBitsPerPixel()
        );
    }
}

void Controller::setDialogOverlappingDimensions()
{
    emit setOverlappingWidthSignal(m_posteRazorCore->overlappingWidth());
    emit setOverlappingHeightSignal(m_posteRazorCore->overlappingHeight());
}

void Controller::setDialogOverlappingOptions()
{
    setDialogOverlappingDimensions();
    emit setOverlappingPositionSignal(m_posteRazorCore->overlappingPosition());
}

void Controller::readSettings(const QSettings *settings)
{
    m_posteRazorCore->readSettings(settings);
    m_launchPDFApplication = settings->value(settingsKey_LaunchPDFApplication, m_launchPDFApplication).toBool();
    m_translationName = settings->value(settingsKey_TranslationName, m_translationName).toString();
    loadTranslation(m_translationName);
    updateDialog();
}

void Controller::writeSettings(QSettings *settings) const
{
    settings->setValue(settingsKey_LaunchPDFApplication, m_launchPDFApplication);
    if (!m_translationName.isEmpty())
        settings->setValue(settingsKey_TranslationName, m_translationName);
    m_posteRazorCore->writeSettings(settings);
}

void Controller::loadInputImage()
{
    QStringList allFilters;
    QStringList allWildcards;

    const QVector<QPair<QStringList, QString> > &formats = m_posteRazorCore->imageFormats();
    for (int i = 0; i < formats.count(); i++) {
        QStringList formatWildcards;
        foreach (const QString &extension, formats.at(i).first)
            formatWildcards << "*." + extension;
        allWildcards << formatWildcards;
        QString formatName = formats.at(i).second;
        // Some Open File dialogs (at least OSX) ar irritated if there are brackes in the file type name
        formatName.remove('(');
        formatName.remove(')');
        allFilters << formatName + " (" +  formatWildcards.join(" ") + ")";
    }
    allFilters.prepend(QCoreApplication::translate("Main window", "All image formats") + " (" +  allWildcards.join(" ") + ")");

    QSettings loadPathSettings;

    QString loadFileName = QFileDialog::getOpenFileName (
        m_view,
        QCoreApplication::translate("Main window", "Load an input image"),
        loadPathSettings.value(settingsKey_ImageLoadPath,
#if QT_VERSION >= 0x040400
            QStandardPaths::PicturesLocation
#else
            "."
#endif
            ).toString(),
        allFilters.join(";;")
    );

    if (!loadFileName.isEmpty()) {
        const bool successful = loadInputImage(loadFileName);
        if (successful)
            loadPathSettings.setValue(settingsKey_ImageLoadPath,
                QDir::toNativeSeparators(QFileInfo(loadFileName).absolutePath()));
    }
}

bool Controller::loadInputImage(const QString &fileName)
{
    QString loadErrorMessage;
    const bool successful = loadInputImage(fileName, loadErrorMessage);
    if (!successful)
        QMessageBox::critical(m_view, "", QCoreApplication::translate("Main window", "The image '%1' could not be loaded.")
            .arg(QFileInfo(fileName).fileName()));
    return successful;
}

bool Controller::loadInputImage(const QString &imageFileName, QString &errorMessage)
{
    const bool result = m_posteRazorCore->loadInputImage(imageFileName, errorMessage);
    if (result) {
        updateDialog();
        emit showImageFileNameSignal(imageFileName);
        emit setPosterSavingEnabledSignal(true);
    }
    return result;
}

int Controller::savePoster(const QString &fileName) const
{
    const int result = m_posteRazorCore->savePoster(fileName);
#if !defined(__EMSCRIPTEN__)
    if (result == 0 && m_launchPDFApplication)
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
#endif
    return result;
}

void Controller::savePoster() const
{
    QSettings savePathSettings;

#if defined(__EMSCRIPTEN__)
    QString saveFileName = "fotowall_poster.pdf"; // save to local browser storage
    int result = savePoster(saveFileName.toLatin1());
    if (result != 0)
    {
       QMessageBox::critical(m_view, "", QCoreApplication::translate("Main window", "The file '%1' could not be saved.").arg(saveFileName), QMessageBox::Ok, QMessageBox::NoButton);
    }
#else
    QString saveFileName = savePathSettings.value(settingsKey_PosterSavePath,
#if QT_VERSION >= 0x040400
           QStandardPaths::DocumentsLocation
#else
        "."
#endif
        ).toString();
    bool fileExistsAskUserForOverwrite = false;

    do {
        saveFileName = QFileDialog::getSaveFileName(
            m_view,
            QCoreApplication::translate("Main window", "Save the poster"),
            saveFileName,
            QLatin1String("Portable Document format (*.pdf)"),
            NULL,
            QFileDialog::DontConfirmOverwrite
        );

        if (!saveFileName.isEmpty()) {
            const QFileInfo saveFileInfo(saveFileName);
            if (saveFileInfo.suffix().toLower() != QLatin1String("pdf"))
                saveFileName.append(".pdf");

            fileExistsAskUserForOverwrite = QFileInfo(saveFileName).exists();

            if (!fileExistsAskUserForOverwrite
                || QMessageBox::Yes == (QMessageBox::question(m_view, "", QCoreApplication::translate("Main window", "The file '%1' already exists.\nDo you want to overwrite it?").arg(saveFileInfo.fileName()), QMessageBox::Yes, QMessageBox::No))
                ) {
                int result = savePoster(saveFileName.toLatin1());
                if (result != 0)
                    QMessageBox::critical(m_view, "", QCoreApplication::translate("Main window", "The file '%1' could not be saved.").arg(saveFileInfo.fileName()), QMessageBox::Ok, QMessageBox::NoButton);
                else
                    savePathSettings.setValue(settingsKey_PosterSavePath,
                        QDir::toNativeSeparators(QFileInfo(saveFileName).absolutePath()));
                fileExistsAskUserForOverwrite = false;
            }
        } else {
            break;
        }
    } while (fileExistsAskUserForOverwrite);
#endif
}

void Controller::loadTranslation(const QString &localeName)
{
    const QString saneLocaleName = localeName.isEmpty()?QLocale::system().name():localeName;
    const QString translationFileName(":/Translations/" + saneLocaleName);
    if (m_translator->load(translationFileName)) {
        QCoreApplication::removeTranslator(m_translator);
        QCoreApplication::installTranslator(m_translator);
        emit setCurrentTranslationSignal(saneLocaleName);
        if (!localeName.isEmpty())
            m_translationName = localeName;
        m_wizardController->updateDialogWizardStepDescription();
    }
}

void Controller::setUnitOfLength(const QString &unit)
{
    m_posteRazorCore->setUnitOfLength(Types::unitOfLenthFromString(unit));
    updateDialog();
}

void Controller::openPosteRazorWebsite()
{
    QDesktopServices::openUrl(QCoreApplication::translate("Help", "http://posterazor.sourceforge.net/", "Only translate, if the website has this language."));
}

void Controller::setImageLoadingAvailable(bool available)
{
    m_wizardController->setStepAvailable(WizardController::WizardStepInputImage, available);
}

void Controller::showExtraAboutDialog()
{
    const QString title = QLatin1String("About ") + m_posteRazorCore->imageIOLibraryName();
    QMessageBox::about(
        m_view, title,
        QString(QLatin1String("<h3>%1</h3>%2")) // QMessageBox::aboutQt() also uses <h3>
            .arg(title)
            .arg(Types::newlineToParagraph(m_posteRazorCore->imageIOLibraryAboutText()))
    );
}
