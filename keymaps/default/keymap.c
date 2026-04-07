// Copyright 2020 QMK / MudkipMao
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "keymap_korean.h" // 이걸 해줘야 KR_HAEN, KR_HANJ 를 쓸 수 있다.
#include "quantum.h"       // TAP_DANCE 등 가능하게. 원래 있던 코드.

enum custom_keycodes {
  TG_MOZC = SAFE_RANGE, // 일본어 자판 전환용.
                        // 가상키를 만들지 않고, 그냥 KC_INT2(case KC_INT2:)
                        // 상태에 따라 실제 발사될 키를 바꿔도 될 듯은 한데,
                        // 그냥 이렇게 해봤다.
                        // TEST_GRV = SAFE_RANGE + 1,
                        // MCTRL_H = SAFE_RANGE + 2, // H(Dvorak)을 mod-tap 으로
                        // 이용하기 위한...
  // PG_UPDN = SAFE_RANGE + 1, // PgUp/Dn 용인데.. 다른 방식으로 바꿨다.
};

// Tap Dance 用
enum {
  TD_PU_PD,
  TD_HM_ED,
};

// 각 자판 활성 Flag.
static bool mozc_jap = false;   // true 이면 mozc 활성화.
static bool hangul_kor = false; // true 이면 hangul 활성화.
// static bool dvorak_eng = false; // true 이면 영어 활성화. 아직 구현 전.

// for Dvorak/Qwerty Layer Change.
// LM : 키를 누르고 있으면 레이어가 바뀌고, Ctrl/Super/Alt 는 그 기능 그대로
// 작동하게끔.
#define LCDQ LM(_BAQT, MOD_LCTL)
#define LSDQ LM(_BAQT, MOD_LGUI)
#define LADQ LM(_BAQT, MOD_LALT)

// https://getreuer.info/posts/keyboards/triggers/index.html#tap-vs.-long-press
#define HM_CH LT(0, KC_HOME)
#define ED_CE LT(0, KC_END)

// Home Row Mode 用 (Tap Hold Workaround)
// 만들긴 했는데, 사용은 안하고 있다.
// for Dvorak (H Tap _ CTrl, T Tap _ ShiFt ....)
#define HT_CT LT(0, KC_H) // H : Ctrl
#define TT_SF LT(0, KC_T) // T : Shift
#define NT_AL LT(0, KC_N) // N : Alt
#define ST_GU LT(0, KC_S) // S : GUI
#define UT_CT LT(0, KC_U) // U : Ctrl
#define ET_SF LT(0, KC_E) // E : Shift
#define OT_AL LT(0, KC_O) // O : Alt
#define AT_GU LT(0, KC_A) // A : GUI

// for Qwerty
#define JT_CT LT(1, KC_J)     // J : Ctrl
#define KT_SF LT(1, KC_K)     // K : Shift
#define LT_AL LT(1, KC_L)     // L : Alt
#define SCT_GU LT(1, KC_SCLN) // Semicolon : GUI
#define FT_CT LT(1, KC_F)     // U : Ctrl
#define DT_SF LT(1, KC_D)     // E : Shift
#define ST_AL LT(1, KC_S)     // O : Alt
#define AQT_GU LT(1, KC_A)    // A : GUI Dvorak 과 같이 쓰면 안된다.

// Each layer gets a name for readability, which is then used in the keymap
// matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or
// any other name. Layer names don't all need to be of the same length,
// obviously, and you can also skip them entirely and just use numbers.

enum layer_names {
  _BADV,
  _BAQT,
  _NPD,
  _FN,
};

// 여기부터는 코드 정리 끄기. 정렬이 흩어진다.
// clang-format off

