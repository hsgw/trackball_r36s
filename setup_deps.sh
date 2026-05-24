#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  setup_deps.sh  —  依存パッケージのインストール
# ─────────────────────────────────────────────────────────────
set -e

echo "==> 依存パッケージをインストールします..."
sudo apt update -qq
sudo apt install -y \
    cmake ninja-build \
    libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev \
    pkg-config

echo "✓ 依存パッケージのインストールが完了しました。"
