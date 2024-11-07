Fotowall
========

Fotowall is a desktop APP that lets you create graphical compositions by layering
and manipulating photos and pictures, text, live video, wordclouds, and drag&drop
content from the internet. All to create a fun graphical composition with the
maximum ease of use!

Check out the [home page](https://www.enricoros.com/opensource/fotowall), the
[news](https://www.enricoros.com/opensource/fotowall/news), and the
[old blog](https://www.enricoros.com/oldblog/tag/fotowall/) too.
*Focus is on simplicity, express your creativity! ;-)*

## Installation

We provide several installation options: AppImage or Ubuntu packages on Linux systems, Executables on Windows or build from source on other OS.

### Packages (Ubuntu)

We provide two repositories to install Fotowall from on Ubuntu: the `stable` repository contains packages for the latest release, the `head` repository contains the latest developpement version (github's main branch).

#### stable (recommended)

To install the `stable` version, first set-up our repository:

```sh
curl -1sLf \
  'https://dl.cloudsmith.io/public/arntanguy/stable/setup.deb.sh' \
  | sudo -E bash
```

Then install Fotowall

```sh
sudo apt install fotowall
```

#### head (for testing purposes)

To install the `head` version, first set-up our repository:

```sh
curl -1sLf \
  'https://dl.cloudsmith.io/public/arntanguy/head/setup.deb.sh' \
  | sudo -E bash
```

Then install Fotowall

```sh
sudo apt install fotowall
```

### AppImage

Or you can install it from our `AppImage` releases.

#### head (for testing purposes)

| Version | AppImage Download Link |
| ------- | ---------------------- |
| Ubuntu 20.04 | `curl -1sLf -O 'https://dl.cloudsmith.io/public/arntanguy/head/raw/versions/latest/Fotowall-latest-x86_64-ubuntu-20.04.AppImage'` |
| Ubuntu 22.04 | `curl -1sLf -O 'https://dl.cloudsmith.io/public/arntanguy/head/raw/versions/latest/Fotowall-latest-x86_64-ubuntu-22.04.AppImage'` |
| Ubuntu 24.04 | `curl -1sLf -O 'https://dl.cloudsmith.io/public/arntanguy/head/raw/versions/latest/Fotowall-latest-x86_64-ubuntu-24.04.AppImage'` |

### Download

On other systems, you can download the latest release from the [Releases](https://github.com/enricoros/fotowall/releases/latest) page.

## ChangeLog
### Fotowall "head" (upcoming):
* Undo/Redo system
* Ubuntu packaging

### Fotowall "RETRO" 1.0 (2017-07-09):
* Picture cropping (reqby Rossana)
* Exporters: vast improvements, high-quality print-ready PDF output
* Wordcloud editor
* Text: Color Triangle picker, and Shaking
* Re-Arrange Contents
* View perspective and rotation (reqby Nicolas Brisset)
* Port to QT5
* Arnaud's Undo/Redo system (not active yet)
* Symbian port (not that anyone uses it anymore :D)

### Fotowall "Alchimia" 0.9 (2009-12-08):
* Fast, Solid and Integrated Workflow with Home Screen
* New Contents: Canvas and Wordcloud
* Graphics Effects
* Nested Editors
* LikeBack feedback system
* Zoomable canvas
* Other changes (more than 250 ;-)
  * Autoblend effect for Images
  * SVG export
  * Quick and improved property editors
  * Cursor key movement
  * OpenGL performance tests and auto-tuning
  * Google Images search (by Marco Bavagnoli, reqby. Rossana)
  * Cleanups and Refactors: Selection, Frames, Rendering, Backgrounding, DVD/CD, Print/Export
  * Fixed relative saving (with contents auto-search)
  * Fixed multiple Webcams and bad Colors
  * Fixed the Exact Size modes
  * Fixed licensing

### Version 0.8 (2009-09-12):
* Bezier Shaped Text
* Pictures: Crop and change Opacity
* Context-sensitive Top-Bar
* Window Transparency
* OpenGL Acceleration
* Added many User Requested Features
  * add ok button for finalizing the text and just close the tab with X (req. by Rossana)
  * network image loading from http (req. by Rossana)
  * pictures on background: keep ratio in some way (req. by Rossana)
  * same folder when you open the picture (req. by Rossana)
  * Video -> Webcam (req. by Rossana)
  * FotoWall -> Fotowall (req. by Rossana)
* Merged external Contributions
 * fixes from Georges Dubus (intro overflow and translation) and Andreas Brech (bad accelereators, scale with all corners)
* Other changes
  * restore the common behavior for rotating (left click)
  * animations with 4.6: top bar, item show/hide, properties show/hide
  * builds with 4.4 (without many features), 4.5 (without animations), 4.6
  * fix: left alignment of some items
  * fix: resizing works only with the bottom right corner. fix the others.

### Version 0.7.1 (2009-07-23):
* Flickr content download
* Qt 4.6 GFX cuteness and network speed (4.6 highly recommended)
* Rubberband selection of items
* Prettified properties dialogs
* Better load/save (even of network objects)
* Load additional frames + 1 new frame (by Arnaud Tanguy)
* German translation (by Martin Zimmermann)
* Check for updates
* Fixed many bugs

### Version 0.6 (2009-04-29):
* PosteRazor Integration
* Live Video (Linux only, requires a WebCam)
* Export Wizard (Wallpaper, Image, PosteRazor, Print)
* XML file format (will ensure compatibility from now on)
* Multi-items selection (to move, delete, set properties, ...)
* All-in-one Scale+Rotate button (different with LMB/RMB and Shift/Ctrl)
* Perspective Transform
* Fixed Size mode
* Sepia Effect
* Scale Text like Images

### Version 0.5 (2009-04-11):
* CD/DVD Cover composition and printing (by Arnaud Tanguy)
* Rich Text (with extended editing)
* Glow effect (by Arnaud Tanguy)
* Properties configurable for all objects [reqby Ilan d'Inca]
* Usability fixes [reqby Ilan d'Inca]
* Save to all the supported image formats (by Arnaud Tanguy)
* Properties close button [reqby Ilan d'Inca]
* Improved stacking (fixes + relative to neighbourhoods) [reqby Ilan d'Inca]
* Add the 'No effect' effect removal option (by Arnaud Tanguy)
* French Introduction translation (by Arnaud Tanguy)
* Translation of the Introduction text (by Arnaud Tanguy)
* Fall back to QTextDocument where QtWebkit is not available

### Version 0.4 (2009-03-29):
* Windows executable on [github](http://github.com/enricoros/Fotowall/downloads)
* Image Effects (by Arnaud Tanguy)
* French Translation (by Arnaud Tanguy)
* German translation [Martin]
* Apply properties to All

### Version 0.3.1 (2009-03-13):
* builds with Qt 4.4
* Brazilian translation [Marcio Moraes]
* Configuration Panel: right click on an image and edit properties
* More Frame types (default, heart, Plasma backgrounds) [David Eccher]
* More Image Formats (bmp, tiff, gif, ...) [Superstoned]
* Reflections, and really cool ones
* Mirror the photo by clicking on an axis [Alberto Scarpa]
* DoubleClick on a picture -> set it as background, and vice-versa [Alberto Scarpa]
* More pretty (plasma icons) and usable (behavior) frame buttons
* Stacking order [Superstoned]
* Export a single picture as-you-see-it on disk
* Fixed-ratio resize: hold CTRL while resizing
* Visual Help [Alberto Scarpa]
* Drag&Drop: better behavior and can change a picture inside an existing frame too
* Pictures stay on-screen


Building
--------
This Fotowall package provides source code; to make an executable out of the
sources you have to know a little about programming or ask one of your friends
to do it ;-) Or just grab an 'executable package' from the project homepage.

## Ubuntu
Download Ubuntu 16.04 on a VM, then:
```
sudo apt-get update
sudo apt-get install git qtbase5-dev libqt5svg5-dev qt5-default qtcreator pax-utils tree -y
git clone https://github.com/enricoros/fotowall.git
cd fotowall
qmake
make -j6
./fotowall # enjoy
```
Optionally you can use 'sudo make install' to perform an installation in your system,
including showing the icon in the system menu; however you just need the 'fotowall'
executable.

## Ubuntu Static
Start by creating static qt5 development libraries. You can even use an older Ubuntu 14.04 for it, for greater compatibility.
```
wget https://download.qt.io/archive/qt/5.9/5.9.1/single/qt-everywhere-opensource-src-5.9.1.tar.xz
tar xpf qt-everywhere-opensource-src-5.9.1.tar.xz
cd qt-everywhere-opensource-src-5.9.1
./configure -static -release -prefix /opt/qt-5.9.1-static -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -ssl -opensource -confirm-license -make libs -nomake tools -nomake examples -nomake tests -qpa xcb -qt-xcb -opengl desktop -fontconfig
time make -j6
sudo make install
```
Then, to make the static executable follow the procedure above, replacing 'qmake'
with '/opt/qt-5.9.1-static/bin/qmake'.

## Windows Static
Start by creating static qt5 development libraries. Unpack the sources in c:\qt\src,
or you'll risk too long FS paths. First add the following 2 lines to C:\qt\src\qtbase\mkspecs\win32-g++\qmake.conf:
```
QMAKE_LFLAGS += -static -static-libgcc
DEFINES += QT_STATIC_BUILD
```
Then make sure you have mingw32 (likely installed via the Qt5 dynamic libs installer), and make
sure the SSL includes and libraries (libcrypto, libssl) are in the default paths in the mingw distro.
You may have to copy includes and libs all over the place.

Then execute the following commands in Powershell (assuming you already downloaded and patched):
```
Set-StrictMode -Version 3
$QtDir = "C:\qt\static-5.9.1"
$MingwDir = "C:\p\apps\Qt\Tools\mingw530_32"
$OpenSSLDir = "C:\OpenSSL-Win32"
$env:Path = "$OpenSSLDir\bin;$MingwDir\bin;$MingwDir\opt\bin;$env:SystemRoot\system32;$env:SystemRoot"
$env:LANG = "en"
$env:QT_INSTALL_PREFIX = $QtDir
./configure.bat -static -release -platform win32-g++ -prefix $QtDir -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg -qt-freetype -ssl -opengl desktop -opensource -confirm-license -make libs -nomake tools -nomake examples -nomake tests
mingw32-make -k -j6
mingw32-make -k install
# note: the following don't work yet:
# ./configure ... -openssl-linked -I $OpenSSLDir\include -L $OpenSSLDir\lib\MinGW OPENSSL_LIBS="-leay32 -l:ssleay32.a"
```
In case of problems, take a look at http://doc.qt.io/qt-5/windows-requirements.html,
and http://doc.qt.io/qt-5/ssl.html (for -openssl-linked ).
You can now clone Fotowall, open Creator and point it to the new libraries, and make the static executable.


Sharing Ideas
-------------

I hope to expand the program as soon as I can, since there is many room for
improvement and the code is kept clean and small.

- You can join the development.. the whole stuff is done with Qt 4.5 or higher
  using the GraphicsView framework.
- And if you have any Idea or Suggestion, just drop me a line, on the project
  homepage or by mail at  enrico.ros@gmail.com ;-)

### Current Wishlist and user's requests

* Add the possibility to create a template ("like specify the elements you can readily replace (like photos and design elements)
 and specify those that are permanent.. this would be very useful for album layouting.") - assigned to Arnaud TANGUY
* Make the propeties dialog moveable/stay in the current view (reqby Martin Zimmermann, and many French users)
* Integrate Marco Bavagnoli updates on the googleimagedownloader
* Parametric "Random placement", so the image can not be rotated, for example
* Change all pictures opacity at the same time (workaround for the 'random placement' that changes opacity too)
* Font selectable - this is used for title and for frame's text
* Create a MacOS like screensaver - where polaroids fall on the background over time, one on top of the other (or mixed)
* Use the Exact size specifying the Pixel size too (carefully choose the other constraint: dpis or print size?)
* Progress dialog when loading images/{fotowall file made of many} and/or threaded loading
* Text shaking
* pop-art: pointel-ize, like krita brushes, using circles of different size
* add the undo/redo support
* canvas 'dummy' mode for taking snapshots only (shuts off video, for example)
* frame text: centered
* make uninstall
* transform pictures with a 'magneto tool' (good on grids)
* collate pictures, like autocollage, but keeping user transformations
  * blend images in one another (reqby Ruud)
* handle initial load-save after being shown. also evaluate normalized rescaling.
* frame: add bordercolor-propagating frame
* svg drawings [balloons, signs, etc..]
* masked pic
* both new and old resize&co
* webcam: on win32
* webcam: configuration panel
* templates
  * the template system could even be some auto-generated fotowall file with changing urls.
* provide some default/example files, also loadable with a short path
  * use a fotowall scene as gui (4 squared load/save/welcome/etc..)
  * scene-in-scene for 'recent files'
  * embedded examples (inside/network)
* duplicate/clone content
* real cropping
* scissors tool
  * real shaping, intersects frame shape
* integrate with a gimp-like library for effects/graphics ops
* new graphics effects (like Colorize or Warhol-ize ;-)
  * mosaic effect: from 1 picture to a grid of misaligned pics (needs re-cropping)
* update the video tutorial (still at 0.2!)
* add the "color changing stuff" to the picture themselves (req. by Contrast, Superstoned)
* win: photo dragging from the 'add picture' filedialog (seems harder that it should be. works on linux) (req. by Rossana)
* auto-placement [force field, shape, collage, etc..]
  * ap: perform rectangular image composition based on an input picture (color, pattern)
  * ap: perform image distribution based on shapes or probability sampling (on a sprayed grayscale canvas)
* 3d text
  * global illumitation/shading for shadows, frames, 3d text
* Google-suggest completion (already in, but misplaced when embedded into qgraphicsview)
* integrate kde-apps knowledgebase (already in, but doesn't work great)
* scene matrix (global perspective)
* regional links to web resources
 - italian: http://www.dylanblog.com/2009/05/27/creare-dei-wallpaper-personalizzati-con-fotowall-061/
 - french: http://forum.ubuntu-fr.org/viewtopic.php?id=308200
           http://geenux.wordpress.com/tag/fotowall/  (Arnaud Tanguy's blog)
           http://benoit.balon.free.fr/?tag=fotowall  (Benoît Bâlon's blog)
* missing relicences
 - 3rdparty/richtextedit/richtexteditor*: redo 'cause non free


Old News
--------

**2009-03-02** Fotowall is now on GitHub. Development will continue here. Changes are coming soon!

**2008-09-03** Peter Upfold put together a great ScreenCast of the application!!

See http://fosswire.com/2008/09/03/Fotowall-make-wallpaper-collages-from-your-photos/ fot the article and click on the Video link to see the screencast.

**2008-08-01** Thanks to everybody that is sending me patches!

I really appreciate that, even if I don't have time right now to check and apply them.. but I'm willing to give everybody the SVN access, so drop me a note if you want to contribute!

**2008-04-14** Updated this page including the Wishlist

**2008-04-06** Fotowall 0.21 released

Changes in this release are: relicensing and a new program icon

**2008-04-06** Imported the project into Google Code
