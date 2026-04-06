/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      This code was inspired from an example on the Graphics Dojo page   *
 *      of the Qt Software Labs by Zack Rusin. The code was released       *
 *      under the GNU GPL version 2.0.                                     *
 *                                                                         *
 ***************************************************************************/

#include "GlowEffectWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

GlowEffectWidget::GlowEffectWidget(QWidget * parent)
: QWidget(parent), m_radius(5), m_tile(100, 100), m_mousePressed(false)
{
  m_tile.fill(Qt::white);
  QPainter tilePainter(&m_tile);
  QColor color(240, 240, 240);
  tilePainter.fillRect(0, 0, 50, 50, color);
  tilePainter.fillRect(50, 50, 50, 50, color);
}

void GlowEffectWidget::setPreviewImage(const QImage & preview)
{
  m_image = preview.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  setFixedSize(m_image.size());
  update();
}

void GlowEffectWidget::setGlowRadius(int radius)
{
  m_radius = qBound(0, radius, 99);
  update();
}

int GlowEffectWidget::glowRadius() const
{ return m_radius; }

template<int aprec, int zprec>
void expblur(QImage & img, int radius);

QImage GlowEffectWidget::glown(const QImage & image, int radius)
{
  QImage glownImage(image.size(), QImage::Format_ARGB32_Premultiplied);
  glownImage.fill(0x00);
  QPainter imagePainter(&glownImage);

  // NOTE: what's the half-difference supposed to do here?
  int x = 0; //(size().width() - blurred.size().width())/2;
  int y = 0; //(size().height() - blurred.size().height())/2;
  imagePainter.drawImage(qMax(x, 0), qMax(0, y), image);

  // ExpBlur with 0.16 fp for alpha and
  // 8.7 fp for state parameters zR,zG,zB and zA
  QImage blurred = image;
  expblur<16, 7>(blurred, radius);

  imagePainter.save();
  imagePainter.setCompositionMode(QPainter::CompositionMode_Plus);
  imagePainter.drawImage(qMax(0, x), qMax(0, y), blurred);
  imagePainter.restore();
  imagePainter.end();

  return glownImage;
}

QImage GlowEffectWidget::dropShadow(const QImage & orig,
                                    const QColor & shadowColor,
                                    int radius,
                                    int xOffset,
                                    int yOffset)
{
  // create a black copy of the original image
  QImage shadow = orig;
  QPainter shadowPainter(&shadow);
  shadowPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
  shadowPainter.fillRect(shadow.rect(), shadowColor);
  shadowPainter.end();

  // create an enlarged destination image
  QImage final(orig.width() + radius * 2, orig.height() + radius * 2, QImage::Format_ARGB32_Premultiplied);
  final.fill(0);
  QPainter p(&final);
  p.drawImage(radius, radius, shadow);
  p.end();

  // inline blur destination image
  expblur<16, 7>(final, radius / 2);

  // copy the offseted original image over it
  p.begin(&final);
  p.drawImage(radius - xOffset, radius - yOffset, orig);
  p.end();
  return final;
}

void GlowEffectWidget::mousePressEvent(QMouseEvent *)
{
  m_mousePressed = true;
  update();
}

void GlowEffectWidget::mouseReleaseEvent(QMouseEvent *)
{
  m_mousePressed = false;
  update();
}

void GlowEffectWidget::paintEvent(QPaintEvent * e)
{
  QImage image(size(), QImage::Format_ARGB32_Premultiplied);
  image.fill(0x00);

  QPainter imagePainter(&image);
  imagePainter.setClipRect(e->rect());
  imagePainter.drawTiledPixmap(rect(), m_tile);

  // NOTE: what's the half-difference supposed to do here?
  int x = 0; //(size().width()  - blurred.size().width())/2;
  int y = 0; //(size().height() - blurred.size().height())/2;

  imagePainter.drawImage(qMax(0, x), qMax(0, y), m_image);

  if(m_mousePressed)
  {
    // ExpBlur with 0.16 fp for alpha and
    // 8.7 fp for state parameters zR,zG,zB and zA
    QImage blurred = m_image;
    expblur<16, 7>(blurred, m_radius);

    imagePainter.save();
    imagePainter.setCompositionMode(QPainter::CompositionMode_Plus);
    imagePainter.drawImage(qMax(0, x), qMax(0, y), blurred);
    imagePainter.restore();
  }
  imagePainter.end();

  QPainter painter(this);
  painter.drawImage(0, 0, image);
  drawRadiusBox(&painter);
}

