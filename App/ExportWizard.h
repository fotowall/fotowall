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

#ifndef __ExportWizard_h__
#define __ExportWizard_h__

#include <QtCore/QMap>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QWizard>

class Canvas;
class QPrinter;
namespace Ui
{
class ExportWizard;
}

class ExportWizard : public QWizard
{
  Q_OBJECT
public:
  ExportWizard(Canvas * canvas, bool printPreferred);
  ~ExportWizard();

public Q_SLOTS:
  // the main functions
  void setWallpaper();
  void saveImage();
  void startPosterazor();
  bool printPaper();
  bool printPdf();
  void saveSvg();

protected:
  // manually sets a page
  void setPage(int pageId);

  // ::QWizard
  int nextId() const;

private:
  enum PageCode
  {
    PageMode = 0,
    PageWallpaper = 1,
    PageImage = 2,
    PagePosteRazor = 3,
    PagePrint = 4,
    PageSvg = 5,
    PagePdf = 6
  };
  QSizeF canvasNatInches() const;
  void initPageSizeNames();
  Ui::ExportWizard * m_ui;
  Canvas * m_canvas;
  bool m_printPreferred;
  int m_nextId;
  QSizeF m_printSizeInches;
  QPrinter * m_pdfPrinter;
  QMap<QPageSize::PageSizeId, QString> m_paperSizeNames;

private slots:
  // contents related
  void slotChoosePdfPage();
  void slotChoosePdfPath();
  void slotImageFromCanvas();
  void slotImageFromDpi();
  void slotPrintUnityChanged(int);
  void slotPrintSizeChanged();
  void slotPdfPreview();
  void slotPdfResChanged(int);
  void slotPdfUpdateGui();

  // wizard related
  void slotFinished(int);
  void slotModeButtonClicked();
  void slotOpenLink(const QString & address);
};

#endif
