/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009-2010 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CanvasAppliance.h"

#include "App.h"
#include "Canvas/Canvas.h"
#include "Canvas/CanvasModeInfo.h"
#include "Canvas/CanvasViewContent.h"
#include "Canvas/WordcloudContent.h"
#include "ExactSizeDialog.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/Commands.h"
#include "Shared/PanePropertyEditor.h"
#include "Shared/VideoProvider.h"
#if defined(HAS_EXPORTDIALOG)
#  include "ExportWizard.h"
#endif
#include "FotowallFile.h"
#include "Settings.h"
#include "Workflow.h"

#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QSlider>
#include <QWidgetAction>

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
#  include <QScreen>
#endif

CanvasAppliance::CanvasAppliance(Canvas * extCanvas, QObject * parent)
: QObject(parent), m_extCanvas(extCanvas), m_dummyWidget(new QWidget), m_gBackModeGroup(0), m_gBackRatioGroup(0),
  m_gBackContentAction(0)
{
  // init UI
  ui.setupUi(m_dummyWidget);
  connect(ui.bPicture, SIGNAL(clicked()), this, SLOT(slotAddPicture()));
  connect(ui.bText, SIGNAL(clicked()), this, SLOT(slotAddText()));
  connect(ui.bWordcloud, SIGNAL(clicked()), this, SLOT(slotAddWordcloud()));
  connect(ui.bCanvas, SIGNAL(clicked()), this, SLOT(slotAddCanvas()));
  connect(ui.bWebsearch, SIGNAL(toggled(bool)), this, SLOT(slotSearchPicturesToggled(bool)));
#if !defined(MOBILE_UI)
  ui.propertiesBox->collapse();
  ui.canvasPropertiesBox->expand();
#endif
  connect(ui.projectCombo, SIGNAL(activated(int)), this, SLOT(slotProjectComboActivated(int)));
  connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(slotFileSave()));
#if defined(HAS_EXPORTDIALOG)
  connect(ui.exportButton, SIGNAL(clicked()), this, SLOT(slotFileExport()));
#else
  ui.exportButton->hide();
#endif

  // configure the appliance
  windowTitleSet(m_extCanvas->prettyBaseName());
  sceneSet(m_extCanvas);
  ui.addContentBox->setProperty("@onTopbar", true);
  topbarAddWidget(ui.addContentBox);
  topbarAddWidget(ui.propertiesBox);
  topbarAddWidget(ui.canvasPropertiesBox);
  topbarAddWidget(ui.fileBox, true);

  // populate menus
  ui.arrangeButton->setMenu(createArrangeMenu());
  ui.backButton->setMenu(createBackgroundMenu());
  ui.decoButton->setMenu(createDecorationMenu());

  // react to canvas
  connect(m_extCanvas, SIGNAL(backConfigChanged()), this, SLOT(slotBackConfigChanged()));

  connect(m_extCanvas, SIGNAL(requestContentEditing(AbstractContent *)), this,
          SLOT(slotEditContent(AbstractContent *)));
  connect(m_extCanvas, SIGNAL(showPropertiesWidget(QWidget *)), this, SLOT(slotShowPropertiesWidget(QWidget *)));
  connect(m_extCanvas, SIGNAL(filePathChanged()), this, SLOT(slotFilePathChanged()));

  // react to VideoProvider
  slotVerifyVideoInputs(VideoProvider::instance()->inputCount());
  connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

  // set the startup project mode
  setProjectMode(extCanvas->modeInfo()->projectMode());

  ui.undoButton->setEnabled(false);
  m_extCanvas->commandStack().createRedoAction(ui.redoButton, tr("Redo"));
  connect(ui.undoButton, SIGNAL(clicked()), m_extCanvas, SLOT(undoSlot()));
  connect(m_extCanvas->commandStackPtr(), SIGNAL(undoTextChanged(const QString &)), this,
          SLOT(slotUndoTextChanged(const QString &)));

  ui.redoButton->setEnabled(false);
  connect(ui.redoButton, SIGNAL(clicked()), m_extCanvas, SLOT(redoSlot()));
  connect(m_extCanvas->commandStackPtr(), SIGNAL(redoTextChanged(const QString &)), this,
          SLOT(slotRedoTextChanged(const QString &)));
}

