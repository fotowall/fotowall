/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Tanguy Arnaud <arn.tanguy@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ExportWizard.h"

#include "App.h"
#include "Canvas/Canvas.h"
#include "Canvas/CanvasModeInfo.h"
#include "Settings.h"
#include "Shared/RenderOpts.h"
#include "controller.h"
#include "imageloaderqt.h"
#include "posterazorcore.h"
#include "ui_ExportWizard.h"
#include "wizard.h"

#include <QDebug>
#include <QDesktopServices>
#if QT_VERSION <= QT_VERSION_CHECK(6, 0, 0)
#  include <QDesktopWidget>
#else
#  include <QScreen>
#endif
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLocale>
#include <QMessageBox>
#include <QPageSetupDialog>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProcess>
#include <QSettings>
#include <QSvgGenerator>
#include <QTimer>
#include <QUrl>
#include <math.h>
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
#  include <QScreen>
#endif

#if defined(__EMSCRIPTEN__)
#  include <QBuffer>
#endif

#if defined(Q_OS_WIN)
#  include <windows.h> // for background changing stuff
#endif

#define POSTERAZOR_WEBSITE_LINK "http://posterazor.sourceforge.net/"
#define POSTERAZOR_TUTORIAL_LINK "http://www.youtube.com/watch?v=p7XsFZ4Leo8"

static QString getSavePath(const QString & initialValue,
                           const QString & defaultExt,
                           const QString & title,
                           const QString & type)
{
  // make up the default save path (stored as 'Fotowall/ExportDir')
  QString defaultSavePath = initialValue;
  if(defaultSavePath.isEmpty())
  {
    defaultSavePath = ExportWizard::tr("Unnamed %1.%2").arg(QDate::currentDate().toString()).arg(defaultExt);
    if(App::settings->contains("Fotowall/ExportDir"))
      defaultSavePath.prepend(App::settings->value("Fotowall/ExportDir").toString() + QDir::separator());
  }

  // ask the file name, validate it, store back to settings
  QString saveFilePath = QFileDialog::getSaveFileName(0, title, defaultSavePath, type);
  if(!saveFilePath.isEmpty())
  {
    App::settings->setValue("Fotowall/ExportDir", QFileInfo(saveFilePath).absolutePath());
    if(QFileInfo(saveFilePath).suffix().isEmpty()) saveFilePath += "." + defaultExt;
  }
  return saveFilePath;
}

