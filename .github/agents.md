# Agents & CI Workflows

This document describes the automated CI/CD workflows and agent conventions for the Fotowall project.

## Project Overview

Fotowall is a Qt-based desktop application (C++17, CMake) that creates graphical photo compositions.
It supports Qt5 and Qt6, and builds on Linux, macOS, Windows, and WebAssembly.

## CI Workflows

### Linux build (`build.yml`)

Runs on every push/PR to `master` (excluding packaging and documentation changes).

| Dimension | Values |
|-----------|--------|
| OS | `ubuntu-22.04`, `ubuntu-24.04`, `ubuntu-26.04` |
| Build type | `Release`, `RelWithDebInfo` |
| Compiler | `gcc`, `clang` |
| Qt version | Qt5 (22.04, 24.04) · Qt6 (26.04) |

Uses the [`jrl-umi3218/github-actions`](https://github.com/jrl-umi3218/github-actions) reusable actions for dependency installation and CMake build/test.

### Nix build (`nix.yml`)

Runs on every push/PR to `master`/`main`/`devel`.

Validates the Nix flake on `ubuntu-latest` and `macos-latest`:
- `nix flake check -L` — verifies the flake
- `nix build -L` — builds the project

A summary job (`check-macos-linux-nix`) aggregates the matrix results.

### Windows release (`windows.yml`)

Runs on PRs targeting `master` and on version tags (`v*`).

Steps:
1. Install Qt 6.6.3 (`msvc2019_64`) via `jurplel/install-qt-action`.
2. Configure and build with CMake + MSVC in Release mode.
3. Collect `fotowall.exe` into a `dist/` directory.
4. Run `windeployqt` to bundle all required Qt DLLs alongside the executable.
5. **Smoke-test** the bundled executable (`QT_QPA_PLATFORM=offscreen`) to catch missing DLLs or startup crashes.
6. Upload the `dist/` directory as a GitHub Actions artifact (`fotowall-windows`).
7. On version tags: attach `dist/**` to the GitHub Release.

### Packaging (`package.yml`)

Runs on pushes to `master`, version tags, and `repository_dispatch` events (`package-master`, `package-release`).

| Job | Target distros | Qt | Destination |
|-----|----------------|----|-------------|
| `package-qt5` | `jammy` (22.04), `noble` (24.04) | Qt5 | Cloudsmith `arntanguy/head` or `arntanguy/stable` |
| `package-qt6` | `resolute` (26.04) | Qt6 | Cloudsmith `arntanguy/head` or `arntanguy/stable` |
| `package-appimage` | `ubuntu-22.04`, `ubuntu-24.04`, `ubuntu-26.04` | Qt5/Qt6 | Cloudsmith `arntanguy/head` (on `master`) |

AppImages are built with `AppImageCrafters/build-appimage-action` using the platform-specific `AppImageBuilder-qt5.yml` / `AppImageBuilder-qt6.yml` templates.

### Devcontainers & WebAssembly (`build-devcontainers.yml`)

Runs on every push/PR to `master` (excluding packaging and documentation changes).

**Devcontainer images** (published to `ghcr.io/fotowall/fotowall:<tag>` on `master`):

| Tag | Description |
|-----|-------------|
| `qt5-gcc` | Qt5 + GCC |
| `qt6-gcc` | Qt6 + GCC |
| `qt6-emscripten` | Qt6 + Emscripten (for WebAssembly) |

**WebAssembly build** (depends on `build_devcontainers`):
- Builds inside the `qt6-emscripten` devcontainer using `qt-cmake` + `ninja`.
- Bundles `fotowall.js`, `fotowall.html` (renamed `index.html`), `fotowall.wasm`, `qtloader.js`, `qtlogo.svg`.
- On `master` (main repository): pushes to Cloudsmith and deploys to GitHub Pages (`gh-pages` branch).

## Required Secrets

| Secret | Used by |
|--------|---------|
| `CLOUDSMITH_API_KEY` | `package.yml`, `build-devcontainers.yml` |
| `GH_PAGES_TOKEN` | `package.yml` (Cloudsmith push of packages) |
| `GITHUB_TOKEN` | `windows.yml` (GitHub Release), `build-devcontainers.yml` (GHCR push) |

## Coding Agent Guidelines

When working on this repository, keep the following in mind:

- **Build system**: CMake (minimum 3.22). The project auto-detects Qt6 first, then falls back to Qt5. Always validate Qt version-specific code paths for both.
- **C++ standard**: C++17.
- **Qt compatibility**: Prefer APIs available in both Qt5 and Qt6. When Qt6 removed a deprecated API (e.g. `QTime::start/elapsed` → `QElapsedTimer`), guard with `#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)` or use the Qt6-compatible replacement directly where both versions support it.
- **Windows**: The executable is named `fotowall.exe` (lowercase, matching the CMake target). The `Release/` subdirectory is the MSVC output location for `--config Release` builds.
- **Platform guards**: Video capture (`VideoDevice`, `VideoInput`, `bayer`, `sonix_compress`) and Unix timers (`sys/time.h`) are Linux-only. Non-Unix code paths use the `#else` branch of `#ifdef Q_OS_UNIX`.
- **CI triggers**: The Linux CI skips changes to `debian/**`, `README.md`, `doc/**`, `.pre-commit-config.yaml`, and packaging workflow files — avoid marking source changes as "documentation only".
- **Tests**: Use the `jrl-umi3218/github-actions/build-cmake-project` action convention; any CTest tests added to `CMakeLists.txt` will be executed automatically in the Linux CI matrix.