void GlowEffectWidget::wheelEvent(QWheelEvent * e)
{ setGlowRadius((e->angleDelta().y() > 0) ? m_radius + 1 : m_radius - 1); }

void GlowEffectWidget::drawRadiusBox(QPainter * p)
{
  p->save();
  p->setWindow(0, 0, 800, 600);
  p->setRenderHint(QPainter::Antialiasing, true);
  p->setPen(QPen(Qt::black));
  p->setBrush(QBrush(QColor(193, 193, 193, 127)));
  p->drawRoundedRect(10, 540, 160, 50, 10, 20);

  QFont font("Arial", 16);
  font.setUnderline(true);
  p->setFont(font);
  p->drawText(20, 560, "Radius");

  font.setUnderline(false);
  font.setPointSize(14);
  p->setFont(font);
  p->drawText(30, 580, QString("Radius is: %1").arg(m_radius));

  p->restore();
}

// Exponential blur, Jani Huhtanen, 2006
#include <cmath>

template<int aprec, int zprec>
static inline void blurinner(unsigned char * bptr, int & zR, int & zG, int & zB, int & zA, int alpha)
{
  int R, G, B, A;
  R = *bptr;
  G = *(bptr + 1);
  B = *(bptr + 2);
  A = *(bptr + 3);
  zR += (alpha * ((R << zprec) - zR)) >> aprec;
  zG += (alpha * ((G << zprec) - zG)) >> aprec;
  zB += (alpha * ((B << zprec) - zB)) >> aprec;
  zA += (alpha * ((A << zprec) - zA)) >> aprec;
  *bptr = zR >> zprec;
  *(bptr + 1) = zG >> zprec;
  *(bptr + 2) = zB >> zprec;
  *(bptr + 3) = zA >> zprec;
}

template<int aprec, int zprec>
static inline void blurrow(QImage & im, int line, int alpha)
{
  int zR, zG, zB, zA;
  QRgb * ptr = (QRgb *)im.scanLine(line);
  zR = *((unsigned char *)ptr) << zprec;
  zG = *((unsigned char *)ptr + 1) << zprec;
  zB = *((unsigned char *)ptr + 2) << zprec;
  zA = *((unsigned char *)ptr + 3) << zprec;
  for(int index = 1; index < im.width(); index++)
    blurinner<aprec, zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
  for(int index = im.width() - 2; index >= 0; index--)
    blurinner<aprec, zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
}

template<int aprec, int zprec>
static inline void blurcol(QImage & im, int col, int alpha)
{
  int zR, zG, zB, zA;
  QRgb * ptr = (QRgb *)im.bits();
  ptr += col;
  zR = *((unsigned char *)ptr) << zprec;
  zG = *((unsigned char *)ptr + 1) << zprec;
  zB = *((unsigned char *)ptr + 2) << zprec;
  zA = *((unsigned char *)ptr + 3) << zprec;
  for(int index = im.width(); index < (im.height() - 1) * im.width(); index += im.width())
    blurinner<aprec, zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
  for(int index = (im.height() - 2) * im.width(); index >= 0; index -= im.width())
    blurinner<aprec, zprec>((unsigned char *)&ptr[index], zR, zG, zB, zA, alpha);
}

/*
 *  expblur(QImage &img, int radius)
 *
 *  In-place blur of image 'img' with kernel
 *  of approximate radius 'radius'.
 *
 *  Blurs with two sided exponential impulse
 *  response.
 *
 *  aprec = precision of alpha parameter
 *  in fixed-point format 0.aprec
 *
 *  zprec = precision of state parameters
 *  zR,zG,zB and zA in fp format 8.zprec
 */
template<int aprec, int zprec>
void expblur(QImage & img, int radius)
{
  if(radius < 1) return;

  // Calculate the alpha such that 90% of the kernel is within the radius. Kernel extends to infinity.
  int alpha = (int)((1 << aprec) * (1.0f - expf(-2.3f / (radius + 1.f))));

  for(int row = 0; row < img.height(); row++) blurrow<aprec, zprec>(img, row, alpha);
  for(int col = 0; col < img.width(); col++) blurcol<aprec, zprec>(img, col, alpha);
}
