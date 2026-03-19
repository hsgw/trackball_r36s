#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  install_locally.sh  —  本体の /roms/ports への配置
# ─────────────────────────────────────────────────────────────
set -e

# 配布物が準備されているか確認
if [ ! -d "dist/ports" ]; then
    echo "Error: dist/ports が見つかりません。先に ./make_dist.sh を実行してください。"
    exit 1
fi

# R36S (ArkOS等) の標準的なパスを指定
# SDカードの構成により /roms/ports または /mnt/sdcard2/roms/ports などの可能性があります
TARGET_DIR="/roms/ports"

if [ ! -d "$TARGET_DIR" ]; then
    echo "Error: ポート用ディレクトリ $TARGET_DIR が見つかりません。"
    echo "パスを確認してください。"
    exit 1
fi

echo "==> $TARGET_DIR へインストールを開始します..."
cp -rv dist/ports/* "$TARGET_DIR/"

# exFATなどで権限が失われた場合に備えて再付与
chmod +x "$TARGET_DIR/trackball/trackball_demo"
chmod +x "$TARGET_DIR/Trackball Demo.sh"

echo ""
echo "✓ インストール完了！"
echo "EmulationStation の Ports メニューで起動を確認してください。"
echo ""