/* 키배열
왼쪽
┌──────┬──────┬──────┬──────┬─────┬────┬───────┐
│ ESC  │ F1   │ F2   │ F3   │ F4  │ F5 │ F6    │
├──────┼──────┼──────┼──────┼─────┼────┼───────┤
│ GRV  │ 1    │ 2    │ 3    │ 4   │ 5  │ BSPC  │
├──────┼──────┼──────┼──────┼─────┼────┼───────┤
│ TAB  │ Q    │ W    │ E    │ R   │ T  │ TG(1) │
├──────┼──────┼──────┼──────┼─────┼────┼───────┤
│ CAPS │ A    │ S    │ D    │ F   │ G  │       │
├──────┼──────┼──────┼──────┼─────┼────┼───────┤
│ LSFT │ Z    │ X    │ C    │ V   │ B  │       │
├──────┼──────┼──────┼──────┼─────┼────┼───────┤
│ LCTL │ LWIN │ NONE │ LALT │ SPC │    │       │
└──────┴──────┴──────┴──────┴─────┴────┴───────┘

오른쪽
┌─────┬──────┬───────┬─────────┬─────────┬──────────┬──────────┬─────────┬──────┐
│ F7  │ F8   │ F9    │ F10     │ F11     │ F12      │ F13      │ DEL     │ MUTE │
├─────┼──────┼───────┼─────────┼─────────┼──────────┼──────────┼─────────┼──────┤
│ 6   │ 7    │ 8     │ 9       │ 0       │ -(Minus) │ =(equal) │ BSPC    │ HOME │
├─────┼──────┼───────┼─────────┼─────────┼──────────┼──────────┼─────────┼──────┤
│ Y   │ U    │ I     │ O       │ P       │ [(LBRC)  │ ](RBRC)  │ \(BSLS) │ PGUP │
├─────┼──────┼───────┼─────────┼─────────┼──────────┼──────────┼─────────┼──────┤
│ H   │ J    │ K     │ L       │ ;(SCLN) │ (QUOT)   │ ENT      │         │ PGDN │
├─────┼──────┼───────┼─────────┼─────────┼──────────┼──────────┼─────────┼──────┤
│ B   │ N    │ M     │ ,(COMM) │ .(DOT)  │ /(SLSH)  │ RSFT     │ UP      │      │
├─────┼──────┼───────┼─────────┼─────────┼──────────┼──────────┼─────────┼──────┤
│ SPC │ RALT │ MO(1) │ RCTL    │         │          │ LEFT     │ DOWN    │ RGHT │
└─────┴──────┴───────┴─────────┴─────────┴──────────┴──────────┴─────────┴──────┘

*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BADV] = LAYOUT(
        // left hand
        KC_ESC,      KC_F1,       KC_F2,       KC_F3,       KC_F4,       KC_F5,       KC_F6,
        KC_GRV,      KC_1,        KC_2,        KC_3,        KC_4,        KC_5,        KC_BSPC,
        KC_TAB,      KC_QUOT,     KC_COMM,     KC_DOT,      KC_P,        KC_Y,        TG(_NPD),
        KC_CAPS,     KC_A,        KC_O,        KC_E,        KC_U,        KC_I,
        KC_LSFT,     KC_SCLN,     KC_Q,        KC_J,        KC_K,        KC_X,
        LCDQ,        LSDQ,        KC_NO,       LADQ,        KC_SPC,
        // right hand
        KC_F7,       KC_F8,       KC_F9,       KC_F10,      KC_F11,      KC_F12,      KC_PGDN,     KC_DEL,      KC_MUTE,
        KC_6,        KC_7,        KC_8,        KC_9,        KC_0,        KC_LBRC,     KC_RBRC,     KC_BSPC,     HM_CH,
        KC_F,        KC_G,        KC_C,        KC_R,        KC_L,        KC_SLSH,     KC_EQL,      KC_BSLS,     TD(TD_PU_PD),
        KC_D,        KC_H,        KC_T,        KC_N,        KC_S,        KC_MINS,                  KC_ENT,      ED_CE,
        KC_X,        KC_B,        KC_M,        KC_W,        KC_V,        KC_Z,        KC_RSFT,     KC_UP,
                     KC_SPC,                   KC_RALT,     MO(_FN),     KC_RCTL,     KC_LEFT,     KC_DOWN,     KC_RGHT
    ),

    [_BAQT] = LAYOUT(
        // left hand
        KC_ESC,      KC_F1,       KC_F2,       KC_F3,       KC_F4,       KC_F5,       KC_F6,
        KC_GRV,      KC_1,        KC_2,        KC_3,        KC_4,        KC_5,        KC_BSPC,
        KC_TAB,      KC_Q,        KC_W,        KC_E,        KC_R,        KC_T,        TG(_NPD),
        KC_CAPS,     KC_A,        KC_S,        KC_D,        KC_F,        KC_G,
        KC_LSFT,     KC_Z,        KC_X,        KC_C,        KC_V,        KC_B,
        KC_LCTL,     KC_LWIN,     KC_NO,       KC_LALT,                  KC_SPC,
        // right hand
        KC_F7,       KC_F8,       KC_F9,       KC_F10,      KC_F11,      KC_F12,      KC_NO,       KC_DEL,      KC_MUTE,
        KC_6,        KC_7,        KC_8,        KC_9,        KC_0,        KC_MINS,     KC_EQL,      KC_BSPC,     HM_CH,
        KC_Y,        KC_U,        KC_I,        KC_O,        KC_P,        KC_LBRC,     KC_RBRC,     KC_BSLS,     TD(TD_PU_PD),
        KC_H,        KC_J,        KC_K,        KC_L,        KC_SCLN,     KC_QUOT,                  KC_ENT,      ED_CE,
        KC_B,        KC_N,        KC_M,        KC_COMM,     KC_DOT,      KC_SLSH,     KC_RSFT,     KC_UP,
                     KC_SPC,                   KC_RALT,     MO(_FN),     KC_RCTL,     KC_LEFT,     KC_DOWN,     KC_RGHT
    ),

    [_FN] = LAYOUT(
        // left hand
        // ESC       F1(1)        F2(2)        F3(3)        F4(4)        F5(5)        F6()
        QK_BOOT,     DF(_BADV),   DF(_BAQT),   KC_NO,       KC_NO,       KC_NO,       KC_NO,
        KC_NO,       PDF(_BADV),  PDF(_BAQT),  KC_NO,       KC_NO,       KC_NO,       KC_NO,
        KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       TG(_NPD),
        KC_NO,       KC_NO,       KC_SCRL,     KC_NO,       KC_NO,       KC_NO,
        KC_NO,       KC_NO,       KC_NO,       KC_CALC,     KC_NO,       KC_NO,
        KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,
        // right hand
        // F7(6)     F8(7)        F9(8)        F10(9)       F11(0)       F12(-)       F13?(=)
        KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       TG(_NPD),    KC_NO,       KC_NO,       KC_NO,
        KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       HM_CH,
        KC_NO,       KC_NO,       KC_INS,      KC_NO,       KC_PSCR,     KC_NO,       KC_NO,       KC_NO,       KC_PGUP,
        KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,                    KC_NO,       KC_PGDN,
        KC_NO,       KC_NUM,      KC_NO,       KC_NO,       KC_NO,       KC_INT4,     TG_MOZC,     KC_NO,
                     KC_NO,                    KR_HAEN,     KC_NO,       KR_HANJ,     KC_NO,       KC_NO,       KC_NO
    ),

    [_NPD] = LAYOUT(
        // left hand
        // ESC       F1(1)        F2(2)        F3(3)        F4(4)        F5(5)        F6()
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_TRNS,
        KC_TRNS,     KC_NO,       KC_COMM,     KC_DOT,      KC_NO,       KC_NO,       TG(_NPD),
        KC_NO,       KC_PPLS,     KC_PMNS,     KC_PAST,     KC_PSLS,     KC_NO,
        KC_TRNS,     KC_NO,       KC_NO,       KC_CALC,     KC_NO,       KC_NO,
        KC_LCTL,     KC_LGUI,     KC_NO,       KC_LALT,     KC_NO,
        // right hand
        // F7(6)     F8(7)        F9(8)        F10(9)       F11(0)       F12(-)       F13?(=)
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_NUM,      KC_PSLS,     KC_PAST,     KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_TRNS,     TD(TD_HM_ED),
        KC_TRNS,     KC_P7,       KC_P8,       KC_P9,       KC_NO,       KC_NO,       KC_NO,       KC_NO,       KC_PGUP,
        KC_TRNS,     KC_P4,       KC_P5,       KC_P6,       KC_PPLS,     KC_PMNS,                  KC_TRNS,     KC_PGDN,
        KC_TRNS,     KC_P0,       KC_P1,       KC_P2,       KC_P3,       KC_PDOT,     KC_TRNS,     KC_TRNS,
                     KC_TRNS,                  KC_NO,       MO(_FN),     KC_NO,       KC_TRNS,     KC_TRNS,     KC_TRNS
    ),
/*    여기는 보존용
    [1] = LAYOUT(
        // left hand
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        // right hand
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,                  KC_TRNS,     KC_TRNS,
        KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,
                     KC_TRNS,                  KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS,     KC_TRNS
    )
*/

};
// clang-format on

