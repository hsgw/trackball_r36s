#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  install.sh  —  R36S 用の配布ディレクトリ (dist) を作成
# ─────────────────────────────────────────────────────────────
set -e

# ビルドが完了しているか確認
if [ ! -f "build/trackball_demo" ]; then
    echo "Error: build/trackball_demo が見つかりません。先に ./build.sh を実行してください。"
    exit 1
fi

echo "==> 配布用ディレクトリ dist/ を作成します..."
rm -rf dist
mkdir -p dist/ports/trackball

echo "==> ファイルをコピーします..."
cp build/trackball_demo dist/ports/trackball/
cp src/PressStart2P-Regular.ttf dist/ports/trackball/

echo "==> 起動用スクリプトを生成します..."
cat << 'EOF' > "dist/ports/Trackball Demo.sh"
#!/bin/bash
# R36S Ports Launcher Script

# 1. 作業ディレクトリに移動
cd /roms/ports/trackball/

# 2. 実行 (コンソール出力を抑制しない場合はそのまま)
./trackball_demo
EOF

echo "==> 実行権限を付与します..."
chmod +x dist/ports/trackball/trackball_demo
chmod +x "dist/ports/Trackball Demo.sh"

echo ""
echo "✓ 準備完了: dist/ フォルダの内容を R36S の SDカードの /roms/ パスへコピーしてください。"
echo ""
echo "コピー先イメージ:"
echo "  dist/ports/Trackball Demo.sh  ->  /roms/ports/Trackball Demo.sh"
echo "  dist/ports/trackball/         ->  /roms/ports/trackball/"
echo ""
