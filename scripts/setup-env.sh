#!/usr/bin/env bash
# CUtils 開発環境セットアップスクリプト (Debian/Ubuntu 系, WSL2 を想定)
#
# 使い方:
#   scripts/setup-env.sh              # パッケージ導入 + Debug 構成 (build/)
#   scripts/setup-env.sh --no-apt     # パッケージ導入をスキップ
#   scripts/setup-env.sh --all        # Debug / Coverage / Address の全構成を作成
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

USE_APT=1
ALL_CONFIGS=0
for arg in "$@"; do
  case "$arg" in
    --no-apt) USE_APT=0 ;;
    --all) ALL_CONFIGS=1 ;;
    -h | --help)
      sed -n '2,8p' "${BASH_SOURCE[0]}"
      exit 0
      ;;
    *)
      echo "不明なオプション: $arg" >&2
      exit 1
      ;;
  esac
done

PACKAGES=(
  build-essential cmake ninja-build git
  clangd clang-format clang-tidy
  gdb gcovr lcov doxygen graphviz
)

if [[ $USE_APT -eq 1 ]]; then
  if ! command -v apt-get > /dev/null; then
    echo "apt-get が見つかりません。--no-apt を指定し、手動で依存を導入してください。" >&2
    exit 1
  fi
  SUDO=""
  [[ $EUID -ne 0 ]] && SUDO="sudo"
  echo "==> パッケージをインストール"
  $SUDO apt-get update
  $SUDO apt-get install -y "${PACKAGES[@]}"
fi

for cmd in cmake ninja gcc clangd; do
  command -v "$cmd" > /dev/null || {
    echo "必須コマンドがありません: $cmd" >&2
    exit 1
  }
done

configure() {
  local dir="$1" type="$2"
  echo "==> cmake 構成: $dir ($type)"
  cmake -S . -B "$dir" -G Ninja \
    -DCMAKE_BUILD_TYPE="$type" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
}

# clangd は build/compile_commands.json を参照する (.vscode/settings.json 参照)
configure build Debug
if [[ $ALL_CONFIGS -eq 1 ]]; then
  configure build_cov Coverage
  configure build_asan Address
fi

echo "==> ビルド (build/)"
cmake --build build

echo
echo "完了。VS Code で clangd 拡張を入れ、'clangd: Restart language server' を実行してください。"
