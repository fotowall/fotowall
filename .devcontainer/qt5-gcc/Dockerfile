FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

RUN sudo apt update
RUN sudo apt install -y --no-install-recommends qtbase5-dev libqt5svg5-dev libqt5webkit5-dev libqt5opengl5-dev
RUN sudo apt install -y --no-install-recommends ninja-build
RUN sudo apt install -y --no-install-recommends ripgrep fzf

ENTRYPOINT ["/bin/bash", "-c"]
