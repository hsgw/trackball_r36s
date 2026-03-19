#!/bin/bash
# ─────────────────────────────────────────────────────────────
#  make_dist.sh  —  実機でのビルドと配布用ディレクトリの作成
# ─────────────────────────────────────────────────────────────
set -e

echo "==> プロジェクトをビルドします..."
./build.sh

echo "==> 配布用ディレクトリ dist/ を作成します..."
rm -rf dist
mkdir -p dist/ports/trackball

echo "==> 実行ファイルとフォントを dist/ へ集約中..."
cp build/trackball_demo dist/ports/trackball/
cp assets/PressStart2P-Regular.ttf dist/ports/trackball/

echo "==> 起動用スクリプトを生成中..."
cat << 'EOF' > "dist/ports/Trackball Demo.sh"
#!/bin/bash
# R36S Ports Launcher Script
cd /roms/ports/trackball/
./trackball_demo
EOF

chmod +x dist/ports/trackball/trackball_demo
chmod +x "dist/ports/Trackball Demo.sh"

echo ""
echo "✓ 完了: dist/ フォルダに配布物が準備されました。"
echo "次は ./install_locally.sh で本体にインストールできます。"
echo ""
