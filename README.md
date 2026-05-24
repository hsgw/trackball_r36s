# Trackball Demo for R36S

R36S (ArkOS / RetroArch ベースの携帯ゲーム機) 向けのトラックボール入力テスト用デモアプリケーションです。
SDL2 を使用して、トラックボールによるカーソル操作、スクロール、ボタン入力を視覚化します。

## 特徴

- **スムーズなカーソル操作**: 軌跡（トレイル）表示付き。
- **滑らかなスクロールバー**: マウスホイール（トラックボールの第3軸）に対応し、1ピクセル単位の微細な動きとスムーズな補間を実現。
- **ボタン入力表示**: R36S の各ボタンおよびトラックボールのクリックに対応。
- **レトロなデザイン**: フォスファースクリーン風の配色と 8-bit 風フォント（Press Start 2P）を採用。

## ディレクトリ構造

- `src/`: ソースコード (`main.cpp`)
- `assets/`: フォントなどのアセットファイル
- `build/`: ビルド成果物（一時ディレクトリ）
- `dist/`: R36S インストール用パッケージ（`make_dist.sh` で生成）

## セットアップとビルド

実機（R36S）または同様の Debian/Ubuntu ベースの Linux 環境でビルドできます。

### (OPTION) 効果音

hit.wavとshot.wavを用意して`assets/`へ配置してください。

### 1. 依存関係のインストール

最初に一度だけ実行してください。

```bash
chmod +x setup_deps.sh
./setup_deps.sh
```

### 2. ビルド

```bash
chmod +x build.sh
./build.sh
```

`build/trackball_demo` が生成されます。

## R36S へのインストール

実機上でビルドし、そのまま本体の Ports セクションに登録する場合の手順です。

### 1. インストール用パッケージの作成

```bash
chmod +x make_dist.sh
./make_dist.sh
```

`dist/` ディレクトリに必要なファイル（バイナリ、フォント、起動スクリプト）が集約されます。

### 2. 本体の Ports フォルダへコピー

```bash
chmod +x install_locally.sh
./install_locally.sh
```

これにより、`/roms/ports/` にアプリケーションが配置されます。

### 3. 起動

EmulationStation の **Ports** メニューから 「**Trackball Demo**」を選択して起動してください。

## ライセンス

フォント: [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P) ([SIL Open Font License](https://openfontlicense.org/documents/OFL.md))

コード: MIT License ((c) 2026, Takuya Urakawa (5z6p.com))
