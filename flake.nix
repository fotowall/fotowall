{
  description = "Nix flake for Fotowall development and build";

  inputs = {
    nixpkgs.url = "github:Nixos/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "fotowall";
          version = "1.0.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            # ccache removed here
            qt6.wrapQtAppsHook
          ];

          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtsvg
            qt6.qt5compat
            xorg.libX11
          ];

          dontWrapQtApps = false; 
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ self.packages.${system}.default ];

          packages = with pkgs; [
            gdb
            clazy
            clang-tools
            cmake-language-server
            bashInteractive
          ];

          shellHook = ''
            export PS1="\n\[\033[1;32m\][nix-dev:\w]\$\[\033[0m\] "
            echo "--- Fotowall Development Shell ---"
            echo "Run 'cmake -B build -G Ninja' to start."
          '';
        };
      });
}
