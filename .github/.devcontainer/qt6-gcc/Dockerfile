# Point at any base image that you find suitable to extend.
FROM stateoftheartio/qt6:6.6-gcc-aqt

RUN sudo apt update
RUN sudo apt install -y libglx-dev libgl1-mesa-dev # Required for Qt 6 OpenGL support


ENTRYPOINT ["/bin/bash", "-c"]