CanvasAppliance::~CanvasAppliance()
{
  if(m_extCanvas) qWarning("CanvasAppliance::~CanvasAppliance: we still have a Canvas. take it before deleting this");
  delete ui.addContentBox;
  delete ui.propertiesBox;
  delete ui.canvasPropertiesBox;
  delete ui.fileBox;
  delete m_dummyWidget;
}

Canvas * CanvasAppliance::takeCanvas()
{
  Canvas * canvas = m_extCanvas;
  disconnect(canvas, 0, this, 0);
  m_extCanvas = 0;
  sceneClear();
  return canvas;
}

bool CanvasAppliance::applianceCommand(int command)
{
  switch(command)
  {
    // No Background
    case App::AC_ClearBackground:
      if(m_extCanvas->backMode() != Canvas::BackNone)
      {
        ButtonsDialog query("SwitchTransparent", tr("Transparency"),
                            tr("You won't see through the Canvas unless you clear the background.<br><b>Do you want me "
                               "to clear it?</b>"),
                            QDialogButtonBox::Yes | QDialogButtonBox::No, true, true);
        query.setButtonText(QDialogButtonBox::Yes, tr("Yes, thanks"));
        query.setIcon(QStyle::SP_MessageBoxQuestion);
        if(query.execute() == QDialogButtonBox::Yes) m_extCanvas->setBackMode(Canvas::BackNone);
      }
      return true;

    // Close picture Search
    case App::AC_ClosePicureSearch:
      ui.bWebsearch->setChecked(false);
      return true;
  }

  // unimplemented command
  qWarning("CanvasAppliance::applianceCommand: unimplemented 0x%x", command);
  return false;
}

bool CanvasAppliance::appliancePendingChanges() const
{
  return m_extCanvas ? m_extCanvas->pendingChanges() : false;
}

bool CanvasAppliance::applianceSave(const QString & __filePath)
{
  // ask for file name if not given
  if(__filePath.isEmpty())
  {
    QString fwFilePath = FotowallFile::getSaveFotowallFile(m_extCanvas->filePath());
    if(fwFilePath.isNull()) return false;
    return FotowallFile::saveV2(fwFilePath, m_extCanvas);
  }

  return FotowallFile::saveV2(__filePath, m_extCanvas);
}

QMenu * CanvasAppliance::createArrangeMenu()
{
  QMenu * menu = new QMenu(m_dummyWidget);
  QAction * aAU = new QAction(tr("Random Placement"), menu);
  aAU->setShortcut(QKeySequence("CTRL+SHIFT+R"));
  connect(aAU, SIGNAL(triggered()), this, SLOT(slotArrangeRandom()));
  menu->addAction(aAU);
  QAction * aAS = new QAction(tr("Shaped Placement"), menu);
  aAS->setShortcut(QKeySequence("CTRL+SHIFT+S"));
  aAS->setEnabled(false);
  connect(aAS, SIGNAL(triggered()), this, SLOT(slotArrangeShaped()));
  menu->addAction(aAS);
  QAction * aAC = new QAction(tr("Color Collage"), menu);
  aAC->setShortcut(QKeySequence("CTRL+SHIFT+C"));
  aAC->setEnabled(false);
  connect(aAC, SIGNAL(triggered()), this, SLOT(slotArrangeColorCollage()));
  menu->addAction(aAC);
  menu->addSeparator();
  QAction * aForceField = new QAction(tr("Enable force field"), menu);
  aForceField->setShortcut(QKeySequence("CTRL+SHIFT+F"));
  aForceField->setCheckable(true);
  aForceField->setChecked(m_extCanvas->forceFieldEnabled());
  connect(aForceField, SIGNAL(toggled(bool)), this, SLOT(slotArrangeForceField(bool)));
  menu->addAction(aForceField);
  return menu;
}

