/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "App/MainWindow.h"

#include "App.h"
#include "Hardware3DTest.h"
#include "OnlineServices.h"
#include "PictureSearchWidget.h"
#include "SceneView.h"
#include "Settings.h"
#include "Shared/BreadCrumbBar.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/RenderOpts.h"
#include "Workflow.h"
#include "ui_MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QScreen>
#include <QVariant>

// const strings
#define FOTOWALL_FEEDBACK_LANGS "en,it,fr"
#define FOTOWALL_FEEDBACK_SCHEME "https"
#define FOTOWALL_FEEDBACK_SERVER "www.enricoros.com"
#define FOTOWALL_FEEDBACK_PATH "/opensource/fotowall/feedback/send.php"

// uncomment the following to disable the blurring effects behind the windows
#define DISABLE_BEHIND_BLUR

MainWindow::MainWindow(QWidget * parent)
: PlugGui::Container(parent), ui(new Ui::MainWindow()), m_networkAccessManager(new QNetworkAccessManager),
  m_navigationLayout(0), m_pictureSearch(0), m_applyingAccelState(false)
{
  // setup widget
  applianceSetTitle(QString());
  setWindowIcon(QIcon(":/data/fotowall.png"));

  // init ui
  ui->setupUi(this);
  ui->topBar->setFixedHeight(App::TopBarHeight);

#if defined(Q_OS_LINUX)
  // DISABLE, it's too UNSTABLE
  ui->transpBox->setEnabled(false);
#else
  ui->transpBox->setEnabled(false);
  ui->transpBox->setVisible(false);
#endif
  ui->accelBox->setEnabled(ui->sceneView->supportsOpenGL());
  ui->accelTestButton->setEnabled(ui->sceneView->supportsOpenGL());
  ui->applianceSidebar->hide();
  ui->sceneView->setFocus();
  connect(ui->sceneView, SIGNAL(heavyRepaint()), this, SLOT(slotRenderingSlow()));

  // create the navigation layout
  m_navigationLayout = new QGridLayout;
  m_navigationLayout->setContentsMargins(0, 0, 0, 0);
  m_navigationLayout->setSpacing(3);
  ui->navBar->setLayout(m_navigationLayout);

  // create the Workflow navigation bar
  BreadCrumbBar * workflowBar = new BreadCrumbBar(ui->sceneView);
  workflowBar->setObjectName(QString::fromUtf8("applianceNavBar"));
  workflowBar->setClickableLeaves(false);
  workflowBar->setBackgroundOffset(-1);
  addNavigationWidget(workflowBar, 0, Qt::AlignLeft);

  // create the Help bar
  BreadCrumbBar * helpBar = new BreadCrumbBar(ui->sceneView);
  connect(helpBar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotHelpBarClicked(quint32)));
  helpBar->setBackgroundOffset(1);
  helpBar->addNode(1, tr(" Help "), 0);
  addNavigationWidget(helpBar, 0, Qt::AlignRight);

  // show (with last geometry)
#if defined(Q_OS_SYMBIAN)
  showFullScreen();
#else
  if(!restoreGeometry(App::settings->value("Fotowall/Geometry").toByteArray()))
  {
    QRect desktopGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    resize(2 * desktopGeometry.width() / 3, 2 * desktopGeometry.height() / 3);
    showMaximized();
  }
  else
    show();
#endif

  // re-apply transparency
#if defined(Q_OS_LINUX)
  // DISABLE, it's too UNSTABLE
  // if (App::settings->value("Fotowall/Tranlucent", false).toBool())
  //    ui->transpBox->setChecked(true);
#endif

  // start the workflow
  new Workflow((PlugGui::Container *)this, workflowBar);

  // create the online services
  new OnlineServices(m_networkAccessManager);

#if 0
    // start with the Help appliance the first time
    if (App::settings->firstTime())
        App::workflow->stackHelpAppliance();
#endif
}

MainWindow::~MainWindow()
{
  // save window geometry
  if(!isMaximized() && !isFullScreen())
    App::settings->setValue("Fotowall/Geometry", saveGeometry());
  else
    App::settings->remove("Fotowall/Geometry");

  // delete everything
  delete App::workflow;
  delete App::onlineServices;
  delete m_networkAccessManager;
  delete ui;
}

QSize MainWindow::sceneViewSize() const
{
  return ui->sceneView->viewport()->size();
}