ExportWizard::ExportWizard(Canvas * canvas, bool printPreferred)
: QWizard(), m_ui(new Ui::ExportWizard), m_canvas(canvas), m_printPreferred(printPreferred), m_nextId(0),
  m_pdfPrinter(0)
{
  // create and init UI
  m_ui->setupUi(this);
  connect(m_ui->clWallpaper, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
  connect(m_ui->clImage, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
  connect(m_ui->clPosteRazor, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
#if defined(__EMSCRIPTEN__)
  m_ui->clPrint->setVisible(false);
  m_ui->clPosteRazor->setVisible(false); // TODO: fix this as it ought to be working
#else
  connect(m_ui->clPrint, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
#endif
  connect(m_ui->clPdf, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
  connect(m_ui->clSvg, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
  m_ui->prWebLabel->setText("<html><body><a href='" POSTERAZOR_WEBSITE_LINK "'>" + m_ui->prWebLabel->text()
                            + "</a></body></html>");
  connect(m_ui->prWebLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));
  m_ui->prTutorialLabel->setText("<html><body><a href='" POSTERAZOR_TUTORIAL_LINK "'>" + m_ui->prTutorialLabel->text()
                                 + "</a></body></html>");
  connect(m_ui->prTutorialLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));

#if !defined(Q_OS_WIN) && !defined(Q_OS_LINUX) && !defined(Q_OS_OS2)
#  warning "Implement background change for this OS"
  m_ui->clWallpaper->hide();
#endif

  // set default sizes
  m_ui->saveHeight->setValue(m_canvas->height());
  m_ui->saveWidth->setValue(m_canvas->width());
  m_ui->imgFromDpi->setEnabled(m_printPreferred);
  m_ui->printDpi->setValue(m_canvas->modeInfo()->printDpi());
  m_ui->printLandscape->setChecked(m_canvas->modeInfo()->printLandscape());
  m_printSizeInches = canvasNatInches();

  // connect buttons
  connect(m_ui->imgFromCanvas, SIGNAL(clicked()), this, SLOT(slotImageFromCanvas()));
  connect(m_ui->imgFromDpi, SIGNAL(clicked()), this, SLOT(slotImageFromDpi()));
  connect(m_ui->printUnity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintUnityChanged(int)));
  connect(m_ui->printWidth, SIGNAL(valueChanged(double)), this, SLOT(slotPrintSizeChanged()));
  connect(m_ui->printHeight, SIGNAL(valueChanged(double)), this, SLOT(slotPrintSizeChanged()));
  connect(m_ui->printDpi, SIGNAL(valueChanged(int)), this, SLOT(slotPrintSizeChanged()));
  connect(m_ui->pdfPageButton, SIGNAL(clicked()), this, SLOT(slotChoosePdfPage()));
  connect(m_ui->pdfPreviewButton, SIGNAL(clicked()), this, SLOT(slotPdfPreview()));
  connect(m_ui->pdfRes, SIGNAL(valueChanged(int)), this, SLOT(slotPdfResChanged(int)));
  bool imperial = QLocale::system().measurementSystem() == QLocale::ImperialSystem;
  m_ui->printUnity->setCurrentIndex(imperial ? 2 : 1);

  // configure Wizard
  setOptions(NoDefaultButton | NoBackButtonOnStartPage | IndependentPages);
  setPage(PageMode);
  setMinimumWidth(400);
  if(m_printPreferred)
  {
    // clear the boldness of non-print buttons
    QFont font;
    font.setBold(false);
    m_ui->clImage->setFont(font);
    m_ui->clWallpaper->setFont(font);
    m_ui->clPosteRazor->setFont(font);
    m_ui->clPdf->setFont(font);
    m_ui->clSvg->setFont(font);

    // set the focus to the print button
    show();
    m_ui->clPrint->setFocus();
    // QTimer::singleShot(800, m_ui->clPrint, SLOT(animateClick()));
  }

  // react to 'finish'
  connect(this, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
}

ExportWizard::~ExportWizard()
{
  delete m_ui;
  delete m_pdfPrinter;
}

void ExportWizard::setWallpaper()
{
  // find a new path
  QString wFilePath;
  int fileNumber = 0;
  while(wFilePath.isEmpty() || QFile::exists(wFilePath))
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    wFilePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + "fotowall-background"
                + QString::number(++fileNumber) + ".bmp";
#else
    wFilePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + "fotowall-background"
                + QString::number(++fileNumber) + ".jpg";
#endif

  // render the image
  QImage image;
  QSize sceneSize(m_canvas->width(), m_canvas->height());
  QSize desktopSize = QGuiApplication::primaryScreen()->size();
  if(m_ui->wbZoom->isChecked())
    image = m_canvas->renderedImage(desktopSize, Qt::KeepAspectRatioByExpanding);
  else if(m_ui->wbScaleKeep->isChecked())
    image = m_canvas->renderedImage(desktopSize, Qt::KeepAspectRatio);
  else if(m_ui->wbScaleIgnore->isChecked())
    image = m_canvas->renderedImage(desktopSize, Qt::IgnoreAspectRatio);
  else
    image = m_canvas->renderedImage(sceneSize);

    // save the right kind of image into the home dir
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
  if(!image.save(wFilePath, "BMP"))
  {
#else
  if(!image.save(wFilePath, "JPG", 100))
  {
#endif
    QMessageBox::warning(this, tr("Wallpaper Error"), tr("Can't save the image to disk."));
    return;
  }

#if defined(Q_OS_WIN)
  // Set new background path
  {
    QSettings appSettings("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    appSettings.setValue("ConvertedWallpaper", wFilePath);
    appSettings.setValue("Wallpaper", wFilePath);
  }

  // Notification to windows refresh desktop
  SystemParametersInfoA(SPI_SETDESKWALLPAPER, true, (void *)qPrintable(wFilePath),
                        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#elif defined(Q_OS_LINUX)
  // KDE4
  if(QString(qgetenv("KDE_SESSION_VERSION")).startsWith("4"))
    QMessageBox::warning(this, tr("Manual Wallpaper Change"),
                         tr("KDE4 doesn't yet support changing wallpaper automatically.\nGo to the Desktop Settings "
                            "and select the file:\n  %1")
                             .arg(wFilePath));

  // KDE3
  QProcess::startDetached("dcop", QStringList()
                                      << "kdesktop" << "KBackgroundIface" << "setWallpaper" << wFilePath << "6");

  // Gnome2
  QProcess::startDetached("gconftool", QStringList() << "-t" << "string" << "-s"
                                                     << "/desktop/gnome/background/picture_filename" << wFilePath);
#else
#  warning "Implement background change for this OS"
#endif
}

void ExportWizard::saveImage()
{
  // get the rendering size
  QSize imageSize(m_ui->saveWidth->value(), m_ui->saveHeight->value());

  // render the image
  QImage image;
  bool hideTools = !m_ui->imgAsIsBox->isChecked();
  if(m_ui->ibZoom->isChecked())
    image = m_canvas->renderedImage(imageSize, Qt::KeepAspectRatioByExpanding, hideTools);
  else if(m_ui->ibScaleKeep->isChecked())
    image = m_canvas->renderedImage(imageSize, Qt::KeepAspectRatio, hideTools);
  else
    image = m_canvas->renderedImage(imageSize, Qt::IgnoreAspectRatio, hideTools);

  // rotate image if requested
  if(m_ui->saveLandscape->isChecked())
  {
    // Save in landscape mode, so rotate
    QTransform matrix;
    matrix.rotate(90);
    image = image.transformed(matrix);
  }

#if defined(__EMSCRIPTEN__)
  QByteArray arr;
  QBuffer buffer(&arr);
  buffer.open(QIODevice::WriteOnly);
  // TODO: allow user to choose image format when saving from the browser
  // This would require adding a dedicated format option to the image save UI.
  image.save(&buffer, "PNG");
  QFileDialog::saveFileContent(arr, "fotowall.png");
#else
  QString imgFilePath =
      getSavePath("", "png", tr("Choose the Image file"), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
  if(image.save(imgFilePath) && QFile::exists(imgFilePath))
  {
    int size = QFileInfo(imgFilePath).size();
    QMessageBox::information(this, tr("Done"), tr("The target image is %1 bytes long").arg(size));
  }
  else
    QMessageBox::warning(this, tr("Rendering Error"), tr("Error rendering to the file '%1'").arg(imgFilePath));
#endif
}

void ExportWizard::startPosterazor()
{
  static const quint32 posterPixels = 6 * 1000000; // Megapixels * 3 bytes!
  // We will use up the whole posterPixels for the render, respecting the aspect ratio.
  const qreal widthToHeightRatio = m_canvas->width() / m_canvas->height();
  // Thanks to colleague Oswald for some of the math :)
  const int posterPixelWidth = int(sqrt(widthToHeightRatio * posterPixels));
  const int posterPixelHeight = posterPixels / posterPixelWidth;

  static const QLatin1String settingsGroup("posterazor");
  App::settings->beginGroup(settingsGroup);

  // TODO: Eliminate Poster size in %
  ImageLoaderQt loader;
  loader.setQImage(m_canvas->renderedImage(QSize(posterPixelWidth, posterPixelHeight)));
  PosteRazorCore posterazor(&loader);
  posterazor.readSettings(App::settings);
  Wizard * wizard = new Wizard;
  Controller controller(&posterazor, wizard);
  controller.setImageLoadingAvailable(false);
  controller.setPosterSizeModeAvailable(Types::PosterSizeModePercentual, false);
  QDialog dialog(this, Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
  dialog.setWindowTitle(tr("Export poster"));
  dialog.setLayout(new QVBoxLayout);
  dialog.layout()->addWidget(wizard);
  dialog.resize(640, 480);
  dialog.exec();
  App::settings->sync();
  posterazor.writeSettings(App::settings);
  App::settings->endGroup();
}

bool ExportWizard::printPaper()
{
  // update the realsizeinches, just in case..
  slotPrintSizeChanged();

  // get dpi, compute printed size
  int printDpi = m_ui->printDpi->value();
  int printWidth = (int)(m_printSizeInches.width() * (float)printDpi);
  int printHeight = (int)(m_printSizeInches.height() * (float)printDpi);
  QSize printSize(printWidth, printHeight);
  Qt::AspectRatioMode printRatio = m_ui->printKeepRatio->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;

  // check if print params differ from the 'Exact Size' stuff
  if(m_printPreferred)
  {
    if(printDpi != m_canvas->modeInfo()->printDpi())
    {
      qWarning("ExportWizard::print: dpi changed to %d from the default %d", printDpi,
               (int)m_canvas->modeInfo()->printDpi());
    }
    else
    {
      QSize exactPrintSize = m_canvas->modeInfo()->fixedPrinterPixels();
      if(printSize != exactPrintSize)
        qWarning("ExportWizard::print: size changed to %dx%d from the default %dx%d", printWidth, printHeight,
                 exactPrintSize.width(), exactPrintSize.height());
    }
  }

  // setup printer
  QPrinter printer;
  printer.setResolution(printDpi);
  printer.setPageSize(QPageSize(QPageSize::A4));

  // configure printer via the print dialog
  QPrintDialog printDialog(&printer);
  if(printDialog.exec() != QDialog::Accepted) return false;

  // TODO: use different ratio modes?
  QImage image = m_canvas->renderedImage(printSize, printRatio);
  if(m_ui->printLandscape->isChecked())
  {
    // Print in landscape mode, so rotate
    QTransform matrix;
    matrix.rotate(90);
    image = image.transformed(matrix);
  }

  // And then print
  QPainter painter;
  painter.drawImage(image.rect(), image);
  painter.end();
  return true;
}

bool ExportWizard::printPdf()
{
#if !defined(__EMSCRIPTEN__)
  QString savePath = getSavePath(m_pdfPrinter->outputFileName(), "pdf", tr("Choose the PDF file"), tr("PDF (*.pdf)"));
#else
  QString savePath = "fotowall.pdf"; // Save to browser's in-memory local file storage
#endif

  if(!savePath.isEmpty()) m_pdfPrinter->setOutputFileName(savePath);

  // get dpi, compute printed size
  int printDpi = m_pdfPrinter->resolution();
  QSizeF canvasPrintSize = canvasNatInches() * (qreal)printDpi;
  Qt::AspectRatioMode printAspect = Qt::KeepAspectRatio;

  // open the painter over the printer
  QPainter painter;
  if(!painter.begin(m_pdfPrinter))
  {
    QMessageBox::warning(0, tr("PDF Error"), tr("Error saving to the PDF file, try to chose another one."),
                         QMessageBox::Cancel);
    return false;
  }
  QRect paperRect = painter.viewport();
  QRect targetRect(0, 0, (int)canvasPrintSize.width(), (int)canvasPrintSize.height());

  // adapt rect Scale
  switch(m_ui->pdfScaleCombo->currentIndex())
  {
    case 0: // Original
      break;
    case 1:
    { // Fit to page
      QSize targetSize = targetRect.size();
      targetSize.scale(paperRect.size(), printAspect);
      targetRect.setSize(targetSize);
    }
    break;
  }

  // adapt Position
  switch(m_ui->pdfPosCombo->currentIndex())
  {
    case 0: // Top Left
      targetRect.moveTopLeft(paperRect.topLeft());
      break;
    case 1: // Center
      targetRect.moveCenter(paperRect.center());
      break;
  }

  // render to PDF
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
  RenderOpts::PDFExporting = true;
  m_canvas->renderVisible(&painter, targetRect, m_canvas->sceneRect(), Qt::IgnoreAspectRatio, true);
  RenderOpts::PDFExporting = false;
  painter.end();

#if defined(__EMSCRIPTEN__)
  QFile pdfFile(savePath); // Read PDF stored in local browser storage
  pdfFile.open(QIODevice::ReadOnly);
  QByteArray content = pdfFile.readAll();
  pdfFile.remove();

  QFileDialog::saveFileContent(content, "fotowall.pdf"); // download pdf
#endif

  return true;
}

void ExportWizard::saveSvg()
{
  // get the rendering size
  QRect svgRect(m_canvas->sceneRect().toRect());

  // create the SVG writer
  QSvgGenerator generator;
#if defined(__EMSCRIPTEN__)
  QByteArray arr;
  QBuffer buffer(&arr);
  buffer.open(QIODevice::WriteOnly);
  generator.setOutputDevice(&buffer);
#else
  QString svgFilePath = getSavePath("", "svg", tr("Choose the Image file"), tr("Images (*.svg)"));
  generator.setFileName(svgFilePath);
#endif
  generator.setSize(svgRect.size());
  generator.setResolution(physicalDpiX());
  generator.setViewBox(svgRect);
  generator.setTitle(m_canvas->titleText());
  generator.setDescription(
      tr("Created with %1").arg(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion()));

  // paint over the writer
  QPainter painter(&generator);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
  m_canvas->renderVisible(&painter, svgRect, svgRect, Qt::IgnoreAspectRatio, !m_ui->svgAsIsBox->isChecked());
  painter.end();

#if defined(__EMSCRIPTEN__)
  QFileDialog::saveFileContent(arr, "fotowall.svg");
#endif
}

void ExportWizard::setPage(int pageId)
{
  // adapt buttons
  QList<QWizard::WizardButton> layout;
  layout << QWizard::Stretch << QWizard::BackButton;
  if(pageId >= PageWallpaper && pageId <= PagePdf) layout << QWizard::FinishButton;
  layout << QWizard::CancelButton;
  setButtonLayout(layout);

  // change page
  m_nextId = pageId;
  next();

  // execute on-entry code
  switch(pageId)
  {
    case PagePdf:
      if(!m_pdfPrinter)
      {
        m_pdfPrinter = new QPrinter;
        m_pdfPrinter->setOutputFormat(QPrinter::PdfFormat);
        m_pdfPrinter->setResolution(m_canvas->modeInfo()->printDpi());
        m_pdfPrinter->setPageSize(QPageSize(canvasNatInches(), QPageSize::Inch));
        m_pdfPrinter->setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Inch);
        initPageSizeNames();
        slotPdfUpdateGui();
      }
      break;
  }
}

int ExportWizard::nextId() const
{
  // dynamic page ordering
  const int pageId = currentId();

  // mode selection: return the id of the next page (set by the linkbuttons)
  if(pageId == PageMode) return m_nextId;

  // final pages
  if(pageId >= PageWallpaper && pageId <= PagePdf) return -1;

  // fallback
  qWarning("ExportWizard::nextId: unhandled nextId for page %d", pageId);
  return -1;
}

QSizeF ExportWizard::canvasNatInches() const
{
  if(m_canvas->modeInfo()->fixedSize()) return m_canvas->modeInfo()->fixedSizeInches();
  QSize canvasPixels = m_canvas->sceneSize();
  QPointF canvasRes = m_canvas->modeInfo()->screenDpi();
  return QSizeF((qreal)canvasPixels.width() / canvasRes.x(), (qreal)canvasPixels.height() / canvasRes.y());
}

void ExportWizard::initPageSizeNames()
{
  m_paperSizeNames[QPageSize::A0] = QPrintDialog::tr("A0");
  m_paperSizeNames[QPageSize::A1] = QPrintDialog::tr("A1");
  m_paperSizeNames[QPageSize::A2] = QPrintDialog::tr("A2");
  m_paperSizeNames[QPageSize::A3] = QPrintDialog::tr("A3");
  m_paperSizeNames[QPageSize::A4] = QPrintDialog::tr("A4");
  m_paperSizeNames[QPageSize::A5] = QPrintDialog::tr("A5");
  m_paperSizeNames[QPageSize::A6] = QPrintDialog::tr("A6");
  m_paperSizeNames[QPageSize::A7] = QPrintDialog::tr("A7");
  m_paperSizeNames[QPageSize::A8] = QPrintDialog::tr("A8");
  m_paperSizeNames[QPageSize::A9] = QPrintDialog::tr("A9");
  m_paperSizeNames[QPageSize::B0] = QPrintDialog::tr("B0");
  m_paperSizeNames[QPageSize::B1] = QPrintDialog::tr("B1");
  m_paperSizeNames[QPageSize::B2] = QPrintDialog::tr("B2");
  m_paperSizeNames[QPageSize::B3] = QPrintDialog::tr("B3");
  m_paperSizeNames[QPageSize::B4] = QPrintDialog::tr("B4");
  m_paperSizeNames[QPageSize::B5] = QPrintDialog::tr("B5");
  m_paperSizeNames[QPageSize::B6] = QPrintDialog::tr("B6");
  m_paperSizeNames[QPageSize::B7] = QPrintDialog::tr("B7");
  m_paperSizeNames[QPageSize::B8] = QPrintDialog::tr("B8");
  m_paperSizeNames[QPageSize::B9] = QPrintDialog::tr("B9");
  m_paperSizeNames[QPageSize::B10] = QPrintDialog::tr("B10");
  m_paperSizeNames[QPageSize::C5E] = QPrintDialog::tr("C5E");
  m_paperSizeNames[QPageSize::DLE] = QPrintDialog::tr("DLE");
  m_paperSizeNames[QPageSize::Executive] = QPrintDialog::tr("Executive");
  m_paperSizeNames[QPageSize::Folio] = QPrintDialog::tr("Folio");
  m_paperSizeNames[QPageSize::Ledger] = QPrintDialog::tr("Ledger");
  m_paperSizeNames[QPageSize::Legal] = QPrintDialog::tr("Legal");
  m_paperSizeNames[QPageSize::Letter] = QPrintDialog::tr("Letter");
  m_paperSizeNames[QPageSize::Tabloid] = QPrintDialog::tr("Tabloid");
  m_paperSizeNames[QPageSize::Comm10E] = QPrintDialog::tr("US Common #10 Envelope");
  m_paperSizeNames[QPageSize::PageSizeId::Custom] = QPrintDialog::tr("Custom");
}

void ExportWizard::slotChoosePdfPage()
{
  QPageSetupDialog pageSetup(m_pdfPrinter);
  if(pageSetup.exec() == QDialog::Accepted) slotPdfUpdateGui();
}

void ExportWizard::slotChoosePdfPath() {}

void ExportWizard::slotImageFromCanvas()
{
  m_ui->saveWidth->setValue(m_canvas->width());
  m_ui->saveHeight->setValue(m_canvas->height());
}

void ExportWizard::slotImageFromDpi()
{
  QSize printSize = m_canvas->modeInfo()->fixedPrinterPixels();
  m_ui->saveWidth->setValue(printSize.width());
  m_ui->saveHeight->setValue(printSize.height());
}

void ExportWizard::slotPrintUnityChanged(int index)
{
  m_ui->printWidth->blockSignals(true);
  m_ui->printHeight->blockSignals(true);
  if(index == 0)
  {
    m_ui->printWidth->setValue(m_printSizeInches.width() * (qreal)m_ui->printDpi->value());
    m_ui->printHeight->setValue(m_printSizeInches.height() * (qreal)m_ui->printDpi->value());
  }
  else if(index == 1)
  {
    m_ui->printWidth->setValue(m_printSizeInches.width() * 2.54);
    m_ui->printHeight->setValue(m_printSizeInches.height() * 2.54);
  }
  else if(index == 2)
  {
    m_ui->printWidth->setValue(m_printSizeInches.width());
    m_ui->printHeight->setValue(m_printSizeInches.height());
  }
  m_ui->printWidth->blockSignals(false);
  m_ui->printHeight->blockSignals(false);
}

void ExportWizard::slotPrintSizeChanged()
{
  qreal newWidth = m_ui->printWidth->value();
  qreal newHeight = m_ui->printHeight->value();
  qreal newDpi = (qreal)m_ui->printDpi->value();
  qWarning("SPSC %f %f %f!", newWidth, newHeight, newDpi);
  switch(m_ui->printUnity->currentIndex())
  {
    case 0: // pixels/dpi -> inches
      m_printSizeInches = QSizeF(newWidth / newDpi, newHeight / newDpi);
      break;
    case 1: // cm/2.54 -> inches
      m_printSizeInches = QSizeF(newWidth / 2.54, newHeight / 2.54);
      break;
    case 2: // inches -> inches
      m_printSizeInches = QSizeF(newWidth, newHeight);
      break;
  }
}

void ExportWizard::slotPdfPreview()
{
  QPrintPreviewDialog * previewDialog = new QPrintPreviewDialog(m_pdfPrinter, this);
  connect(previewDialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(printPdf()));
  previewDialog->exec();
  delete previewDialog;
}

void ExportWizard::slotPdfResChanged(int value)
{
  if(m_pdfPrinter) m_pdfPrinter->setResolution(value);
}

void ExportWizard::slotPdfUpdateGui()
{
  if(!m_pdfPrinter) return;

  // change paper button text
  QPageSize::PageSizeId pNumber = m_pdfPrinter->pageLayout().pageSize().id();
  QString paperName = m_paperSizeNames.contains(pNumber) ? m_paperSizeNames[pNumber] : tr("Other");
  m_ui->pdfPageButton->setText(paperName);
  //    if (pNumber == QPrinter::Custom) {
  bool useInches = QLocale::system().measurementSystem() == QLocale::ImperialSystem;
  QSizeF paperSize = m_pdfPrinter->pageLayout().pageSize().size(useInches ? QPageSize::Inch : QPageSize::Millimeter);
  if(useInches)
    m_ui->pdfPageButton->setText(
        tr("%1  (%2 x %3 inch)").arg(paperName).arg(paperSize.width()).arg(paperSize.height()));
  else
    m_ui->pdfPageButton->setText(
        tr("%1  (%2 x %3 cm)").arg(paperName).arg(paperSize.width() / 10).arg(paperSize.height() / 10));
  //    }

  // change resolution text
  m_ui->pdfRes->setValue(m_pdfPrinter->resolution());
}

void ExportWizard::slotFinished(int code)
{
  if(code == QDialog::Accepted)
  {
    switch(currentId())
    {
      case PageWallpaper:
        setWallpaper();
        break;
      case PageImage:
        saveImage();
        break;
      case PagePosteRazor:
        startPosterazor();
        break;
      case PagePrint:
        printPaper();
        break;
      case PagePdf:
        printPdf();
        break;
      case PageSvg:
        saveSvg();
        break;
      default:
        qWarning("ExportWizard::slotFinished: unhndled end for page %d", currentId());
        break;
    }
  }
}

void ExportWizard::slotModeButtonClicked()
{
  setPage(sender()->property("nextPageId").toInt());
}

void ExportWizard::slotOpenLink(const QString & address)
{
  QDesktopServices::openUrl(QUrl(address));
}