QMenu * CanvasAppliance::createBackgroundMenu()
{
  QMenu * menu = new QMenu(m_dummyWidget);
  m_gBackModeGroup = new QActionGroup(menu);
  connect(m_gBackModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(slotSetBackMode(QAction *)));
  QAction * aNone = new QAction(tr("None"), menu);
  aNone->setToolTip(tr("Transparency can be saved to PNG images only."));
  aNone->setShortcut(QKeySequence("CTRL+1"));
  aNone->setProperty("modeId", (int)Canvas::BackNone);
  aNone->setCheckable(true);
  aNone->setActionGroup(m_gBackModeGroup);
  menu->addAction(aNone);
  QAction * aBlack = new QAction(tr("Black"), menu);
  aBlack->setShortcut(QKeySequence("CTRL+2"));
  aBlack->setProperty("modeId", (int)Canvas::BackBlack);
  aBlack->setCheckable(true);
  aBlack->setActionGroup(m_gBackModeGroup);
  menu->addAction(aBlack);
  QAction * aWhite = new QAction(tr("White"), menu);
  aWhite->setShortcut(QKeySequence("CTRL+3"));
  aWhite->setProperty("modeId", (int)Canvas::BackWhite);
  aWhite->setCheckable(true);
  aWhite->setActionGroup(m_gBackModeGroup);
  menu->addAction(aWhite);
  QAction * aGradient = new QAction(tr("Gradient"), menu);
  aGradient->setShortcut(QKeySequence("CTRL+4"));
  aGradient->setProperty("modeId", (int)Canvas::BackGradient);
  aGradient->setCheckable(true);
  aGradient->setActionGroup(m_gBackModeGroup);
  menu->addAction(aGradient);
  menu->addSeparator();
  m_gBackContentAction = new QAction(tr("Content"), menu);
  connect(m_gBackContentAction, SIGNAL(toggled(bool)), this, SLOT(slotBackContentRemove(bool)));
  m_gBackContentAction->setToolTip(tr("Double click on any content to put it on background."));
  m_gBackContentAction->setEnabled(false);
  m_gBackContentAction->setCheckable(true);
  menu->addAction(m_gBackContentAction);
  QMenu * mScaling = new QMenu(tr("Content Aspect Ratio"), menu);
  m_gBackRatioGroup = new QActionGroup(menu);
  connect(m_gBackRatioGroup, SIGNAL(triggered(QAction *)), this, SLOT(slotSetBackRatio(QAction *)));
  menu->addMenu(mScaling);
  QAction * aRatioKeepEx = new QAction(tr("Keep proportions by expanding"), mScaling);
  aRatioKeepEx->setShortcut(QKeySequence("CTRL+5"));
  aRatioKeepEx->setProperty("ratioId", (int)Qt::KeepAspectRatioByExpanding);
  aRatioKeepEx->setCheckable(true);
  aRatioKeepEx->setActionGroup(m_gBackRatioGroup);
  mScaling->addAction(aRatioKeepEx);
  QAction * aRatioKeep = new QAction(tr("Keep proportions"), mScaling);
  aRatioKeep->setShortcut(QKeySequence("CTRL+6"));
  aRatioKeep->setProperty("ratioId", (int)Qt::KeepAspectRatio);
  aRatioKeep->setCheckable(true);
  aRatioKeep->setActionGroup(m_gBackRatioGroup);
  mScaling->addAction(aRatioKeep);
  QAction * aRatioIgnore = new QAction(tr("Ignore proportions"), mScaling);
  aRatioIgnore->setShortcut(QKeySequence("CTRL+7"));
  aRatioIgnore->setProperty("ratioId", (int)Qt::IgnoreAspectRatio);
  aRatioIgnore->setCheckable(true);
  aRatioIgnore->setActionGroup(m_gBackRatioGroup);
  mScaling->addAction(aRatioIgnore);
  menu->addSeparator();
  QMenu * mPerspective = new QMenu(tr("View Perspective"), menu);
  menu->addMenu(mPerspective);
  QWidgetAction * aPerspective = new QWidgetAction(mPerspective);
  PaneWidget * perspWidget = new PaneWidget;
  perspWidget->setFixedSize(71, 71);
  perspWidget->setRange(QRectF(-30.0, -30.0, 60.0, 60.0));
  new PE_PaneWidget(perspWidget, m_extCanvas, "perspective", perspWidget);
  aPerspective->setDefaultWidget(perspWidget);
  mPerspective->addAction(aPerspective);
  QMenu * mRotation = new QMenu(tr("View Rotation"), menu);
  menu->addMenu(mRotation);
  QWidgetAction * aRotation = new QWidgetAction(mRotation);
  QSlider * rotWidget = new QSlider(Qt::Horizontal);
  rotWidget->setMinimum(-40);
  rotWidget->setValue(0);
  rotWidget->setMaximum(40);
  rotWidget->setMinimumWidth(81 * 2);
  rotWidget->setProperty("noremap", true);
  new PE_AbstractSlider(rotWidget, m_extCanvas, "rotation", rotWidget);
  aRotation->setDefaultWidget(rotWidget);
  mRotation->addAction(aRotation);

  // initially check the action
  slotBackConfigChanged();
  return menu;
}

