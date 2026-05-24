#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>

// ─────────────────────────────────────────────────────────────
//  Layout  (640 x 480)
// ─────────────────────────────────────────────────────────────
static const int W = 640;
static const int H = 480;
static const int SCROLL_W = 60; // スクロールバー幅 (少し拡大)
static const int TRACK_W = 32;  // トラック幅 (少し拡大)
static const int BTN_H = 100;   // ボタンエリア高さ (拡大)
static const int CUR_W = W - SCROLL_W; // カーソルエリア幅
static const int CUR_H = H - BTN_H;    // カーソルエリア高さ

// ─────────────────────────────────────────────────────────────
//  Colors  (フォスファースクリーン風)
// ─────────────────────────────────────────────────────────────
static const SDL_Color C_BG = {0x1a, 0x0a, 0x2e, 0xff};
static const SDL_Color C_BG_DARK = {0x11, 0x08, 0x20, 0xff};
static const SDL_Color C_BORDER = {0x3a, 0x1a, 0x00, 0xff};
static const SDL_Color C_GRID = {0x22, 0x10, 0x3a, 0xff}; // グリッド用
static const SDL_Color C_DIM = {0x5a, 0x3a, 0x00, 0xff};
static const SDL_Color C_MID = {0xc8, 0x86, 0x0a, 0xff};
static const SDL_Color C_BRIGHT = {0xff, 0xcc, 0x44, 0xff};
static const SDL_Color C_TRACK_BG = {0x0d, 0x06, 0x18, 0xff};
static const SDL_Color C_BTN_OFF = {0x1c, 0x10, 0x08,
                                    0xff}; // 非押下ボタンを#1c1008に設定
static const SDL_Color C_BTN_TXT = {0x1a, 0x0a, 0x2e, 0xff};

