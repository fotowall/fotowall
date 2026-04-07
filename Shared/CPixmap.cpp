/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CPixmap.h"
#include "Shared/GlowEffectWidget.h"
#include <QImage>
#include <cmath>

CPixmap::CPixmap() {}

CPixmap::CPixmap(const QString & filePath) : QPixmap(filePath), m_filePath(filePath) {}

CPixmap::CPixmap(const QImage & image) : QPixmap(QPixmap::fromImage(image)), m_image(image) {}

CPixmap::CPixmap(const QPixmap & pixmap) : QPixmap(pixmap) {}

void CPixmap::addEffect(const PictureEffect & effect)
{
  switch(effect.effect)
  {
    case PictureEffect::ClearEffects:
      clearEffects();
      break;
    case PictureEffect::FlipH:
      toHFlip();
      break;
    case PictureEffect::FlipV:
      toVFlip();
      break;
    case PictureEffect::InvertColors:
      toInvertedColors();
      break;
    case PictureEffect::NVG:
      toNVG();
      break;
    case PictureEffect::BlackAndWhite:
      toBlackAndWhite();
      break;
    case PictureEffect::Glow:
      toGlow((int)effect.param);
      break;
    case PictureEffect::Sepia:
      toSepia();
      break;
    case PictureEffect::Opacity:
      m_effects.push_back(PictureEffect(PictureEffect::Opacity, (qreal)effect.param));
      break;
    case PictureEffect::Crop:
      toCropped(effect.rect);
      break;
    case PictureEffect::AutoBlend:
      toAutoBlend(effect.param);
      break;
    case PictureEffect::Rotate:
      rotate();
      break;
  }
}

QList<PictureEffect> CPixmap::effects() const
{ return m_effects; }

void CPixmap::clearEffects()
{
  if(!m_image.isNull())
    updateImage(m_image);
  else if(!m_filePath.isEmpty())
    load(m_filePath);
  m_effects.clear();
}

void CPixmap::toNVG()
{
  m_effects.push_back(PictureEffect::NVG);
  QImage img = this->toImage();
  QImage dest(img.size(), QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      const QRgb pixel = img.pixel(x, y);
      unsigned int average = (qGreen(pixel) + qRed(pixel) + qBlue(pixel)) / 3;
      dest.setPixel(x, y, qRgba(average, average, average, qAlpha(pixel)));
    }
  }
  updateImage(dest);
}

void CPixmap::toInvertedColors()
{
  // can't use invertPixels, because it gives artifacts
  m_effects.push_back(PictureEffect::InvertColors);
  const QImage img = this->toImage();
  QImage inverted(img.size(), QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      QRgb pixel = img.pixel(x, y);
      pixel = qRgba(255 - qRed(pixel), 255 - qGreen(pixel), 255 - qBlue(pixel), qAlpha(pixel));
      inverted.setPixel(x, y, pixel);
    }
  }
  updateImage(inverted);
}

void CPixmap::toHFlip()
{
  m_effects.push_back(PictureEffect::FlipH);
  QImage img = this->toImage().mirrored(true, false);
  updateImage(img);
}

void CPixmap::toVFlip()
{
  m_effects.push_back(PictureEffect::FlipV);
  QImage img = this->toImage().mirrored(false, true);
  updateImage(img);
}

void CPixmap::toBlackAndWhite()
{
  m_effects.push_back(PictureEffect::BlackAndWhite);
  const QImage img = this->toImage();
  QImage dest(img.size(), QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      const QRgb pixel = img.pixel(x, y);
      unsigned int average = (qGreen(pixel) + qRed(pixel) + qBlue(pixel)) / 3;
      if(average > 127)
        average = 255;
      else
        average = 0;
      dest.setPixel(x, y, qRgba(average, average, average, qAlpha(pixel)));
    }
  }
  updateImage(dest);
}

void CPixmap::toGlow(int radius)
{
  m_effects.push_back(PictureEffect(PictureEffect::Glow, (qreal)radius));
  QImage dest = GlowEffectWidget::glown(this->toImage(), radius);
  updateImage(dest);
}

void CPixmap::toSepia()
{
  m_effects.push_back(PictureEffect::Sepia);
  const QImage img = this->toImage();
  QImage dest(img.size(), QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      QRgb pixel = img.pixel(x, y);
      const unsigned int average = (qGreen(pixel) + qRed(pixel) + qBlue(pixel)) / 3;
      const int red = average * 1.176, green = average * 0.837, blue = average * 0.558;
      pixel = qRgba((red <= 255) ? red : 255, (green <= 255) ? green : 255, (blue <= 255) ? blue : 255, qAlpha(pixel));
      dest.setPixel(x, y, pixel);
    }
  }
  updateImage(dest);
}
/*
void CPixmap::toLuminosity(int value) {
    m_effects.push_back(...);
    QImage img = this->toImage();
    QImage dest(img.size(), img.format());
    QColor pixel;
    for(int x=0; x<img.width();x++) {
        for (int y=0; y<img.height(); y++) {
            pixel = img.pixel(x, y);
            int green = pixel.green() + value;
            int red = pixel.red() + value;
            int blue = pixel.blue() + value;
            if(green > 255) green = 255;
            else if(green < 0) green = 0;
            if(red > 255) red = 255;
            else if(red < 0) red = 0;
            if(blue > 255) blue = 255;
            else if(blue <0) blue = 0;

            pixel.setGreen(green);
            pixel.setBlue(blue);
            pixel.setRed(red);
            dest.setPixel(x,y,pixel.rgb());
        }
    }
    updateImage(dest);
}
*/
void CPixmap::toCropped(const QRect & cropRect)
{
  if(cropRect.isNull()) return;
  m_effects.push_back(PictureEffect(PictureEffect::Crop, PictureEffect::Crop, cropRect));
  const QImage img = this->toImage().copy(cropRect);
  updateImage(img);
}

void CPixmap::toAutoBlend(qreal strength)
{
  m_effects.push_back(PictureEffect(PictureEffect::AutoBlend, strength));
  const QImage img = this->toImage();
  QImage dest(img.size(), QImage::Format_ARGB32);
  const int w = img.width();
  const int h = img.height();
  // create a fast lookup-table for the 256 possible alpha levels {a = i^5^(2*strength - 1)}
  int gammaValue[256];
  qreal expIdx = pow((qreal)5, (strength * 2) - 1);
  for(int i = 0; i < 256; i++)
  {
    qreal unif = (qreal)i / 255.f;
    gammaValue[i] = (int)(255.f * pow(unif, expIdx));
  }
  for(int y = 0; y < h; y++)
  {
    for(int x = 0; x < w; x++)
    {
      QRgb pixel = img.pixel(x, y);
      const int gray = qGray(pixel);
      const int alpha = gammaValue[gray];
      pixel = qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), alpha);
      dest.setPixel(x, y, pixel);
    }
  }
  updateImage(dest);
}

void CPixmap::rotate()
{
  m_effects.push_back(PictureEffect::Rotate);
  const QImage srcImg = this->toImage();
  QPoint center = srcImg.rect().center();
  QTransform matrix;
  matrix.translate(center.x(), center.y());
  matrix.rotate(-90);
  QImage dstImg = srcImg.transformed(matrix);
  updateImage(dstImg);
}

void CPixmap::updateImage(const QImage & newImage)
{
  QImage copyImage = m_image;
  QString copyFilePath = m_filePath;
  QList<PictureEffect> copyEffects = m_effects;
  *this = fromImage(newImage);
  m_image = copyImage;
  m_filePath = copyFilePath;
  m_effects = copyEffects;
}
