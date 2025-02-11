cmake_minimum_required(VERSION 3.1)
project(fotowall)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(NOTICE "Selected CCache: ${CCACHE_PROGRAM}")
  # get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY) use
  # cmake_path instead
  message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
  cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH PARENT_DIR)
  message(STATUS "PARENT_DIR: ${PARENT_DIR}")
  set(ccacheEnv
      CCACHE_BASEDIR=${PARENT_DIR}
      # locale and umask are important for docker containers no_hash_dir useful?
      CCACHE_SLOPPINESS="umask,locale,no_hash_dir,pch_defines,file_macro,include_file_ctime"
      CCACHE_NOHASHDIR=true)
  set(CMAKE_C_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env ${ccacheEnv}
                                ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env ${ccacheEnv}
                                  ${CCACHE_PROGRAM})
endif()

find_package(
  Qt6 QUIET
  COMPONENTS Core
             Widgets
             Gui
             Network
             Svg
             Xml
             OpenGL
             OpenGLWidgets
             PrintSupport)
if(Qt6_FOUND)
  message(STATUS "Found Qt6")
  message(STATUS "Qt6 Version is: ${Qt6_VERSION}")
else()
  find_package(
    Qt5
    COMPONENTS
    REQUIRED
    Core
    Widgets
    Gui
    Network
    Svg
    Xml
    OpenGL
    PrintSupport)
  message(STATUS "Qt5 Version is: ${Qt5_VERSION}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/fotowall.qrc
    # ${CMAKE_SOURCE_DIR}/3rdparty/likebackfrontend/LikeBack.cpp
    # ${CMAKE_SOURCE_DIR}/3rdparty/likebackfrontend/LikeBack.h
    # ${CMAKE_SOURCE_DIR}/3rdparty/likebackfrontend/LikeBack_p.h
    # ${CMAKE_SOURCE_DIR}/3rdparty/likebackfrontend/LikeBackDialog.cpp
    # ${CMAKE_SOURCE_DIR}/3rdparty/likebackfrontend/LikeBackDialog.h
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/FloodGenerator.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/FloodGenerator.h
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/FloodPoly.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/FloodPoly.h
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/PencilItem.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/pencil/PencilItem.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/controller.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/controller.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/imageloaderinterface.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/imageloaderqt.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/imageloaderqt.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/paintcanvas.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/paintcanvas.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/paintcanvasinterface.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/pdfwriter.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/pdfwriter.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/posterazorcore.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/posterazorcore.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/snapspinbox.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/snapspinbox.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/types.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/types.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/wizard.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/wizard.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/wizardcontroller.h
    ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/wizardcontroller.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/qtcolortriangle/qtcolortriangle.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/qtcolortriangle/qtcolortriangle.h
    ${CMAKE_SOURCE_DIR}/3rdparty/richtextedit/richtexteditor.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/richtextedit/richtexteditor_p.h
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/bayer.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/bayer.h
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/sonix_compress.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/sonix_compress.h
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/VideoDevice.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/VideoDevice.h
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/VideoInput.cpp
    ${CMAKE_SOURCE_DIR}/3rdparty/videocapture/VideoInput.h
    ${CMAKE_SOURCE_DIR}/3rdparty/enricomath.h
    # ${CMAKE_SOURCE_DIR}/3rdparty/gsuggest.cpp
    # ${CMAKE_SOURCE_DIR}/3rdparty/gsuggest.h
    ${CMAKE_SOURCE_DIR}/App/App.cpp
    ${CMAKE_SOURCE_DIR}/App/App.h
    ${CMAKE_SOURCE_DIR}/App/CanvasAppliance.cpp
    ${CMAKE_SOURCE_DIR}/App/CanvasAppliance.h
    ${CMAKE_SOURCE_DIR}/App/ExactSizeDialog.cpp
    ${CMAKE_SOURCE_DIR}/App/ExactSizeDialog.h
    ${CMAKE_SOURCE_DIR}/App/ExportWizard.cpp
    ${CMAKE_SOURCE_DIR}/App/ExportWizard.h
    ${CMAKE_SOURCE_DIR}/App/FotowallFile.cpp
    ${CMAKE_SOURCE_DIR}/App/FotowallFile.h
    ${CMAKE_SOURCE_DIR}/App/Hardware3DTest.cpp
    ${CMAKE_SOURCE_DIR}/App/Hardware3DTest.h
    ${CMAKE_SOURCE_DIR}/App/HelpAppliance.cpp
    ${CMAKE_SOURCE_DIR}/App/HelpAppliance.h
    ${CMAKE_SOURCE_DIR}/App/HelpItem.cpp
    ${CMAKE_SOURCE_DIR}/App/HelpItem.h
    ${CMAKE_SOURCE_DIR}/App/HomeAppliance.cpp
    ${CMAKE_SOURCE_DIR}/App/HomeAppliance.h
    ${CMAKE_SOURCE_DIR}/App/HomeScene.cpp
    ${CMAKE_SOURCE_DIR}/App/HomeScene.h
    ${CMAKE_SOURCE_DIR}/App/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/App/MainWindow.h
    ${CMAKE_SOURCE_DIR}/App/OnlineServices.cpp
    ${CMAKE_SOURCE_DIR}/App/OnlineServices.h
    ${CMAKE_SOURCE_DIR}/App/PictureSearchWidget.cpp
    ${CMAKE_SOURCE_DIR}/App/PictureSearchWidget.h
    ${CMAKE_SOURCE_DIR}/App/SceneView.cpp
    ${CMAKE_SOURCE_DIR}/App/SceneView.h
    ${CMAKE_SOURCE_DIR}/App/Settings.cpp
    ${CMAKE_SOURCE_DIR}/App/Settings.h
    ${CMAKE_SOURCE_DIR}/App/UrlHistoryBox.cpp
    ${CMAKE_SOURCE_DIR}/App/UrlHistoryBox.h
    ${CMAKE_SOURCE_DIR}/App/VersionCheckDialog.cpp
    ${CMAKE_SOURCE_DIR}/App/VersionCheckDialog.h
    ${CMAKE_SOURCE_DIR}/App/WordcloudAppliance.cpp
    ${CMAKE_SOURCE_DIR}/App/WordcloudAppliance.h
    ${CMAKE_SOURCE_DIR}/App/WordcloudSidebar.cpp
    ${CMAKE_SOURCE_DIR}/App/WordcloudSidebar.h
    ${CMAKE_SOURCE_DIR}/App/Workflow.cpp
    ${CMAKE_SOURCE_DIR}/App/Workflow.h
    ${CMAKE_SOURCE_DIR}/Canvas/AbstractConfig.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/AbstractConfig.h
    ${CMAKE_SOURCE_DIR}/Canvas/AbstractContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/AbstractContent.h
    ${CMAKE_SOURCE_DIR}/Canvas/BezierCubicItem.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/BezierCubicItem.h
    ${CMAKE_SOURCE_DIR}/Canvas/ButtonItem.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/ButtonItem.h
    ${CMAKE_SOURCE_DIR}/Canvas/Canvas.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/Canvas.h
    ${CMAKE_SOURCE_DIR}/Canvas/CanvasModeInfo.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/CanvasModeInfo.h
    ${CMAKE_SOURCE_DIR}/Canvas/CanvasViewContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/CanvasViewContent.h
    ${CMAKE_SOURCE_DIR}/Canvas/ContentProperties.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/ContentProperties.h
    ${CMAKE_SOURCE_DIR}/Canvas/CornerItem.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/CornerItem.h
    ${CMAKE_SOURCE_DIR}/Canvas/MirrorItem.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/MirrorItem.h
    ${CMAKE_SOURCE_DIR}/Canvas/PictureConfig.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/PictureConfig.h
    ${CMAKE_SOURCE_DIR}/Canvas/PictureContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/PictureContent.h
    ${CMAKE_SOURCE_DIR}/Canvas/PictureProperties.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/PictureProperties.h
    ${CMAKE_SOURCE_DIR}/Canvas/SelectionProperties.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/SelectionProperties.h
    ${CMAKE_SOURCE_DIR}/Canvas/StyledButtonItem.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/StyledButtonItem.h
    ${CMAKE_SOURCE_DIR}/Canvas/TextConfig.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/TextConfig.h
    ${CMAKE_SOURCE_DIR}/Canvas/TextContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/TextContent.h
    ${CMAKE_SOURCE_DIR}/Canvas/TextProperties.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/TextProperties.h
    ${CMAKE_SOURCE_DIR}/Canvas/WebcamContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/WebcamContent.h
    ${CMAKE_SOURCE_DIR}/Canvas/WordcloudContent.cpp
    ${CMAKE_SOURCE_DIR}/Canvas/WordcloudContent.h
    ${CMAKE_SOURCE_DIR}/Frames/Frame.cpp
    ${CMAKE_SOURCE_DIR}/Frames/Frame.h
    ${CMAKE_SOURCE_DIR}/Frames/FrameFactory.cpp
    ${CMAKE_SOURCE_DIR}/Frames/FrameFactory.h
    ${CMAKE_SOURCE_DIR}/Frames/HeartFrame.cpp
    ${CMAKE_SOURCE_DIR}/Frames/HeartFrame.h
    ${CMAKE_SOURCE_DIR}/Frames/PlasmaFrame.cpp
    ${CMAKE_SOURCE_DIR}/Frames/PlasmaFrame.h
    ${CMAKE_SOURCE_DIR}/Frames/StandardFrame.cpp
    ${CMAKE_SOURCE_DIR}/Frames/StandardFrame.h
    ${CMAKE_SOURCE_DIR}/Shared/PictureServices/AbstractPictureService.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PictureServices/AbstractPictureService.h
    ${CMAKE_SOURCE_DIR}/Shared/PictureServices/FlickrPictureService.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PictureServices/FlickrPictureService.h
    # ${CMAKE_SOURCE_DIR}/Shared/PictureServices/GoogleImagesPictureService.cpp
    # ${CMAKE_SOURCE_DIR}/Shared/PictureServices/GoogleImagesPictureService.h
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/AbstractAppliance.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/AbstractAppliance.h
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/Container.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/Container.h
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/Stacker.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PlugGui/Stacker.h
    ${CMAKE_SOURCE_DIR}/Shared/AbstractDisposeable.cpp
    ${CMAKE_SOURCE_DIR}/Shared/AbstractDisposeable.h
    ${CMAKE_SOURCE_DIR}/Shared/AbstractResourceProvider.h
    ${CMAKE_SOURCE_DIR}/Shared/AbstractScene.cpp
    ${CMAKE_SOURCE_DIR}/Shared/AbstractScene.h
    ${CMAKE_SOURCE_DIR}/Shared/BlinkingToolButton.cpp
    ${CMAKE_SOURCE_DIR}/Shared/BlinkingToolButton.h
    ${CMAKE_SOURCE_DIR}/Shared/BreadCrumbBar.cpp
    ${CMAKE_SOURCE_DIR}/Shared/BreadCrumbBar.h
    # ${CMAKE_SOURCE_DIR}/Shared/BrowserItem.cpp
    # ${CMAKE_SOURCE_DIR}/Shared/BrowserItem.h
    ${CMAKE_SOURCE_DIR}/Shared/ButtonsDialog.cpp
    ${CMAKE_SOURCE_DIR}/Shared/ButtonsDialog.h
    ${CMAKE_SOURCE_DIR}/Shared/ColorPickerItem.cpp
    ${CMAKE_SOURCE_DIR}/Shared/ColorPickerItem.h
    ${CMAKE_SOURCE_DIR}/Shared/CPixmap.cpp
    ${CMAKE_SOURCE_DIR}/Shared/CPixmap.h
    ${CMAKE_SOURCE_DIR}/Shared/CroppingDialog.cpp
    ${CMAKE_SOURCE_DIR}/Shared/CroppingDialog.h
    ${CMAKE_SOURCE_DIR}/Shared/CroppingWidget.cpp
    ${CMAKE_SOURCE_DIR}/Shared/CroppingWidget.h
    ${CMAKE_SOURCE_DIR}/Shared/GlowEffectDialog.cpp
    ${CMAKE_SOURCE_DIR}/Shared/GlowEffectDialog.h
    ${CMAKE_SOURCE_DIR}/Shared/GlowEffectWidget.cpp
    ${CMAKE_SOURCE_DIR}/Shared/GlowEffectWidget.h
    ${CMAKE_SOURCE_DIR}/Shared/GroupBoxWidget.cpp
    ${CMAKE_SOURCE_DIR}/Shared/GroupBoxWidget.h
    ${CMAKE_SOURCE_DIR}/Shared/HighlightItem.cpp
    ${CMAKE_SOURCE_DIR}/Shared/HighlightItem.h
    ${CMAKE_SOURCE_DIR}/Shared/MetaXmlReader.cpp
    ${CMAKE_SOURCE_DIR}/Shared/MetaXmlReader.h
    ${CMAKE_SOURCE_DIR}/Shared/PanePropertyEditor.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PanePropertyEditor.h
    ${CMAKE_SOURCE_DIR}/Shared/PictureEffect.h
    ${CMAKE_SOURCE_DIR}/Shared/PixmapButton.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PixmapButton.h
    ${CMAKE_SOURCE_DIR}/Shared/PropertyEditors.cpp
    ${CMAKE_SOURCE_DIR}/Shared/PropertyEditors.h
    ${CMAKE_SOURCE_DIR}/Shared/RenderOpts.h
    ${CMAKE_SOURCE_DIR}/Shared/VideoProvider.cpp
    ${CMAKE_SOURCE_DIR}/Shared/VideoProvider.h
    ${CMAKE_SOURCE_DIR}/Wordcloud/Bits.h
    ${CMAKE_SOURCE_DIR}/Wordcloud/Cloud.cpp
    ${CMAKE_SOURCE_DIR}/Wordcloud/Cloud.h
    ${CMAKE_SOURCE_DIR}/Wordcloud/Scanner.cpp
    ${CMAKE_SOURCE_DIR}/Wordcloud/Scanner.h
    ${CMAKE_SOURCE_DIR}/Wordcloud/WordItem.cpp
    ${CMAKE_SOURCE_DIR}/Wordcloud/WordItem.h
    ${CMAKE_SOURCE_DIR}/main.cpp)

