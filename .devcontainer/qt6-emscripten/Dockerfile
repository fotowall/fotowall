# Point at any base image that you find suitable to extend.
FROM stateoftheartio/qt6:6.6-wasm-aqt

RUN sudo apt update
RUN sudo apt install -y ca-certificates gpg wget
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | sudo tee /etc/apt/sources.list.d/kitware.list
RUN sudo apt update
RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || sudo rm /usr/share/keyrings/kitware-archive-keyring.gpg
RUN sudo apt install -y kitware-archive-keyring
RUN sudo apt install -y cmake

RUN sudo apt install -y libglx-dev libgl1-mesa-dev # Required for Qt 6 OpenGL support

RUN sudo chmod +x /opt/Qt/6.6.1/wasm_singlethread/bin/qt-cmake
ENV PATH="/opt/Qt/6.6.1/wasm_singlethread/bin:$PATH"

ENTRYPOINT ["/bin/bash", "-c"]
