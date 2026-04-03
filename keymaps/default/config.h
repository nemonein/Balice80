#pragma once

#define VIAL_KEYBOARD_UID {0xEF, 0x3F, 0xB8, 0xA5, 0xCD, 0xCA, 0x69, 0xE4}

// Tap-Hold Configuration Options
// https://docs.qmk.fm/tap_hold
// 200? 250?? 250 일땐 잘되는 것 같았고..
// 200 일 땐 한글 입력시 조금 문제가 있다.
// #define TAPPING_TERM 250
#define TAPPING_TERM 220
// #define TAPPING_TERM_PER_KEY

// https://docs.qmk.fm/tap_hold#permissive-hold
// #define PERMISSIVE_HOLD
// #define PERMISSIVE_HOLD_PER_KEY

// https://getreuer.info/posts/keyboards/achordion/index.html
#define CHORDAL_HOLD
#define PERMISSIVE_HOLD

// https://docs.qmk.fm/tap_hold#retro-tapping
// 해봤는데 안되는 듯..
// #define RETRO_TAPPING