QMenu * CanvasAppliance::createDecorationMenu()
{
  QMenu * menu = new QMenu(m_dummyWidget);
  QAction * aTop = new QAction(tr("Top bar"), menu);
  aTop->setCheckable(true);
  aTop->setChecked(m_extCanvas->topBarEnabled());
  connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotDecoTopBar(bool)));
  menu->addAction(aTop);
  QAction * aBottom = new QAction(tr("Bottom bar"), menu);
  aBottom->setCheckable(true);
  aBottom->setChecked(m_extCanvas->bottomBarEnabled());
  connect(aBottom, SIGNAL(toggled(bool)), this, SLOT(slotDecoBottomBar(bool)));
  menu->addAction(aBottom);
  menu->addSeparator();
  QAction * aSetTitle = new QAction(tr("Set title..."), menu);
  connect(aSetTitle, SIGNAL(triggered()), this, SLOT(slotDecoSetTitle()));
  menu->addAction(aSetTitle);
  QAction * aClearTitle = new QAction(tr("Clear title"), menu);
  connect(aClearTitle, SIGNAL(triggered()), this, SLOT(slotDecoClearTitle()));
  menu->addAction(aClearTitle);
  return menu;
}

void CanvasAppliance::setNormalProject()
{
  m_extCanvas->clearMarkers();
  m_extCanvas->modeInfo()->setFixedSizeInches();
  m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeNormal);
}

void CanvasAppliance::setExactSizeProject(bool usePrevious)
{
  m_extCanvas->clearMarkers();
  if(!usePrevious || !m_extCanvas->modeInfo()->fixedSize())
  {
    ExactSizeDialog sizeDialog;
    QSizeF prevSizeInches = m_extCanvas->modeInfo()->fixedSizeInches();
    if(!prevSizeInches.isEmpty())
    {
      bool cm = !sizeDialog.ui.unityComboBox->currentIndex();
      sizeDialog.ui.widthSpinBox->setValue(cm ? prevSizeInches.width() * 2.54 : prevSizeInches.width());
      sizeDialog.ui.heightSpinBox->setValue(cm ? prevSizeInches.height() * 2.54 : prevSizeInches.height());
    }
    QPointF screenDpi = m_extCanvas->modeInfo()->screenDpi();
    if(screenDpi.x() == screenDpi.y())
      sizeDialog.ui.screenDpi->setValue((int)screenDpi.x());
    else
      sizeDialog.ui.screenDpi->setSpecialValueText(QString("%1, %2").arg(screenDpi.x()).arg(screenDpi.y()));
    if(sizeDialog.exec() != QDialog::Accepted) return;
    float w = sizeDialog.ui.widthSpinBox->value();
    float h = sizeDialog.ui.heightSpinBox->value();
    int printDpi = sizeDialog.ui.printDpi->value();
    bool landscape = sizeDialog.ui.landscapeCheckBox->isChecked();
    bool cm = !sizeDialog.ui.unityComboBox->currentIndex();
    m_extCanvas->modeInfo()->setPrintLandscape(landscape);
    m_extCanvas->modeInfo()->setPrintDpi(printDpi);
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(cm ? (double)w / 2.54 : w, cm ? (double)h / 2.54 : h));
  }
  m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeExactSize);
}

