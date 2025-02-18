FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04
ARG EMAIL="devcontainer@mc-rtc-superbuild.com"
ARG NAME="mc_rtc devcontainer"
ARG UBUNTU_VERSION="jammy"

RUN export DEBIAN_FRONTEND=noninteractive

# Allow to keep apt cache between builds
RUN rm -f /etc/apt/apt.conf.d/docker-clean

# Install latest cmake version
RUN --mount=type=cache,target=/var/cache/apt \
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
 && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null \
 && apt update \
 && rm /usr/share/keyrings/kitware-archive-keyring.gpg \
 && apt-get install -y --no-install-recommends kitware-archive-keyring \
 && apt-get update \
 && apt-get install -y --no-install-recommends cmake

RUN --mount=type=cache,target=/var/cache/apt \
  apt-get -y install --no-install-recommends ccache python3-venv python3-pip ripgrep cmake qtbase5-dev libqt5svg5-dev libqt5webkit5-dev libqt5opengl5-dev


# Git now forces you to have an email
# TODO: remove this (leaving it now to preserve cache)
RUN git config --global user.email "${EMAIL}" && git config --global user.name "${NAME}"

USER vscode

# Mount cache ccache folder
RUN --mount=type=cache,uid=1000,gid=1000,target=/home/vscode/.cache/ccache \
  if [ "$(ls -A /home/vscode/.cache/ccache)" ]; then \
    echo "CCache Mount Directory is not empty, using it"; \
    ls -A /home/vscode/.cache/ccache; \
  else \
    echo "ccache mount directory is empty"; \
  fi

RUN git config --global user.email "${EMAIL}" && git config --global user.name "${NAME}"
COPY --chown=vscode:vscode ../../.. /home/vscode/workspace/fotowall
WORKDIR /home/vscode/workspace/fotowall
RUN ls -al
RUN --mount=type=cache,uid=1000,gid=1000,target=/home/vscode/.cache/ccache \
  ccache -z \
  && cmake --preset release \
  # Build everything to populate ccache
  && cmake --build --preset release \
  && ccache -s

RUN --mount=type=cache,uid=1000,gid=1000,target=/home/vscode/.cache/ccache \
  echo "Copying ccache /home/vscode/.cache/ccache to the image folder /tmp/.ccache" \
  && cp -r /home/vscode/.cache/ccache /tmp/.ccache

RUN echo "Storing ccache folder in the image" \
  && mv /tmp/.ccache /home/vscode/.cache

ENTRYPOINT ["/bin/bash", "-c"]
