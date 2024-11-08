# Point at any base image that you find suitable to extend.
FROM reivilo1234/qt-webassembly-cmake:qt5.15-em1.39.8

# Install required tools that are useful for your project i.e. ninja-build
RUN apt update && apt install -y ninja-build

ENTRYPOINT ["/bin/bash", "-c"]