void CanvasAppliance::setWallpaperProject()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
  QSize wallSize = QGuiApplication::primaryScreen()->size();
#else
  QSize wallSize = QApplication::desktop()->screenGeometry().size();
#endif
  QPointF screenDpi = m_extCanvas->modeInfo()->screenDpi();
  if(!wallSize.isValid() || screenDpi.x() <= 0 || screenDpi.y() <= 0) return;
  m_extCanvas->clearMarkers();
  m_extCanvas->modeInfo()->setFixedSizeInches(
      QSizeF((qreal)wallSize.width() / screenDpi.x(), (qreal)wallSize.height() / screenDpi.y()));
  m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeWallpaper);
}

void CanvasAppliance::setCDProject()
{
  m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(4.75, 4.75));
  m_extCanvas->modeInfo()->setPrintLandscape(false);
  m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeCD);
}

void CanvasAppliance::setDVDProject()
{
  m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(10.83, 7.2));
  m_extCanvas->modeInfo()->setPrintLandscape(true);
  m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeDVD);
}

void CanvasAppliance::updateProjectMode()
{
  m_extCanvas->adjustSceneSize();
  m_extCanvas->clearMarkers();
  switch(m_extCanvas->modeInfo()->projectMode())
  {
    case CanvasModeInfo::ModeDVD:
      m_extCanvas->setDVDMarkers();
      configurePrint(true);
      break;
    case CanvasModeInfo::ModeCD:
      m_extCanvas->setCDMarkers();
      configurePrint(true);
      break;
    case CanvasModeInfo::ModeWallpaper:
      configurePrint(false);
      break;
    case CanvasModeInfo::ModeExactSize:
      configurePrint(true);
      break;
    case CanvasModeInfo::ModeNormal:
      configurePrint(false);
      break;
  }
}

CanvasModeInfo::Mode CanvasAppliance::projectModeFromComboIndex(const int index) const
{
  switch(index)
  {
    case 0:
      return CanvasModeInfo::ModeNormal;
    case 1:
      return CanvasModeInfo::ModeExactSize;
    case 2:
      return CanvasModeInfo::ModeWallpaper;
    case 3:
      return CanvasModeInfo::ModeCD;
    case 4:
      return CanvasModeInfo::ModeDVD;
  }
  throw std::runtime_error("Missing project mode mappping!");
}

int CanvasAppliance::projectComboIndexFromMode(const CanvasModeInfo::Mode mode) const
{
  switch(mode)
  {
    case CanvasModeInfo::ModeNormal:
      return 0;
    case CanvasModeInfo::ModeExactSize:
      return 1;
    case CanvasModeInfo::ModeWallpaper:
      return 2;
    case CanvasModeInfo::ModeCD:
      return 3;
    case CanvasModeInfo::ModeDVD:
      return 4;
  }
  throw std::runtime_error("Missing project mode mappping!");
}

void CanvasAppliance::configurePrint(bool enabled)
{
  ui.exportButton->setText(enabled ? tr("Print") : tr("Export"));
}

void CanvasAppliance::slotAddCanvas()
{
  QStringList fwFilePaths = FotowallFile::getLoadFotowallFiles();
  if(fwFilePaths.isEmpty()) return;
  m_extCanvas->addCanvasViewContent(fwFilePaths);
  setFocusToScene();
}