void MainWindow::applianceSetTitle(const QString & title)
{
  QString tString = title.isEmpty() ? QString() : title + " - ";
  tString += QCoreApplication::applicationName() + " ";
  if(title.isEmpty()) tString += tr("'RETRO' ");
  tString += QCoreApplication::applicationVersion();
  setWindowTitle(tString);
}

void MainWindow::applianceSetScene(AbstractScene * scene)
{
  ui->sceneView->setScene(scene);
}

static void removeLayoutChildWidges(QLayout * layout)
{
  while(QLayoutItem * item = layout->takeAt(0))
  {
    if(QWidget * oldWidget = item->widget()) oldWidget->setVisible(false);
    delete item;
  }
}

void MainWindow::applianceSetTopbar(const QList<QWidget *> & widgets)
{
  // clear the topbar layout hiding all widgets
  removeLayoutChildWidges(ui->applianceLeftBarLayout);
  removeLayoutChildWidges(ui->applianceRightBarLayout);

  // add the widgets to the topbar and show them
  foreach(QWidget * widget, widgets)
  {
    if(widget->property("@rightBar").toBool())
      ui->applianceRightBarLayout->addWidget(widget);
    else
      ui->applianceLeftBarLayout->addWidget(widget);
    widget->setFixedHeight(App::TopBarHeight);
    widget->setVisible(true);
  }
}

void MainWindow::applianceSetSidebar(QWidget * widget)
{
  // clear the sidebar layout hiding any widget
  removeLayoutChildWidges(ui->applianceSidebarLayout);

  // completely hide the sidebar if no widget
  ui->applianceSidebar->setVisible(widget);

  // if any, add the widget to the sidebar
  if(widget)
  {
    ui->applianceSidebarLayout->addWidget(widget);
    widget->show();
  }
}

void MainWindow::applianceSetCentralwidget(QWidget * widget)
{
  if(widget) qWarning("MainWindow::applianceSetCentralwidget: unsupported");
}

void MainWindow::applianceSetValue(quint32 key, const QVariant & value)
{
  if(key == App::CC_ShowPictureSearch)
  {

    // destroy if needed
    bool visible = value.toBool();
    if(!visible && m_pictureSearch)
    {
      m_pictureSearch->deleteLater();
      m_pictureSearch = 0;
      return;
    }

    // create if needed
    if(visible && !m_pictureSearch)
    {
      m_pictureSearch = new PictureSearchWidget(m_networkAccessManager);
      connect(m_pictureSearch, SIGNAL(requestClosure()), this, SLOT(slotClosePictureSearch()), Qt::QueuedConnection);
      m_pictureSearch->setWindowIcon(QIcon(":/data/insert-download.png"));
      m_pictureSearch->setWindowFlags(Qt::Tool);
      QPoint newPos = mapToGlobal(ui->sceneView->pos()) + QPoint(-20, 10);
      if(newPos.x() < 0) newPos.setX(0);
      m_pictureSearch->move(newPos);
      m_pictureSearch->layout()->activate();
      m_pictureSearch->resize(50, 50);
      m_pictureSearch->show();
      m_pictureSearch->setFocus();
      return;
    }
  }
  else
    qWarning("MainWindow::applianceSetValue: unknown key 0x%x", key);
}

void MainWindow::applianceSetFocusToScene()
{
  ui->sceneView->setFocus(Qt::OtherFocusReason);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
  event->setAccepted(App::workflow->requestExit());
}

void MainWindow::slotClosePictureSearch()
{
  App::workflow->applianceCommand(App::AC_ClosePicureSearch);
}

void MainWindow::slotHelpBarClicked(quint32 id)
{
  if(id == 1) App::workflow->stackHelpAppliance();
}

void MainWindow::slotRenderingSlow()
{
  // don't act anymore
  disconnect(ui->sceneView, SIGNAL(heavyRepaint()), this, SLOT(slotRenderingSlow()));

  // draw attenction to the testing button
  if(ui->sceneView->supportsOpenGL())
  {
    ui->modeWidget->setChecked(true);
    ui->accelTestButton->drawAttenction();
  }
}

