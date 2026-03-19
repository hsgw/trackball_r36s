#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  build.sh  —  CMake → Ninja ビルド
# ─────────────────────────────────────────────────────────────
#  注意: 依存パッケージがインストールされていない場合は、
#  事前に ./setup_deps.sh を実行してください。
# ─────────────────────────────────────────────────────────────
set -e

echo "==> ビルドディレクトリを作成します..."
mkdir -p build
cd build

echo "==> CMake を構成します..."
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "==> ビルドします..."
ninja

# 実行権限の付与
chmod +x trackball_demo

echo ""
echo "✓ ビルド完了: build/trackball_demo"
echo ""
echo "実行方法:"
echo "  ./build/trackball_demo"
echo ""
echo "WSL2 (WSLg なし) の場合は事前に DISPLAY を設定してください:"
echo "  export DISPLAY=:0"
