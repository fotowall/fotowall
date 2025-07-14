/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "VideoProvider.h"
#include <QDirIterator>
#include <QPixmap>
#include <QTimer>
#if defined(HAS_VIDEOCAPTURE)
#  include "3rdparty/videocapture/VideoDevice.h"
#endif

// the global video provider instance
Q_GLOBAL_STATIC(VideoProvider, s_providerInstance)
VideoProvider * VideoProvider::instance()
{
  return s_providerInstance();
}

VideoProvider::VideoProvider() : QObject(), m_snapTimer(0)
{
  // defer video initialization, to offload gui on startup...
  if(!Disable) QTimer::singleShot(400, this, SLOT(initDevices()));
}

VideoProvider::~VideoProvider()
{
  delete m_snapTimer;
  qDeleteAll(m_inputs);
}

int VideoProvider::inputCount() const
{
  return m_inputs.size();
}

bool VideoProvider::connectInput(int iIdx, QObject * receiver, const char * method)
{
  // safety check
  if(iIdx < 0 || iIdx >= m_inputs.size() || !receiver || !method)
  {
    qWarning("VideoProvider::connectInput: fail");
    return false;
  }

  // start the video if we're the first
  VideoInput * input = m_inputs[iIdx];
  if(!input->active)
  {
#if defined(HAS_VIDEOCAPTURE)
    // try to start the video
    if(!input->device->setCaptureSize(input->device->maxSize()))
      qWarning("VideoProvider::connectInput: can't set the capture size. trying anyways..");
    if(!input->device->startCapturing())
    {
      qWarning("VideoProvider::connectInput: can't start capture, stopping");
      return false;
    }
#endif

    // mark as active
    input->active = true;
  }

  // connect input
  if(!connect(input, SIGNAL(newPixmap(QPixmap)), receiver, method))
  {
    qWarning("VideoProvider::connectInput: error connecting input %d to %s", iIdx, method);
    return false;
  }

  // add it to the receivers list (for ref. counting only)
  input->receivers.append(receiver);

  // start the capture timer if needed
  if(!m_snapTimer)
  {
    m_snapTimer = new QTimer(this);
    connect(m_snapTimer, SIGNAL(timeout()), this, SLOT(slotCaptureFromDevices()));
    m_snapTimer->start(50);
  }

  // tell that everything went good
  return true;
}

void VideoProvider::disconnectReceiver(QObject * receiver)
{
  if(!receiver) return;

  // remove the receiver from each input
  int activeInputs = 0;
  foreach(VideoInput * input, m_inputs)
  {

    // perform the disconnection
    disconnect(input, 0, receiver, 0);

    // remove the receiver, if any
    input->receivers.removeAll(receiver);

    // stop video if cleared all receivers
    if(input->receivers.isEmpty() && input->active)
    {

      // stop capturing
#if defined(HAS_VIDEOCAPTURE)
      input->device->stopCapturing();
#else
      // TODO
#endif

      // mark as inactive
      input->active = false;
    }

    // if active, increment active input count
    if(input->active) activeInputs++;
  }

  // stop the timer too if not needed
  if(!activeInputs && m_snapTimer)
  {
    delete m_snapTimer;
    m_snapTimer = 0;
  }
}

void VideoProvider::setSwapped(int iIdx, bool swapped)
{
  if(iIdx >= 0 && iIdx < m_inputs.size()) m_inputs[iIdx]->swapped = swapped;
}

bool VideoProvider::swapped(int iIdx) const
{
  if(iIdx >= 0 && iIdx < m_inputs.size()) return m_inputs[iIdx]->swapped;
  return false;
}

void VideoProvider::initDevices()
{
#if defined(HAS_VIDEOCAPTURE)
  foreach(const VideoCapture::DeviceInfo & info, VideoCapture::VideoDevice::scanDevices())
  {
    // create a new capture device and initialize it
    VideoCapture::VideoDevice * capture = new VideoCapture::VideoDevice(info);
    if(!capture->open() || capture->inputCount() < 1)
    {
      delete capture;
      return;
    }
    // capture->close();         // leave capture open...

    // setup input
    capture->setCurrentInput(0);

    // add the internal reference
    VideoInput * input = new VideoInput();
    input->channels = capture->inputCount();
    input->device = capture;
    m_inputs.append(input);

    // update status
    emit inputCountChanged(m_inputs.size());
  }
#endif
}

void VideoProvider::slotCaptureFromDevices()
{
  // capture from every input to every receiver
  foreach(VideoInput * input, m_inputs)
  {

    // skip inactive inputs
    if(!input->active) continue;

#if defined(HAS_VIDEOCAPTURE)
    // get frame (and check correctness)
    if(!input->device->captureFrame()) continue;

    // get the qimage from the frame
    QImage frameImage;
    bool captured = input->device->getLastFrame(&frameImage);
    if(!captured)
    {
      // display the invalid (blue) image anyways
      // continue;
    }

    // apply mirror, if requested
    if(captured && input->swapped) frameImage = frameImage.mirrored(true, false);

    // set the pixmap
    emit input->newPixmap(QPixmap::fromImage(frameImage));
#else
    // TODO
#endif
  }
}

VideoInput::VideoInput() : channels(1), active(false), swapped(false) {}

VideoInput::~VideoInput()
{
#if defined(HAS_VIDEOCAPTURE)
  if(active) device->stopCapturing();
  device->close();
  delete device;
#endif
}