void MainWindow::addNavigationWidget(QWidget * widget, int row, Qt::Alignment alignment)
{
  // add the widget at the right place in the grid layout
  if(alignment == Qt::AlignRight)
    m_navigationLayout->addWidget(widget, row, 1, 1, 1, Qt::AlignRight);
  else if(alignment & Qt::AlignHCenter)
    m_navigationLayout->addWidget(widget, row, 0, 1, 2, Qt::AlignLeft);
  else
    m_navigationLayout->addWidget(widget, row, 0, 1, 1, Qt::AlignLeft);

  // ensure the widget is shown
  widget->show();
}

void MainWindow::removeNavigationWidget(QWidget * widget)
{
  widget->hide();
  m_navigationLayout->removeWidget(widget);
  widget->setParent(0);
}

bool MainWindow::on_accelTestButton_clicked()
{
  // ask for confirmation
  if(!m_applyingAccelState)
  {
    ButtonsDialog d("TestOpenGL", tr("Accelerated Rendering"),
                    tr("OpenGL accelerates graphics. However it's not supported by every system.<br><b>Do you want to "
                       "do an acceleration test?</b>"),
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel, true, false);
    d.setIcon(QStyle::SP_MessageBoxQuestion);
    if(d.execute() == QDialogButtonBox::Cancel) return false;
  }

  // run the Hardware Test
  Hardware3DTest testDialog;
  Hardware3DTest::ExitState choice = testDialog.run();
  if(choice == Hardware3DTest::Canceled) return false;

  // apply the user choice
  bool setGL = choice == Hardware3DTest::UseOpenGL;
  App::settings->setValue("OpenGL/Tested", true);
  App::settings->setValue("OpenGL/TestResult", setGL);
  m_applyingAccelState = true;
  ui->accelBox->setChecked(setGL);
  m_applyingAccelState = false;
  return setGL;
}

void MainWindow::on_accelBox_toggled(bool checked)
{
  // ask for confirmation/testing when enabling opengl
  if(checked && !m_applyingAccelState)
  {
    bool tested = App::settings->contains("OpenGL/Tested");

    QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                                                | (tested ? QDialogButtonBox::NoButton : QDialogButtonBox::Retry);
    ButtonsDialog input("EnableOpenGL", tr("OpenGL"),
                        tr("OpenGL accelerates graphics, but it doesn't work on some systems.<br> - if it feels "
                           "slower, make sure that your driver accelerates OpenGL<br> - if Fotowall stops responding "
                           "after switching to OpenGL, don't enable OpenGL next time"),
                        buttons, true, tested);
    input.setIcon(QStyle::SP_MessageBoxInformation);
    if(!tested) input.setButtonText(QDialogButtonBox::Retry, tr("Test OpenGL"));
    QDialogButtonBox::StandardButton choice = input.execute();

    // action canceled
    if(choice == QDialogButtonBox::Cancel)
    {
      ui->accelBox->setChecked(false);
      return;
    }

    // testing required
    if(choice == QDialogButtonBox::Retry)
    {
      m_applyingAccelState = true;
      checked = on_accelTestButton_clicked();
      m_applyingAccelState = false;
    }
  }

#if QT_VERSION < 0x040600
  // WORKAROUND Qt <= 4.6-beta1: toggle transparency with opengl
  ui->transpBox->setChecked(false);
#endif

  // set opengl state
  ui->sceneView->setOpenGL(checked);

  // save opengl state
  RenderOpts::OpenGLWindow = ui->sceneView->openGL();
}

#if !defined(DISABLE_BEHIND_BLUR)
#  if defined(Q_OS_LINUX)
/**
  Blur behind windows (on KDE4.5+)

  Uses a feature done for Plasma 4.5+ for hinting the window manager to draw
  blur behind the window.
*/
#    include <QX11Info>
#    include <X11/Xatom.h>
#    include <X11/Xlib.h>
static bool kde4EnableBlurBehindWindow(WId window, bool enable, const QRegion & region = QRegion())
{
  Display * dpy = QX11Info::display();
  Atom atom = XInternAtom(dpy, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);

  if(enable)
  {
    QVector<QRect> rects = region.rects();
    QVector<quint32> data;
    for(int i = 0; i < rects.count(); i++)
    {
      const QRect r = rects[i];
      data << r.x() << r.y() << r.width() << r.height();
    }

    XChangeProperty(dpy, window, atom, XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(data.constData()), data.size());
  }
  else { XDeleteProperty(dpy, window, atom); }
  return true;
}
#  elif defined(Q_OS_WIN)
/**
  Blur behind windows (on Windows Vista/7)

  The following code snippet has been borrowed from Jens of Qt Software / Nokia
  see: http://labs.qt.nokia.com/blogs/2009/09/15/using-blur-behind-on-windows/
  the license says: Use, modification and distribution is allowed without
  limitation, warranty, liability or support of any kind.
**/
#    include <QLibrary>
#    include <qt_windows.h>