void CanvasAppliance::slotAddPicture()
{
  // make up the default load path (stored as 'Fotowall/LoadImagesDir')
  QString defaultLoadPath = App::settings->value("Fotowall/LoadImagesDir").toString();

#if defined(__EMSCRIPTEN__)
  auto fileContentReady = [this](const QString & fileName, const QByteArray & fileContent)
  {
    if(fileName.isEmpty())
    {
      // No file was selected
    }
    else
    {
      // Use fileName and fileContent
      QImage img;
      img.loadFromData(fileContent);
      m_extCanvas->addPictureContent(fileName, img);
    }
  };
  QFileDialog::getOpenFileContent(tr("Images (%1)").arg(App::supportedImageFormats()), fileContentReady);
#else
  // ask the file name, validate it, store back to settings and load the file
  QStringList picFilePaths = QFileDialog::getOpenFileNames(
      0, tr("Add Pictures to the Canvas"), defaultLoadPath,
      tr("Images (%1)").arg(App::supportedImageFormats()) /*, 0, QFileDialog::DontResolveSymlinks*/);
  if(picFilePaths.isEmpty()) return;
  App::settings->setValue("Fotowall/LoadImagesDir", QFileInfo(picFilePaths[0]).absolutePath());
  m_extCanvas->addPictureContent(picFilePaths);
#endif
  setFocusToScene();
}

void CanvasAppliance::slotAddText()
{

  m_extCanvas->addTextContent();
  setFocusToScene();
}

void CanvasAppliance::slotAddWebcam()
{
  int webcamIndex = sender()->property("index").toInt();
  m_extCanvas->addWebcamContent(webcamIndex);
  setFocusToScene();
}

void CanvasAppliance::slotAddWordcloud()
{
  m_extCanvas->addWordcloudContent();
  setFocusToScene();
}

void CanvasAppliance::slotSearchPicturesToggled(bool visible)
{
  containerValueSet(App::CC_ShowPictureSearch, visible);
}

void CanvasAppliance::slotBackContentRemove(bool checked)
{
  if(!checked) m_extCanvas->clearBackContent();
}

void CanvasAppliance::setProjectMode(const CanvasModeInfo::Mode mode)
{
  // Prevent combobox from emiting signals while changing its index
  bool oldState = ui.projectCombo->blockSignals(true);
  ui.projectCombo->setCurrentIndex(projectComboIndexFromMode(mode));
  ui.projectCombo->blockSignals(oldState);

  switch(mode)
  {
    case CanvasModeInfo::ModeNormal:
      setNormalProject();
      break;
    case CanvasModeInfo::ModeExactSize:
      setExactSizeProject(!sender());
      break;
    case CanvasModeInfo::ModeWallpaper:
      setWallpaperProject();
      break;
    case CanvasModeInfo::ModeCD:
      setCDProject();
      break;
    case CanvasModeInfo::ModeDVD:
      setDVDProject();
      break;
  }
}

void CanvasAppliance::setProjectMode(const CanvasModeInfo & mode)
{
  // Prevent combobox from emiting signals while changing its index
  bool oldState = ui.projectCombo->blockSignals(true);
  ui.projectCombo->setCurrentIndex(projectComboIndexFromMode(mode.projectMode()));
  ui.projectCombo->blockSignals(oldState);

  *m_extCanvas->modeInfo() = mode;
  updateProjectMode();
}

void CanvasAppliance::slotProjectComboActivated(int index)
{
  CanvasModeInfo previousMode = *m_extCanvas->modeInfo();
  auto & commandStack = m_extCanvas->commandStack();
  commandStack.beginMacro("Set Project Mode");
  m_extCanvas->pushCurrentContentPositionToCommandStack();
  setProjectMode(projectModeFromComboIndex(index));
  m_extCanvas->commandStack().push(new ProjectModeCommand(this, previousMode, *m_extCanvas->modeInfo()));
  commandStack.endMacro();
}

void CanvasAppliance::slotSetBackMode(QAction * action)
{
  Canvas::BackMode mode = (Canvas::BackMode)action->property("modeId").toInt();
  BackgroundModeCommand * command = new BackgroundModeCommand(m_extCanvas, m_extCanvas->backMode(), mode);
  command->redo();
  m_extCanvas->commandStack().push(command);
}