if(Qt6_FOUND)
  qt_add_executable(fotowall ${SOURCE_FILES})
else()
  add_executable(fotowall ${SOURCE_FILES})
endif()
add_compile_definitions(HAS_EXPORTDIALOG)
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x051200)
if(EMSCRIPTEN)
  # XXX needed?
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

  # Configure WebAssembly build settings For some reason, the build settings
  # need to be provided through the linker. Most flags are configured to match
  # Qt qmake See EMCC_COMMON_LFLAGS in
  # https://github.com/qt/qtbase/blob/dev/mkspecs/wasm-emscripten/qmake.conf

  # Activate Embind C/C++ bindings
  # https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
  target_link_libraries(fotowall PUBLIC "--bind")

  # Activate WebGL 2 (in addition to WebGL 1)
  # https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0
  target_link_libraries(fotowall PUBLIC "-s USE_WEBGL2=1")

  # Emulate missing OpenGL ES2/ES3 features
  # https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#opengl-es-2-0-3-0-emulation
  target_link_libraries(fotowall PUBLIC "-s FULL_ES2=1")
  # add_link_options("SHELL:-s FULL_ES3=1")

  # Enable demangling of C++ stack traces
  # https://emscripten.org/docs/porting/Debugging.html
  target_link_libraries(fotowall PUBLIC "-s DEMANGLE_SUPPORT=1")

  # Run static dtors at teardown
  # https://emscripten.org/docs/getting_started/FAQ.html#what-does-exiting-the-runtime-mean-why-don-t-atexit-s-run
  target_link_libraries(fotowall PUBLIC "-s EXIT_RUNTIME=0")

  # Allows amount of memory used to change
  # https://emscripten.org/docs/optimizing/Optimizing-Code.html#memory-growth
  target_link_libraries(fotowall PUBLIC "-s ALLOW_MEMORY_GROWTH=1")
  # target_link_libraries(fotowall PUBLIC "-s MAXIMUM_MEMORY=1GB") # required
  # when combining USE_PTHREADS with ALLOW_MEMORY_GROWTH

  # Enable C++ exception catching
  # https://emscripten.org/docs/optimizing/Optimizing-Code.html#c-exceptions
  target_compile_options(fotowall PUBLIC -fexceptions)
  target_link_libraries(fotowall PUBLIC "-s DISABLE_EXCEPTION_CATCHING=0")

  # Export UTF16ToString,stringToUTF16 Required by
  # https://codereview.qt-project.org/c/qt/qtbase/+/286997 (since Qt 5.14)
  target_link_libraries(
    fotowall
    PUBLIC "-s EXTRA_EXPORTED_RUNTIME_METHODS=[UTF16ToString,stringToUTF16]")

  # Enable Fetch API https://emscripten.org/docs/api_reference/fetch.html
  target_link_libraries(fotowall PUBLIC "-s FETCH=1")
endif()

if(Qt5_FOUND)
  target_link_libraries(
    fotowall
    PUBLIC Qt5::Gui
           Qt5::Widgets
           Qt5::Xml
           Qt5::Network
           Qt5::Svg
           Qt5::OpenGL
           Qt5::Core
           # Qt5::WebKit Qt5::WebKitWidgets
           Qt5::PrintSupport
           X11)
elseif(Qt6_FOUND)
  target_link_libraries(
    fotowall
    PUBLIC Qt6::Gui
           Qt6::Widgets
           Qt6::Xml
           Qt6::Network
           Qt6::Svg
           Qt6::OpenGL
           Qt6::OpenGLWidgets
           Qt6::Core
           # Qt6::WebKit Qt6::WebKitWidgets
           Qt6::PrintSupport
           X11)
endif()
target_include_directories(fotowall
                           PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/posterazor/)

install(
  TARGETS fotowall
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

if(NOT WIN32)
  install(
    FILES fotowall.png
    DESTINATION share/pixmaps
    RENAME fotowall.png)
  install(FILES fotowall.desktop DESTINATION share/applications)
endif()