// Dwm Data Structures
#    define DWM_BB_ENABLE 0x00000001 // fEnable has been specified
typedef struct _DWM_BLURBEHIND
{
  DWORD dwFlags;
  BOOL fEnable;
  HRGN hRgnBlur;
  BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;

// Dwm entry points
typedef HRESULT(WINAPI * PtrDwmIsCompositionEnabled)(BOOL * pfEnabled);
typedef HRESULT(WINAPI * PtrDwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND * pBlurBehind);
static PtrDwmIsCompositionEnabled pDwmIsCompositionEnabled = 0;
static PtrDwmEnableBlurBehindWindow pDwmEnableBlurBehindWindow = 0;

static bool dwmResolveLibs()
{
  if(!pDwmIsCompositionEnabled)
  {
    QLibrary dwmLib(QString::fromLatin1("dwmapi"));
    pDwmIsCompositionEnabled = (PtrDwmIsCompositionEnabled)dwmLib.resolve("DwmIsCompositionEnabled");
    pDwmEnableBlurBehindWindow = (PtrDwmEnableBlurBehindWindow)dwmLib.resolve("DwmEnableBlurBehindWindow");
  }
  return pDwmIsCompositionEnabled != 0;
}

static bool dwmEnableBlurBehindWindow(QWidget * widget, bool enable)
{
  bool result = false;
  if(dwmResolveLibs())
  {
    DWM_BLURBEHIND bb = {0};
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = enable;
    bb.hRgnBlur = NULL;
    HRESULT hr = pDwmEnableBlurBehindWindow((HWND)widget->winId(), &bb);
    if(SUCCEEDED(hr)) result = true;
  }
  return result;
}
#  endif
#endif

void MainWindow::on_transpBox_toggled(bool transparent)
{
#if !defined(DISABLE_BEHIND_BLUR)
#  if defined(Q_OS_WIN)
  static Qt::WindowFlags initialWindowFlags = windowFlags();
#  endif
#endif
  if(transparent)
  {
#if defined(Q_OS_LINUX)
    // one-time warning
    ButtonsDialog warning("EnableTransparency", tr("Transparency"),
                          tr("This feature requires compositing (compiz or kwin4) to work on Linux.<br>If you see a "
                             "black background then transparency is not supported on your system."),
                          QDialogButtonBox::Ok, true, true);
    warning.setIcon(QStyle::SP_MessageBoxInformation);
    warning.execute();
#endif

#if QT_VERSION < 0x040600
    // WORKAROUND Qt <= 4.6-beta1: toggle opengl with transparency
    ui->accelBox->setChecked(false);
#endif

    // go transparent
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // hint the render that we're transparent now
    RenderOpts::ARGBWindow = true;

    // enable blur behind
#if !defined(DISABLE_BEHIND_BLUR)
#  if defined(Q_OS_LINUX)
    kde4EnableBlurBehindWindow(winId(), true);
#  elif defined(Q_OS_WIN)
    if(!dwmEnableBlurBehindWindow(this, true))
    {
      // if blur fails, use a frameless window that's needed on XP for transparency
      setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
      show();
    }
#  endif
#endif
    // disable appliance background too
    if(App::workflow) App::workflow->applianceCommand(App::AC_ClearBackground);
  }
  else
  {
    // back to normal (non-alphaed) window
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_NoSystemBackground, false);

    // disable blur behind
#if !defined(DISABLE_BEHIND_BLUR)
#  if defined(Q_OS_LINUX)
    kde4EnableBlurBehindWindow(winId(), false);
#  elif defined(Q_OS_WIN)
    // disable no-border on windows
    setWindowFlags(initialWindowFlags);
    show();
#  endif
#endif
    // hint the render that we're opaque again
    RenderOpts::ARGBWindow = false;
  }
  // refresh the window
  update();

  // remember in settings
  App::settings->setValue("Fotowall/Tranlucent", transparent);
}