// Dial 用
// 26.04.03 : 확실하진 않은데.. 4개가 모두 정의되어 있지 않으면 컴파일 오류가 난다.
// 그건 아닌데.. 아무튼 오류가 났었다. 이유는 모르겠네.
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    // [_BADV] = { ENCODER_CCW_CW(KC_LEFT, KC_RGHT), ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    // [_BADV] = {ENCODER_CCW_CW(A(KC_TAB), LSA(KC_TAB)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_BADV] = {ENCODER_CCW_CW(C(KC_PGUP), C(KC_PGDN)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_BAQT] = {ENCODER_CCW_CW(C(KC_PGUP), C(KC_PGDN)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    // [_BAQT] = {ENCODER_CCW_CW(KC_UP, KC_DOWN), ENCODER_CCW_CW(KC_BRID, KC_BRIU)},
    // [_FN] = {ENCODER_CCW_CW(KC_UP, KC_DOWN), ENCODER_CCW_CW(KC_BRID, KC_BRIU)},
    [_FN] = {ENCODER_CCW_CW(C(KC_PGUP), C(KC_PGDN)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    [_NPD] = {ENCODER_CCW_CW(C(KC_PGUP), C(KC_PGDN)), ENCODER_CCW_CW(KC_VOLD, KC_VOLU)},
    // [_NPD] = {ENCODER_CCW_CW(KC_UP, KC_DOWN), ENCODER_CCW_CW(KC_BRID, KC_BRIU)},
};
#endif

static bool process_tap_or_long_press_key(keyrecord_t *record, uint16_t long_press_keycode) {
  if (record->tap.count == 0) { // Key is being held.
    if (record->event.pressed) {
      tap_code16(long_press_keycode);
    }
    return false; // Skip default handling.
  }
  return true; // Continue default handling.
};

// Home Row Mode 를 위해.. (일단은 Dvorak 에서만 작동.)
//     --> 2026.04.07 현재 안쓰고 있다. 자꾸 오작동이 생긴다.
// Dvorak Layer 일 때만 작동하게끔 돼 있다. set_single_default_layer 때문인데..
// 이걸 한 함수에서 하려면 현재 레이어를 확인해야만 한다.
// 그게 좋을지, 아니면 Qwerty 용 함수를 따로 만들어서 구분해서 호출하는게 좋을지?
// 복잡한 것 보단 단순한게 좋겠지?
static bool process_hmr_user(keyrecord_t *record, uint16_t mod_keycode) {
  const uint8_t mod_state = get_mods();                                 // Shift/Ctrl/Altl/Super 가 눌렸는지 확인.
  const uint8_t default_layer = get_highest_layer(default_layer_state); // Default Layer 를 알아내는 방법.
  // 문제.. 동시에 2키 이상 누를 순 없다. 그러려면 진짜 Modifiers 와 결합해야한다.
  // 방법이 있겠지? 사실, 키 2개 동시에 누를 일은 잘 없긴 한데..
  // 그리고, 이거.. 확실히 tap 과 hold 구분에 문제가 있다.
  // tap 인데 hold 로 인식하고, 그 결과 원치않는 영문 자판 변경이 이뤄진다...
  // 한글 상태도 같이 확인해야 하려나..? 그럼 문제는 조금 줄어들긴 하겠는데..
  if (!record->tap.count) { // 키를 누르고 있을 때
    if (record->event.pressed) {
      switch (mod_keycode) {
      case KC_LCTL:
      case KC_LALT:
      case KC_LGUI:
        if (default_layer == _BADV) {
          set_single_default_layer(_BAQT);
        }
        break;
      }
      // set_single_default_layer(_BAQT);
      set_mods(MOD_BIT(mod_keycode) | mod_state);

    } else {                                       // key released..
      if (default_layer == _BAQT && !hangul_kor) { // Qwerty 이고, 한글이 아닐 때만 Dvorak 으로.
        set_single_default_layer(_BADV);           // 요게 문제가 되긴 할 거다. 현재 Layer 를 확인하지 않고 그냥 해버렸으니..
      }
      del_mods(MOD_BIT(mod_keycode));
      // clear_mods();
    }
    return false;
  }
  return true; // 원래 키를 내보낸다.
};

// Tap Dance 定義
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for PgUp, twice for PgDn
    [TD_PU_PD] = ACTION_TAP_DANCE_DOUBLE(KC_PGUP, KC_PGDN),
    // Once for Home, Twice for End (on _NPD layer)
    [TD_HM_ED] = ACTION_TAP_DANCE_DOUBLE(KC_HOME, KC_END),
};