void CanvasAppliance::slotSetBackRatio(QAction * action)
{
  Qt::AspectRatioMode ratio = (Qt::AspectRatioMode)action->property("ratioId").toInt();
  BackgroundRatioCommand * command = new BackgroundRatioCommand(m_extCanvas, m_extCanvas->backContentRatio(), ratio);
  command->redo();
  m_extCanvas->commandStack().push(command);
}

void CanvasAppliance::slotArrangeForceField(bool checked)
{
  m_extCanvas->setForceFieldEnabled(checked);
}

void CanvasAppliance::slotArrangeColorCollage()
{
  HERE
}

void CanvasAppliance::slotArrangeRandom()
{
  m_extCanvas->randomizeContents(true, true, true);
}

void CanvasAppliance::slotArrangeShaped()
{
  HERE
}

void CanvasAppliance::slotDecoTopBar(bool checked)
{
  if(m_extCanvas->topBarEnabled() && checked || !m_extCanvas->topBarEnabled() && !checked) return;

  QAction * aTop = qobject_cast<QAction *>(sender());
  DecoTopBarCommand * c = new DecoTopBarCommand(m_extCanvas, aTop, checked);
  m_extCanvas->commandStack().push(c);
}

void CanvasAppliance::slotDecoBottomBar(bool checked)
{
  if(m_extCanvas->bottomBarEnabled() && checked || !m_extCanvas->bottomBarEnabled() && !checked) return;

  QAction * aBottom = qobject_cast<QAction *>(sender());
  DecoBottomBarCommand * c = new DecoBottomBarCommand(m_extCanvas, aBottom, checked);
  m_extCanvas->commandStack().push(c);
}

void CanvasAppliance::slotDecoSetTitle()
{
  // change title dialog
  bool ok = false;
  QString title =
      QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_extCanvas->titleText(), &ok);
  if(ok)
  {
    DecoTitleCommand * c = new DecoTitleCommand(m_extCanvas, title);
    m_extCanvas->commandStack().push(c);
  }

  // set a dummy title, if none
  if(m_extCanvas->titleText().isEmpty()) m_extCanvas->setTitleText("...");
}

void CanvasAppliance::slotDecoClearTitle()
{
  DecoTitleCommand * c = new DecoTitleCommand(m_extCanvas, QString());
  m_extCanvas->commandStack().push(c);
}

bool CanvasAppliance::slotFileLoad()
{
  return App::workflow->loadCanvas_A();
}

bool CanvasAppliance::slotFileSave()
{
  return applianceSave();
}

bool CanvasAppliance::slotFileExport()
{
#if defined(HAS_EXPORTDIALOG)
  bool printPreferred = m_extCanvas->modeInfo()->projectMode() != CanvasModeInfo::ModeNormal
                        && m_extCanvas->modeInfo()->projectMode() != CanvasModeInfo::ModeWallpaper;
  return ExportWizard(m_extCanvas, printPreferred).exec();
#else
  return false;
#endif
}

void CanvasAppliance::slotEditContent(AbstractContent * content)
{
  // handle Canvas
  if(CanvasViewContent * cvc = dynamic_cast<CanvasViewContent *>(content))
  {
    App::workflow->stackSlaveCanvas_A(cvc);
    return;
  }

  // handle Wordcloud
  if(WordcloudContent * wc = dynamic_cast<WordcloudContent *>(content))
  {
#if defined(HAS_WORDCLOUD_APPLIANCE)
    App::workflow->stackSlaveWordcloud_A(wc);
#else
    Q_UNUSED(wc);
    ButtonsDialog info("WordcloudMissingInfo", tr("Wordcloud Editor"),
                       tr("Would be great to have a Wordcloud editor, but we can only move things around for now."),
                       QDialogButtonBox::Ok, true, true);
    info.execute();
#endif
    return;
  }
}

