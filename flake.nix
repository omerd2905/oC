{
  description = "oC: a small self-hosted systems programming language";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      # `nix profile install .` (or `nix run . -- run examples/hello.oc`)
      packages = forAllSystems (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "occ";
          version = "1.0.2";
          src = self;
          nativeBuildInputs = [ pkgs.makeWrapper ];
          dontConfigure = true;
          buildPhase = ''
            $CC -O2 bootstrap/occ_bootstrap.c -o occ
          '';
          # occ finds std/ next to its bin/ directory ($out/bin/occ -> $out/std).
          # The C backend shells out to `cc`, so the wrapper puts one on PATH.
          installPhase = ''
            mkdir -p $out/bin
            cp occ $out/bin/occ
            cp -r std $out/std
            wrapProgram $out/bin/occ --prefix PATH : ${pkgs.lib.makeBinPath [ pkgs.stdenv.cc ]}
          '';
        };
      });
    };
}