// macro 用
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // Shift/Ctrl/Altl/Super 가 눌렸는지 확인.
  // uint8_t mod_state = get_mods();
  // // Default Layer 를 알아내는 방법.
  // uint8_t default_layer = get_highest_layer(default_layer_state);

  const uint8_t mod_state = get_mods(); // Shift/Ctrl/Altl/Super 가 눌렸는지 확인.
  static uint16_t registerd_key = KC_NO;
  switch (keycode) {
  // 여기서부터 내가 만든 것들.
  //
  // PgUp, PgDn 처리 : 그냥 누르면 PgUp, Alt+PgUp 은 PgDn. 궁여지책이긴 한데...
  // Tap Dance 를 쓰기로..
  /* case PG_UPDN:
    // const uint8_t mod_state = get_mods();                                 // Shift/Ctrl/Altl/Super 가 눌렸는지 확인.
    const uint8_t MOD_MASK_LALT = MOD_BIT(KC_LALT); // 왼쪽 쉬프트
    // static uint16_t registerd_key = KC_NO;

    if (record->event.pressed) {       // PG_UPDN 눌렸을 때.
      if (mod_state & MOD_MASK_LALT) { // LALT가 눌렸는지 확인
        registerd_key = KC_PGDN;
      } else {
        registerd_key = KC_PGUP;
      }
      del_mods(mod_state & MOD_MASK_LALT);
      register_code(registerd_key);
      set_mods(mod_state & MOD_MASK_LALT);
    } else {
      unregister_code(registerd_key);
    }
    return false; */

  //
  // Home Row Mode 用
  //    --> 2026.04.07 현재, 쓰진 않고 있음.
  case HT_CT:
    return process_hmr_user(record, KC_LCTL);
  case TT_SF:
    return process_hmr_user(record, KC_RSFT);
  case NT_AL:
    return process_hmr_user(record, KC_LALT);
  case ST_GU:
    return process_hmr_user(record, KC_LGUI);
  case UT_CT:
    return process_hmr_user(record, KC_LCTL);
  case ET_SF:
    return process_hmr_user(record, KC_RSFT);
  case OT_AL:
    return process_hmr_user(record, KC_LALT);
  case AT_GU:
    return process_hmr_user(record, KC_LGUI);
  case JT_CT:
    return process_hmr_user(record, KC_LCTL);
  case KT_SF:
    return process_hmr_user(record, KC_RSFT);
  case LT_AL:
    return process_hmr_user(record, KC_LALT);
  case SCT_GU:
    return process_hmr_user(record, KC_LGUI);
  case FT_CT:
    return process_hmr_user(record, KC_LCTL);
  case DT_SF:
    return process_hmr_user(record, KC_RSFT);
  case ST_AL:
    return process_hmr_user(record, KC_LALT);
  case AQT_GU:
    return process_hmr_user(record, KC_LGUI);
  //
  // https://getreuer.info/posts/keyboards/triggers/index.html#tap-vs.-long-press
  // 여기서 C(KC_HOME) 은, 길게 눌렀을 때 'Ctrl+KC_HOME' 을 뜻한다.
  case HM_CH:
    return process_tap_or_long_press_key(record, C(KC_HOME));
  case ED_CE:
    return process_tap_or_long_press_key(record, C(KC_END));

  // 연습 : 이거도 실패.
  // case HT_CT:
  //   return process_tap_or_long_press_key(record, LM(_BAQT, MOD_LCTL));

  // /* 여기는 Shift+Space 시작.. 즉, 韓英 전환.
  //       --> 더 정확히 말해서,
  //           한글(Qwerty 상태)
  //           영문(Dvorak 상태)
  // 다 잘 작동하는데, 딱 하나. Shift+SPC 를 계속 누르고 있을 때, 한/영 전환이 지속적으로 이뤄지지 않는다.
  // 굳이 이럴 필욘 없지만, 전엔 됐었는데, 안되니까 좀..?
  case KC_SPC: // SPACE 가 눌렸을 때!
    // const uint8_t mod_state = get_mods();                                 // Shift/Ctrl/Altl/Super 가 눌렸는지 확인.
    const uint8_t MOD_MASK_LSHIFT = MOD_BIT(KC_LSFT);                     // 왼쪽 쉬프트
    const uint8_t default_layer = get_highest_layer(default_layer_state); // Default Layer 를 알아내는 방법.
    os_variant_t host = detected_host_os();                               // 키보드 접속 OS 확인
    // static uint16_t registerd_key = KC_NO;
    // static uint16_t registerd_key = KC_SPC;                            // 이걸 이렇게 해놓으면 역시나 뭔가 오류가
    // 생긴다. KC_NO 로 해야 제대로 작동.

    // if 가 여기에 있으면, 가끔 공백이 그냥 입력되는 현상이 생긴다. 왜 그런걸까...?
    /* if (!(mod_state & MOD_MASK_LSHIFT)) { // 왼 Shift 가 안 눌렸다면, 그냥 space 로 넘겨..
      return true;
    } */

    if (record->event.pressed) { // SPC가 눌렸을 때.
      // press 안쪽으로 이동. SPC 가 눌리는 현상이 자꾸 발생하기에.
      // 그러나.. 안쪽에 있어도 마찬가지다. 제대로 하려면, 아래처럼 if pressed, else 文으로 해줘야 한다.
      /* if (!(mod_state & MOD_MASK_LSHIFT)) { // 왼 Shift 가 안 눌렸다면, 그냥 space 로 넘겨..
        // return true;
        // tap_code16(KC_SPC);
        register_code(KC_SPC); // 이거도 꽝.
        // register_code(registerd_key);
        unregister_code(KC_SPC);
        return false;
      } */
      if (mod_state & MOD_MASK_LSHIFT) { // LShift 가 눌렸는지 확인
        mozc_jap = false;                // Shift+SPC 는 한/영 전환에만 사용되므로, 일본어 상태는 무조건 false.
        if (default_layer == _BADV) {
          // Dvorak 자판일 때는 영문이므로, 한글(Qwerty)자판으로 전환
          set_single_default_layer(_BAQT);
          layer_move(_BAQT);
          registerd_key = KR_HAEN;
          hangul_kor = true;
        } else if (default_layer == _BAQT) { // Qwerty 라면, 한글자판이므로 영문/Dvorak 으로 전환.
          set_single_default_layer(_BADV);
          layer_move(_BADV);
          if (host == OS_LINUX) { // Linux 라면 영문자판 전용 전환키를, 그외는 한영키를.
            registerd_key = KC_INT4;
            hangul_kor = false;
          } else {
            registerd_key = KR_HAEN;
            hangul_kor = true;
          }
        }
        // 이걸 해줘야 Shift 눌린 상태가 해제되어, 순수 키만 내보낼 수 있게 된다.
        del_mods(mod_state & MOD_MASK_LSHIFT);
        register_code(registerd_key);
        set_mods(mod_state & MOD_MASK_LSHIFT); // 키를 내보내고 나서 다시 원상복귀.
      } else {                                 // LShift 가 눌리지 않으면 그냥 SPC
        registerd_key = KC_SPC;
        register_code(registerd_key);
      }
    } else { // space가 떼어졌을 때
      unregister_code(registerd_key);
    }
    return false;

  case TG_MOZC:                               // 일본어 전환
    if (record->event.pressed) {              // TG_MOZC가 눌렸을 때.
      os_variant_t host = detected_host_os(); // 키보드 접속 OS 확인

      if (host != OS_LINUX) {
        tap_code16(KC_EXLM);
        break;
      } // linux 가 아니면 無意味.
        // const uint8_t default_layer = get_highest_layer(default_layer_state);
        // // Default Layer 를 알아내는 방법.
      if (mozc_jap) { // 현재 일본어 상태라면 자판은 _BADV 라아먄 한다.
        // if (default_layer == _BAQT && hangul_kor) { // Qwerty 이고, 한글
        // 자판이라면
        if (hangul_kor) { // 한글 자판에서 전환했다면 다시 한글자판으로.
          set_single_default_layer(_BAQT);
          registerd_key = KR_HAEN;
          // mozc_jap = false;
          // registerd_key = KC_INT2;
        } else {
          set_single_default_layer(_BADV); // 한글이 꺼져있다면 영문이므로...
          registerd_key = KC_INT4;
        }
        mozc_jap = false;
        // registerd_key = KC_INT2;
      } else { // 일본어 상태가 아니라면,
        set_single_default_layer(_BADV);
        mozc_jap = true;
        registerd_key = KC_INT2;
      }
      tap_code16(registerd_key);
    }

    return false;

    /* case SS_HLO: // Layer 및 OS 인식 시험用
        os_variant_t host = detected_host_os();
        uint8_t current_layer = get_highest_layer(layer_state);
        // if (record->event.pressed && host == OS_LINUX) {
        if (record->event.pressed) {
            // send_string("Hello, world!\n");
            char buffer[16];
            // int my_variable = 123;
            sprintf(buffer, "Layer: %d\n", current_layer);
            // SEND_STRING(buffer);
            // sprintf(buffer, "The value is: %d", current_layer);
            // Send the buffer content to the host
            send_string(buffer);
            sprintf(buffer, "OS: %d\n", host);
            send_string(buffer);
            // SEND_STRING(host);
        }
        return false; */
  }
  return true;
};