void CanvasAppliance::slotBackConfigChanged()
{
  // update ratios to reflect background mode
  m_gBackModeGroup->blockSignals(true);
  int mode = (int)m_extCanvas->backMode();
  m_gBackModeGroup->actions()[qMax(0, mode)]->setChecked(true);
  m_gBackModeGroup->blockSignals(false);

  // update check to reflect background content presence
  m_gBackContentAction->blockSignals(true);
  bool hasBackContent = m_extCanvas->backContent();
  m_gBackContentAction->setChecked(hasBackContent);
  m_gBackContentAction->setEnabled(hasBackContent);
  m_gBackContentAction->blockSignals(false);

  // update ratios to reflect content ratio
  Qt::AspectRatioMode cRatio = m_extCanvas->backContentRatio();
  if(cRatio == Qt::KeepAspectRatioByExpanding)
    m_gBackRatioGroup->actions()[0]->setChecked(true);
  else if(cRatio == Qt::KeepAspectRatio)
    m_gBackRatioGroup->actions()[1]->setChecked(true);
  else if(cRatio == Qt::IgnoreAspectRatio)
    m_gBackRatioGroup->actions()[2]->setChecked(true);
}

void CanvasAppliance::slotShowPropertiesWidget(QWidget * widget)
{
#if defined(MOBILE_UI)
  // hide the topbar in case of selected content
  containerValueSet(App::CC_HideTopBar, (bool)(widget ? true : false));
#endif

  // delete current Properties content
  QLayoutItem * prevItem = ui.propLayout->takeAt(0);
  if(prevItem)
  {
    delete prevItem->widget();
    delete prevItem;
  }
  if(!ui.propLayout->isEmpty())
    qWarning("CanvasAppliance::slotShowPropertiesWidget: problem in the properties layout: not empty (%d)",
             ui.propLayout->count());

  // show the Properties container with new content and title
  if(widget)
  {
#if !defined(MOBILE_UI)
    ui.canvasPropertiesBox->collapse();
#endif
    widget->setParent(ui.propertiesBox);
    ui.propLayout->addWidget(widget);
    ui.propertiesBox->setTitle(widget->windowTitle());
#if !defined(MOBILE_UI)
    ui.propertiesBox->expand();
#endif
  }
  // or show the Canvas container
  else
  {
#if !defined(MOBILE_UI)
    ui.propertiesBox->collapse();
    ui.canvasPropertiesBox->expand();
#else
    ui.propertiesBox->disappear();
#endif
  }
}

void CanvasAppliance::slotFilePathChanged()
{
  windowTitleSet(m_extCanvas->prettyBaseName());
}

void CanvasAppliance::slotVerifyVideoInputs(int count)
{
  // delete previous buttons
  qDeleteAll(m_webcamButtons);
  m_webcamButtons.clear();

  // create new buttons, indexed and connected
  for(int i = 0; i < count; i++)
  {
    PixmapButton * p = new PixmapButton(ui.addContentBox);
    p->setFixedSize(QSize(50, 50));
    p->setPixmap(QPixmap(":/data/insert-webcam.png"));
    p->setHoverPixmap(QPixmap(":/data/insert-overlay-plus.png"));
    p->setToolTip(tr("Add Webcam"));
    p->setProperty("index", i);
    connect(p, SIGNAL(clicked()), this, SLOT(slotAddWebcam()));
    ui.addWebcamLayout->addWidget(p);
    m_webcamButtons.append(p);
  }
}

void CanvasAppliance::slotUndoTextChanged(const QString & text)
{
  QString tooltip = tr("Undo");
  if(text.size()) { tooltip += " " + text; }
  tooltip += " (" + ui.undoButton->shortcut().toString() + ")";
  ui.undoButton->setToolTip(tooltip);
  ui.undoButton->setEnabled(m_extCanvas->commandStack().canUndo());
}

void CanvasAppliance::slotRedoTextChanged(const QString & text)
{
  QString tooltip = tr("Redo");
  if(text.size()) { tooltip += " " + text; }
  tooltip += " (" + ui.redoButton->shortcut().toString() + ")";
  ui.redoButton->setToolTip(tooltip);
  ui.redoButton->setEnabled(m_extCanvas->commandStack().canRedo());
}