static void setCol(SDL_Renderer *r, SDL_Color c) {
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

// ─────────────────────────────────────────────────────────────
//  Trail（カーソル軌跡）
// ─────────────────────────────────────────────────────────────
struct TrailPt {
  float x, y;
  Uint32 ms;
};
static std::deque<TrailPt> trail;
static const Uint32 TRAIL_MS = 1000; // 短くして負荷軽減 (2000 -> 1000)

// ─────────────────────────────────────────────────────────────
//  Cursor
// ─────────────────────────────────────────────────────────────
static float curX = CUR_W * 0.5f;
static float curY = CUR_H * 0.5f;

// ─────────────────────────────────────────────────────────────
//  Scroll
// ─────────────────────────────────────────────────────────────
static float scrollPos = 0.5f;
static float targetScrollPos = 0.5f;
static Uint32 lastScrollMs = 0;
static int scrollDir = 0; // -1=上, +1=下
static Uint32 lastDirMs = 0;
static const Uint32 RETURN_DELAY = 1200;
static const Uint32 ARROW_GLOW = 300;

// ─────────────────────────────────────────────────────────────
//  Game Constants & Enums (UFO Hunter)
// ─────────────────────────────────────────────────────────────
enum GameMode { MODE_DEMO, MODE_COUNTDOWN, MODE_GAME, MODE_RESULT };
static GameMode currentMode = MODE_DEMO;

// Game State
static Uint32 gameTimer = 0;
static Uint32 stateStartTime = 0;
static int score = 0;
static int combo = 0;
static Uint32 lastHitTime = 0;
static Uint32 lastFireTime = 0;
static int highScore = 0;

// Audio (Optional)
static Mix_Chunk *sndShot = nullptr;
static Mix_Chunk *sndHit = nullptr;

// UFO Definition
static const float UFO_W = 48.0f; // 16 * 3
static const float UFO_H = 36.0f; // 12 * 3
static const float HIT_R = 39.0f; // Hitbox radius for interaction (26 * 1.5)

struct LureUFO {
  float x, y;
  float baseX, baseY; // Target X, Base Y
  float t;
  bool active;
};
static LureUFO lure = {-100, 0, 20, 0, 0, false};

struct EnemyUFO {
  float x, y;
  float vx, vy;
  float t;
  float squish; // 0.0 (normal) to 1.0 (flat)
  bool active;
};
static std::deque<EnemyUFO> enemies;

// 0:None, 1:D(#c8860a), 2:M(#e8a020), 3:B(#ffcc44)
static const int UFO_SPRITE[12][16] = {
    /*         0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 */
    /* r 0 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* r 1 */ {0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 0, 0, 0, 0, 0, 0},
    /* r 2 */ {0, 0, 0, 0, 3, 1, 2, 0, 0, 2, 3, 3, 0, 0, 0, 0},
    /* r 3 */ {0, 0, 0, 0, 3, 1, 0, 1, 2, 0, 2, 3, 0, 0, 0, 0},
    /* r 4 */ {0, 0, 0, 3, 1, 1, 0, 2, 1, 0, 2, 3, 3, 0, 0, 0},
    /* r 5 */ {0, 0, 0, 3, 1, 1, 2, 0, 0, 2, 3, 3, 3, 0, 0, 0},
    /* r 6 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* r 7 */ {0, 0, 3, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 0, 0},
    /* r 8 */ {0, 3, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 0},
    /* r 9 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    /* r10 */ {0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0},
    /* r11 */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// ─────────────────────────────────────────────────────────────
//  Buttons
// ─────────────────────────────────────────────────────────────
struct Btn {
  SDL_Rect r;
  const char *lbl;
  SDL_Scancode key;
  bool on;
  SDL_Texture *texOn;
  SDL_Texture *texOff;
  int tw, th;
};
static Btn btns[6];

// ─────────────────────────────────────────────────────────────
//  三角形塗りつぶし（矢印描画用）
// ─────────────────────────────────────────────────────────────
static void fillTri(SDL_Renderer *r, int x0, int y0, int x1, int y1, int x2,
                    int y2, SDL_Color c) {
  if (y0 > y1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  if (y1 > y2) {
    std::swap(x1, x2);
    std::swap(y1, y2);
  }
  if (y0 > y1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  setCol(r, c);
  for (int y = y0; y <= y2; y++) {
    float t02 = (y2 == y0) ? 0.f : (float)(y - y0) / (y2 - y0);
    float lx = x0 + (x2 - x0) * t02;
    float rx = lx;
    if (y <= y1) {
      float t01 = (y1 == y0) ? 0.f : (float)(y - y0) / (y1 - y0);
      rx = x0 + (x1 - x0) * t01;
    } else {
      float t12 = (y2 == y1) ? 0.f : (float)(y - y1) / (y2 - y1);
      rx = x1 + (x2 - x1) * t12;
    }
    if (lx > rx)
      std::swap(lx, rx);
    SDL_RenderDrawLine(r, (int)lx, y, (int)rx, y);
  }
}

// ─────────────────────────────────────────────────────────────
//  テキストテクスチャ生成
// ─────────────────────────────────────────────────────────────
static SDL_Texture *makeTex(SDL_Renderer *r, TTF_Font *f, const char *s,
                            SDL_Color c, int *tw, int *th) {
  SDL_Surface *sf = TTF_RenderUTF8_Blended(f, s, c);
  if (!sf)
    return nullptr;
  *tw = sf->w;
  *th = sf->h;
  SDL_Texture *t = SDL_CreateTextureFromSurface(r, sf);
  SDL_FreeSurface(sf);
  return t;
}

static void initBtns(SDL_Renderer *r, TTF_Font *f) {
  const int P = 4;
  const int RH = (BTN_H - P * 3) / 2;
  const int Y0 = CUR_H + P;
  const int Y1 = Y0 + RH + P;
  const int TW = (W - P * 3) / 2;
  const int SW = (W - P * 5) / 4;

  btns[0] = {{P, Y0, TW, RH},
             "TOUCH 1",
             SDL_SCANCODE_F15,
             false,
             nullptr,
             nullptr,
             0,
             0};
  btns[1] = {{P * 2 + TW, Y0, TW, RH},
             "TOUCH 2",
             SDL_SCANCODE_F16,
             false,
             nullptr,
             nullptr,
             0,
             0};

  const SDL_Scancode swKeys[4] = {SDL_SCANCODE_A, SDL_SCANCODE_S,
                                  SDL_SCANCODE_D, SDL_SCANCODE_F};
  const char *swLabels[4] = {"SW 1", "SW 2", "SW 3", "SW 4"};
  for (int i = 0; i < 4; i++) {
    btns[2 + i] = {{P * (i + 1) + SW * i, Y1, SW, RH},
                   swLabels[i],
                   swKeys[i],
                   false,
                   nullptr,
                   nullptr,
                   0,
                   0};
  }

  for (auto &b : btns) {
    b.texOn = makeTex(r, f, b.lbl, C_BTN_TXT, &b.tw, &b.th);
    b.texOff = makeTex(r, f, b.lbl, C_DIM, &b.tw, &b.th);
  }
}

static void destroyBtns() {
  for (auto &b : btns) {
    if (b.texOn)
      SDL_DestroyTexture(b.texOn);
    if (b.texOff)
      SDL_DestroyTexture(b.texOff);
  }
}

// ─────────────────────────────────────────────────────────────
//  描画関数群
// ─────────────────────────────────────────────────────────────
static void renderCursorArea(SDL_Renderer *r) {
  SDL_Rect area = {0, 0, CUR_W, CUR_H};
  setCol(r, C_BG);
  SDL_RenderFillRect(r, &area);
  setCol(r, C_GRID);
  for (int x = 40; x < CUR_W; x += 40)
    SDL_RenderDrawLine(r, x, 0, x, CUR_H);
  for (int y = 40; y < CUR_H; y += 40)
    SDL_RenderDrawLine(r, 0, y, CUR_W, y);
  setCol(r, C_BORDER);
  SDL_RenderDrawRect(r, &area);
}

static void renderTrail(SDL_Renderer *r, Uint32 now) {
  SDL_Rect clip = {0, 0, CUR_W, CUR_H};
  SDL_RenderSetClipRect(r, &clip);
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

  // 負荷軽減のため、1つ飛ばしで描画（または間引き）
  for (size_t i = 1; i < trail.size(); i++) {
    float age = std::min(1.f, (float)(now - trail[i].ms) / TRAIL_MS);
    float alpha = powf(1.f - age, 1.5f);
    if (alpha <= 0.05f)
      continue;

    Uint8 a = (Uint8)(alpha * 200.f);
    SDL_SetRenderDrawColor(r, 220, 120, 0, a);

    int x1 = (int)trail[i - 1].x, y1 = (int)trail[i - 1].y;
    int x2 = (int)trail[i].x, y2 = (int)trail[i].y;
    SDL_RenderDrawLine(r, x1, y1, x2, y2);
    // 太線描画を1本に減らして負荷軽減
    SDL_RenderDrawLine(r, x1 + 1, y1 + 1, x2 + 1, y2 + 1);
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
  SDL_RenderSetClipRect(r, nullptr);
}

static void renderCursor(SDL_Renderer *r) {
  int x = (int)curX, y = (int)curY, s = 10;
  setCol(r, C_BRIGHT);
  SDL_RenderDrawLine(r, x, y - s, x, y + s);
  SDL_RenderDrawLine(r, x - s, y, x + s, y);
}

static void renderScrollBar(SDL_Renderer *r, Uint32 now) {
  SDL_Rect sb = {CUR_W, 0, SCROLL_W, CUR_H};
  setCol(r, C_BG_DARK);
  SDL_RenderFillRect(r, &sb);
  setCol(r, C_BORDER);
  SDL_RenderDrawRect(r, &sb);

  bool active = (now - lastDirMs) < ARROW_GLOW;
  SDL_Color upC = (active && scrollDir == -1) ? C_BRIGHT : C_DIM;
  SDL_Color dwnC = (active && scrollDir == 1) ? C_BRIGHT : C_DIM;

  int cx = CUR_W + SCROLL_W / 2;
  int aw = 12, ah = 14;
  fillTri(r, cx, 5, cx - aw, 5 + ah, cx + aw, 5 + ah, upC);
  fillTri(r, cx - aw, CUR_H - 5 - ah, cx + aw, CUR_H - 5 - ah, cx, CUR_H - 5,
          dwnC);

  int arrowZone = 5 + ah + 6;
  int trackX = CUR_W + (SCROLL_W - TRACK_W) / 2;
  int trackY = arrowZone;
  int trackH = CUR_H - arrowZone * 2;
  SDL_Rect tr = {trackX, trackY, TRACK_W, trackH};
  setCol(r, C_TRACK_BG);
  SDL_RenderFillRect(r, &tr);
  setCol(r, C_BORDER);
  SDL_RenderDrawRect(r, &tr);

  int indH = (int)(trackH * 0.12f);
  int indTop = trackY + (int)(scrollPos * (trackH - indH));

  if (active) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0xc8, 0x86, 0x0a, 55);
    if (scrollDir == -1) {
      int hlH = indTop - trackY;
      if (hlH > 0) {
        SDL_Rect hl = {trackX, trackY, TRACK_W, hlH};
        SDL_RenderFillRect(r, &hl);
      }
    } else if (scrollDir == 1) {
      int belowY = indTop + indH;
      int hlH = (trackY + trackH) - belowY;
      if (hlH > 0) {
        SDL_Rect hl = {trackX, belowY, TRACK_W, hlH};
        SDL_RenderFillRect(r, &hl);
      }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
  }

  setCol(r, C_BORDER);
  SDL_RenderDrawLine(r, trackX, trackY + trackH / 2, trackX + TRACK_W,
                     trackY + trackH / 2);

  SDL_Rect ir = {trackX, indTop, TRACK_W, indH};
  setCol(r, C_MID);
  SDL_RenderFillRect(r, &ir);
}

static void renderBtns(SDL_Renderer *r) {
  SDL_Rect bg = {0, CUR_H, W, BTN_H};
  setCol(r, C_BG_DARK);
  SDL_RenderFillRect(r, &bg);
  setCol(r, C_BORDER);
  SDL_RenderDrawLine(r, 0, CUR_H, W, CUR_H);

  for (auto &b : btns) {
    setCol(r, b.on ? C_MID : C_BTN_OFF);
    SDL_RenderFillRect(r, &b.r);
    setCol(r, b.on ? C_BRIGHT : C_BORDER);
    SDL_RenderDrawRect(r, &b.r);

    SDL_Texture *t = b.on ? b.texOn : b.texOff;
    if (t) {
      SDL_Rect dst = {b.r.x + (b.r.w - b.tw) / 2, b.r.y + (b.r.h - b.th) / 2,
                      b.tw, b.th};
      SDL_RenderCopy(r, t, nullptr, &dst);
    }
  }
}

static void renderScanlines(SDL_Renderer *r) {
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(r, 0, 0, 0, 15);
  // 負荷軽減のため間隔を広げる (4 -> 6)
  for (int y = 0; y < H; y += 6) {
    SDL_RenderDrawLine(r, 0, y, W, y);
  }
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

// ─────────────────────────────────────────────────────────────
//  Game Logic & Rendering
// ─────────────────────────────────────────────────────────────
static void drawPixelSprite(SDL_Renderer *r, float x, float y, float squish) {
  const int S = 3; // Scale (1.5x larger: 2 -> 3)
  float sy = (squish > 0) ? 1.0f + squish * 0.6f : 1.0f;
  float ox = x - 8 * S;

  for (int row = 0; row < 12; row++) {
    for (int col = 0; col < 16; col++) {
      int c = UFO_SPRITE[row][col];
      if (c == 0)
        continue;

      if (c == 1)
        SDL_SetRenderDrawColor(r, 0xc8, 0x86, 0x0a, 255); // D
      else if (c == 2)
        SDL_SetRenderDrawColor(r, 0xe8, 0xa0, 20, 255); // M
      else if (c == 3)
        SDL_SetRenderDrawColor(r, 0xff, 0xcc, 0x44, 255); // B

      float dy = (row - 6) * S * sy + 6 * S; // Apply scaling from center Y
      SDL_Rect rect = {(int)(ox + col * S), (int)(y - (6 * S) + dy), S,
                       (int)(S * sy)};
      SDL_RenderFillRect(r, &rect);
    }
  }
}

static void updateLure(Uint32) {
  bool inZone = (curX < CUR_W * 0.4f) && (curY > CUR_H * 0.6f);

  // Activation Logic
  if (inZone)
    lure.active = true;
  else if (lure.x < -20)
    lure.active = false;

  // Movement
  float targetX = inZone ? 30.0f : -30.0f;
  lure.x += (targetX - lure.x) * 0.08f;
  lure.t += 0.05f;
  lure.baseY = CUR_H - 18.0f;
  lure.y = lure.baseY + sinf(lure.t) * 5.0f;
}

static void startGame(Uint32 now) {
  currentMode = MODE_COUNTDOWN;
  stateStartTime = now;
  enemies.clear();
  score = 0;
  combo = 0;
  gameTimer = 30;
}

static void updateGame(Uint32 now) {
  // Timer
  if (currentMode == MODE_GAME) {
    Uint32 elapsed = now - stateStartTime;
    if (elapsed > 30000) { // 30 sec
      gameTimer = 0;
      currentMode = MODE_RESULT;
      stateStartTime = now;
      if (score > highScore)
        highScore = score;
      return;
    }
    gameTimer = 30 - (elapsed / 1000);

    // Spawn Logic (1 initial + 1 every 5 sec, max 6)
    size_t maxEnemies = 1 + (elapsed / 5000);
    if (maxEnemies > 6)
      maxEnemies = 6;

    while (enemies.size() < maxEnemies) {
      float vx = (rand() % 200 - 100) / 100.0f + 0.5f;
      float vy = (rand() % 200 - 100) / 100.0f + 0.5f;
      enemies.push_back({(float)(rand() % (CUR_W - 40) + 20),
                         (float)(rand() % (CUR_H - 40) + 20), vx, vy, 0, 0,
                         true});
    }
  }

  // Update Enemies
  for (auto &e : enemies) {
    if (!e.active)
      continue;

    if (e.squish > 0) {
      e.squish -= 0.1f;
      if (e.squish < 0)
        e.squish = 0;
    } else {
      // Move
      float speedMult = 1.0f + (30 - gameTimer) / 20.0f;
      e.x += e.vx * 1.44f * speedMult; // 0.8x slower: 1.8 * 0.8 = 1.44
      e.y += e.vy * 1.44f * speedMult;

      // Bounce (Update margin for 1.5x size: 16 -> 24)
      if (e.x < 24)
        e.vx = fabsf(e.vx);
      if (e.x > CUR_W - 24)
        e.vx = -fabsf(e.vx);
      if (e.y < 24)
        e.vy = fabsf(e.vy);
      if (e.y > CUR_H - 24)
        e.vy = -fabsf(e.vy);

      // Feint
      if (rand() % 100 < 1) { // 1% chance
        e.vx = (rand() % 200 - 100) / 100.0f;
        e.vy = (rand() % 200 - 100) / 100.0f;
      }
    }
  }
}

static void checkHits(Uint32 now) {
  if (currentMode != MODE_GAME)
    return;
  lastFireTime = now;
  if (sndShot)
    Mix_PlayChannel(-1, sndShot, 0);

  for (auto it = enemies.begin(); it != enemies.end();) {
    float dx = curX - it->x;
    float dy = curY - it->y;
    if (sqrtf(dx * dx + dy * dy) < HIT_R) {
      // Hit!
      it = enemies.erase(it); // Remove immediately
      if (sndHit)
        Mix_PlayChannel(-1, sndHit, 0);

      // Combo
      if (now - lastHitTime < 1200)
        combo++;
      else
        combo = 1;
      lastHitTime = now;

      int pts = (combo >= 3) ? 10 * combo : 10;
      score += pts;
      break; // Single shot limit
    } else {
      ++it;
    }
  }
}

static void renderGameOverlay(SDL_Renderer *r, TTF_Font *f, Uint32 now) {
  if (currentMode == MODE_DEMO) {
    if (lure.x > -20)
      drawPixelSprite(r, lure.x, lure.y, 0);
    return;
  }

  // Render Enemies
  for (const auto &e : enemies) {
    drawPixelSprite(r, e.x, e.y, e.squish);
  }

  // HUD
  if (currentMode == MODE_GAME) {
    // Draw Crosshair
    bool flash = (now - lastFireTime < 120);
    SDL_Color crossCol = flash ? (SDL_Color){255, 255, 255, 255} : C_BRIGHT;
    setCol(r, crossCol);

    int cx = (int)curX, cy = (int)curY;

    // 1. Center Dot (2x2)
    SDL_Rect centerDot = {cx - 1, cy - 1, 2, 2};
    SDL_RenderFillRect(r, &centerDot);

    // 2. Inner Circle (radius 4)
    SDL_RenderDrawPoint(r, cx - 4, cy);
    SDL_RenderDrawPoint(r, cx + 4, cy);
    SDL_RenderDrawPoint(r, cx, cy - 4);
    SDL_RenderDrawPoint(r, cx, cy + 4);
    SDL_RenderDrawPoint(r, cx - 3, cy - 3);
    SDL_RenderDrawPoint(r, cx + 3, cy - 3);
    SDL_RenderDrawPoint(r, cx - 3, cy + 3);
    SDL_RenderDrawPoint(r, cx + 3, cy + 3);

    // 3. Outer Circle (radius 14)
    for (int a = 0; a < 360; a += 15) {
      float rad = a * M_PI / 180.0f;
      SDL_RenderDrawPoint(r, (int)(cx + cosf(rad) * 14),
                          (int)(cy + sinf(rad) * 14));
    }

    // 4. Crosshair Lines (gap 3 from inner circle (r=4+3=7) to outer (r=14))
    SDL_RenderDrawLine(r, cx - 14, cy, cx - 7, cy);
    SDL_RenderDrawLine(r, cx + 14, cy, cx + 7, cy);
    SDL_RenderDrawLine(r, cx, cy - 14, cx, cy - 7);
    SDL_RenderDrawLine(r, cx, cy + 14, cx, cy + 7);

    // 5. Fire Flash Circle
    if (flash) {
      SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(r, 255, 255, 255, 100);
      for (int a = 0; a < 360; a += 10) {
        float rad = a * M_PI / 180.0f;
        SDL_RenderDrawPoint(r, (int)(cx + cosf(rad) * 17),
                            (int)(cy + sinf(rad) * 17));
      }
      SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    // Text
    char buf[64];
    int tw = 0, th = 0;

    snprintf(buf, 64, "TIME:%02d", gameTimer);
    SDL_Texture *tTime = makeTex(r, f, buf, C_BRIGHT, &tw, &th);
    // 少し小さめに表示 (元のサイズの 0.8倍など)
    SDL_Rect rTime = {10, 10, (int)(tw * 0.8f), (int)(th * 0.8f)};
    SDL_RenderCopy(r, tTime, NULL, &rTime);
    SDL_DestroyTexture(tTime);

    snprintf(buf, 64, "SCORE:%d", score);
    SDL_Texture *tScore = makeTex(r, f, buf, C_BRIGHT, &tw, &th);
    SDL_Rect rScore = {CUR_W - (int)(tw * 0.8f) - 10, 10, (int)(tw * 0.8f),
                       (int)(th * 0.8f)};
    SDL_RenderCopy(r, tScore, NULL, &rScore);
    SDL_DestroyTexture(tScore);
    if (combo >= 2) {
      snprintf(buf, 64, "%d COMBO!", combo);
      SDL_Texture *tCombo = makeTex(r, f, buf, {255, 100, 100, 255}, &tw, &th);
      // 同じく 0.8倍に縮小
      SDL_Rect rCombo = {CUR_W / 2 - (int)(tw * 0.8f) / 2, 10, (int)(tw * 0.8f),
                         (int)(th * 0.8f)};
      SDL_RenderCopy(r, tCombo, NULL, &rCombo);
      SDL_DestroyTexture(tCombo);
    }
  }

  // Overlays
  if (currentMode == MODE_COUNTDOWN || currentMode == MODE_RESULT) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 26, 10, 46, 220);
    SDL_Rect overlay = {0, 0, CUR_W, CUR_H};
    SDL_RenderFillRect(r, &overlay);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    if (currentMode == MODE_COUNTDOWN) {
      int cnt = 3 - (now - stateStartTime) / 1000;
      if (cnt <= 0) {
        currentMode = MODE_GAME;
        stateStartTime = now;
      } else {
        char buf[2];
        snprintf(buf, 2, "%d", cnt);
        int tw = 0, th = 0;
        SDL_Texture *t = makeTex(r, f, buf, C_BRIGHT, &tw, &th);
        // Scale up by render copy
        SDL_Rect dst = {CUR_W / 2 - tw * 2, CUR_H / 2 - th * 2, tw * 4, th * 4};
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_DestroyTexture(t);

        SDL_Texture *t2 = makeTex(r, f, "GET READY!", C_MID, &tw, &th);
        SDL_Rect dst2 = {CUR_W / 2 - tw / 2, CUR_H / 2 + 60, tw, th};
        SDL_RenderCopy(r, t2, NULL, &dst2);
        SDL_DestroyTexture(t2);
      }
    } else if (currentMode == MODE_RESULT) {
      int tw = 0, th = 0;
      SDL_Texture *t = makeTex(r, f, "TIME UP!", C_BRIGHT, &tw, &th);
      SDL_Rect dst = {CUR_W / 2 - tw, CUR_H / 2 - 80, tw * 2, th * 2};
      SDL_RenderCopy(r, t, NULL, &dst);
      SDL_DestroyTexture(t);

      char buf[64];
      snprintf(buf, 64, "SCORE: %d", score);
      t = makeTex(r, f, buf, C_BRIGHT, &tw, &th);
      dst = {CUR_W / 2 - tw / 2, CUR_H / 2 - 10, tw, th};
      SDL_RenderCopy(r, t, NULL, &dst);
      SDL_DestroyTexture(t);

      snprintf(buf, 64, "BEST: %d", highScore);
      t = makeTex(r, f, buf, C_DIM, &tw, &th);
      dst = {CUR_W / 2 - tw / 2, CUR_H / 2 + 25, tw, th};
      SDL_RenderCopy(r, t, NULL, &dst);
      SDL_DestroyTexture(t);

      // 3秒経過後に「CLICK TO EXIT」を表示
      if (now - stateStartTime > 3000) {
        t = makeTex(r, f, "CLICK TO EXIT", C_BRIGHT, &tw, &th);
        dst = {CUR_W / 2 - tw / 2, CUR_H / 2 + 80, tw, th};
        SDL_RenderCopy(r, t, NULL, &dst);
        SDL_DestroyTexture(t);
      }
    }
  }
}

// ─────────────────────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────────────────────
int main(int, char **) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    return 1;
  if (TTF_Init() != 0)
    return 1;

  // Audio setup
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    SDL_Log("Mix_OpenAudio error: %s", Mix_GetError());
  } else {
    // Try to load sounds from assets/ or current dir
    const char *pathsShot[] = {"assets/shot.wav", "shot.wav", nullptr};
    for (int i = 0; pathsShot[i]; i++) {
      sndShot = Mix_LoadWAV(pathsShot[i]);
      if (sndShot) {
        SDL_Log("Shot sound loaded: %s", pathsShot[i]);
        break;
      }
    }
    const char *pathsHit[] = {"assets/hit.wav", "hit.wav", nullptr};
    for (int i = 0; pathsHit[i]; i++) {
      sndHit = Mix_LoadWAV(pathsHit[i]);
      if (sndHit) {
        SDL_Log("Hit sound loaded: %s", pathsHit[i]);
        break;
      }
    }
  }

  SDL_Window *win =
      SDL_CreateWindow("Trackball Demo", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
  if (!win)
    return 1;

  // R36S等では PRESENTVSYNC が重い場合があるが、まずは有効で試す
  SDL_Renderer *ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren)
    return 1;

  TTF_Font *font = nullptr;
  // 実行ファイルの場所からの相対パス（buildディレクトリで実行されることを想定）
  // 開発環境と本番の両方で動作するように複数の候補を指定
  const char *fontCandidates[] = {
      "PressStart2P-Regular.ttf", "../assets/PressStart2P-Regular.ttf",
      "assets/PressStart2P-Regular.ttf",
      "../src/PressStart2P-Regular.ttf", // 互換性のため残す
      nullptr};
  for (int i = 0; fontCandidates[i]; i++) {
    font = TTF_OpenFont(fontCandidates[i], 22); // サイズを 15 -> 22 (約1.5倍)
    if (font) {
      SDL_Log("Font loaded: %s", fontCandidates[i]);
      break;
    }
  }
  if (!font) {
    SDL_Log("Failed to load local font, trying system fonts...");
    const char *sysFontCandidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", nullptr};
    for (int i = 0; sysFontCandidates[i]; i++) {
      font = TTF_OpenFont(sysFontCandidates[i], 22);
      if (font)
        break;
    }
  }
  if (!font)
    return 1;

  initBtns(ren, font);

  bool run = true;
  SDL_Event ev;
  while (run) {
    Uint32 now = SDL_GetTicks();
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT)
        run = false;
      if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
          run = false;
        for (auto &b : btns)
          if (ev.key.keysym.scancode == b.key)
            b.on = true;
      }
      if (ev.type == SDL_KEYUP) {
        for (auto &b : btns)
          if (ev.key.keysym.scancode == b.key)
            b.on = false;
      }
      if (ev.type == SDL_MOUSEMOTION && ev.motion.x < CUR_W &&
          ev.motion.y < CUR_H) {
        curX = (float)ev.motion.x;
        curY = (float)ev.motion.y;
        trail.push_back({curX, curY, now});
      }
      if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP) {
        bool dn = (ev.type == SDL_MOUSEBUTTONDOWN);
        if (ev.button.button == SDL_BUTTON_LEFT)
          btns[2].on = dn;
        if (ev.button.button == SDL_BUTTON_MIDDLE)
          btns[3].on = dn;
        if (ev.button.button == SDL_BUTTON_RIGHT)
          btns[4].on = dn;
        if (ev.button.button == SDL_BUTTON_X1)
          btns[5].on = dn;
        if (ev.button.button == SDL_BUTTON_X2)
          btns[0].on = dn;
        SDL_Point p = {ev.button.x, ev.button.y};
        for (auto &b : btns)
          if (SDL_PointInRect(&p, &b.r))
            b.on = dn;

        // Game Interactions
        if (dn) {
          if (currentMode == MODE_DEMO && lure.active) {
            float dx = curX - lure.x;
            float dy = curY - lure.y;
            if (sqrtf(dx * dx + dy * dy) < HIT_R) {
              startGame(now);
            }
          } else if (currentMode == MODE_GAME) {
            checkHits(now);
          } else if (currentMode == MODE_RESULT) {
            // 3秒経過後ならどこをクリックしても戻る
            if (now - stateStartTime > 2000) {
              currentMode = MODE_DEMO;
              lure.x = -30;
            }
          }
        }
      }
      if (ev.type == SDL_MOUSEWHEEL) {
        // 可動範囲(trackH - indH)は約291px。1px移動 = 1.0f / 291.0f
        const float step = 1.0f / 291.0f;
        targetScrollPos = std::max(
            0.005f, std::min(0.995f, targetScrollPos + ev.wheel.y * -step));
        lastScrollMs = now;
        scrollDir = (ev.wheel.y < 0) ? 1 : -1;
        lastDirMs = now;
      }
    }

    while (!trail.empty() && (now - trail.front().ms) > TRAIL_MS)
      trail.pop_front();

    // Game Updates
    updateLure(now);
    updateGame(now);

    if ((now - lastScrollMs) > RETURN_DELAY) {
      targetScrollPos = 0.5f;
    }
    // スムーズな追従 (微細な動きに合わせ、少し速めの 0.3f に調整)
    float scrollDiff = targetScrollPos - scrollPos;
    if (std::abs(scrollDiff) > 0.0001f) {
      scrollPos += scrollDiff * 0.3f;
    } else {
      scrollPos = targetScrollPos;
    }

    setCol(ren, C_BG);
    SDL_RenderClear(ren);
    renderCursorArea(ren);
    renderTrail(ren, now);

    // Draw cursor only if not in game mode (replaced by crosshair)
    if (currentMode != MODE_GAME)
      renderCursor(ren);

    renderGameOverlay(ren, font, now); // Render game elements

    renderScrollBar(ren, now);
    renderBtns(ren);
    renderScanlines(ren);
    SDL_RenderPresent(ren);
  }

  if (sndShot)
    Mix_FreeChunk(sndShot);
  if (sndHit)
    Mix_FreeChunk(sndHit);
  Mix_CloseAudio();

  destroyBtns();
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
  return 0;
}
