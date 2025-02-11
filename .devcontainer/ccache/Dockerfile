FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04
ARG EMAIL="devcontainer@mc-rtc-superbuild.com"
ARG NAME="mc_rtc devcontainer"
ARG UBUNTU_VERSION="jammy"

RUN export DEBIAN_FRONTEND=noninteractive

# Install latest cmake version
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
 && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null \
 && apt update \
 && rm /usr/share/keyrings/kitware-archive-keyring.gpg \
 && apt-get install -y --no-install-recommends kitware-archive-keyring \
 && apt-get update \
 && apt-get install -y --no-install-recommends cmake

RUN apt-get -y install --no-install-recommends ccache python3-venv python3-pip ripgrep cmake qtbase5-dev libqt5svg5-dev libqt5webkit5-dev libqt5opengl5-dev


# Git now forces you to have an email
# TODO: remove this (leaving it now to preserve cache)
RUN git config --global user.email "${EMAIL}" && git config --global user.name "${NAME}"

# Add mc-rtc-superbuild to the build context
USER vscode
RUN echo trest
RUN echo ttt
RUN git config --global user.email "${EMAIL}" && git config --global user.name "${NAME}"
COPY --chown=vscode:vscode ../.. /home/vscode/build_context/fotowall
WORKDIR /home/vscode/build_context/fotowall
RUN ls -al
RUN cmake --preset release
# Build everything to populate ccache
RUN cmake --build --preset release
RUN ccache -z
RUN rm -rf build
RUN cmake --preset release
# Build everything to populate ccache
RUN cmake --build --preset release
RUN ccache -s

ENTRYPOINT ["/bin/bash", "-c"]
