---
題目: QMK 사용 시 영문 Dvorak 자판과 한글 입력기 공존 문제. (Layer 이해)
떠오른 날: 2026-04-09T14:30:00
tags:
  - hardware
  - QMK
  - keyboard
  - Feker
  - IK75_Pro
  - Balice80
  - Dvorak/Qwerty
  - Dvorak
  - 한글입력기
  - QMK_Layers
---

# 序論

QMK 에서, 자체 기능으로 Dvorak 을 구현한 경우, 즉, Default Layer 가 Dvorak 이라면, 한글 자판(두벌식이든, 세벌식이든, 리눅스 Fcitx 에서든 MS 윈도우 입력기에서든)을 쓸 때 문제가 생긴다. 한글 입력기는 영문 Qwerty 가 기본인 모양인데.. 이게 어그러지면 한국어 입력이 아예 되질 않는다.
예를 들어, Qwerty 기준으로 ASDF 키, 즉, 두벌식 'ㅁㄴㅇㄹ' 을 누르면 '매뎌'라고 잘못 입력이 된다.

Fcitx 에선, 기준 자판을 바꿔줄 수가 있긴 한데, 이건 OS 에서 자판을 他자판으로 바꿨을 때 의미가 있는 기능이다. 사실, QMK 이전 모든 키보드는 Qwerty 기준(유럽 국가에서 판매되는 키보드는 좀 다르려나?)일테니, 다른 배열을 인식하지 못하는게 어찌보면 당연한 현상이긴 하다.

Fcitx 제작자에게 문의해봤는데, 안된다고 한다. Fcitx 만의 문제가 아니라 libhangul 에서도 뭔가 수정이 돼야 한다고.

그리하여 결론:

- QMK 에서 Dvorak 층(Layer)을 구현했다면, **한글을 위한 Qwerty 층**도 만들어줘야 한다.
- 그리고 이 Qwerty 층은, 굳이 활성화(Current Layer)할 필요는 없고, 단지 **Default Layer** 로만 지정하면 된다.

이게 뭔 소리인지는 아래에서 다시.

# 本論

그럼, 어떻게?
`TO(layer)` 로 전환은 의미가 없다. 설명에 보면, 'activates _layer_ and de-activates all other layers (except your default layer)' 라고 돼 있는데, except default layer 라고 명시돼 있으므로, 한글 입력기에서 제대로 처리하지 못하여 오류가 발생한다.
TO 와 같은 기능을 하는 함수 `layer_move(layer)` 도 비슷하리라 생각한다. 이 함수가 default layer 를 남기는지 아닌지는 모르겠는데.. 시험해볼 순 있겠으나, 귀찮아서 생략.

## 여기서 잠깐! Default Layer, Current Layer 란?

- Default Layer : 키보드에 전원이 연결되었을 때 기본으로 인식되는 자판 배열. 보통 0 번 층.
- Current Layer : QMK 에서 소프트웨어로 선택하여, **활성화된** 자판 배열.

즉, Default Layer 는 Dvorak 이지만, 현재 활성화된 자판배열은 숫자글쇠배열일 수도 있고, 멀티미디어 조작을 위한 또 다른 배열일 수도 있다.

헌데, 두가지 자판이 동시에 켜진 상태로 있을 수도 있나?
당연히 있을 수 있다. 그럴 때를 위하여 QMK 는 `KC_TRNS` 라는 키코드를 준비해놨다.
예를 들어.. (코드 일부분만..)

```c
[_BADV] = LAYOUT(
    KC_ESC,      KC_F1,       KC_F2,
    KC_GRV,      KC_1,        KC_2,
    KC_TAB,      KC_QUOT,     KC_COMM,
    KC_CAPS,     KC_A,        KC_O,
    KC_LSFT,     KC_SCLN,     KC_Q,
............
[_NPD] = LAYOUT(
    KC_TRNS,     KC_TRNS,     KC_TRNS,
    KC_TRNS,     KC_NO,       KC_NO,
    KC_TRNS,     KC_NO,       KC_COMM,
    KC_NO,       KC_PPLS,     KC_PMNS,
    KC_TRNS,     KC_NO,       KC_NO,
    KC_LCTL,     KC_LGUI,     KC_NO,
............
```

\_NPD 배열에선 KC_PPLS, KC_PMNS 등만 쓰고, 나머지 자리에 KC_TRNS 를 넣어주면, 그 키들은 기본자판배열에 있는 같은 위치 키값들을 갖게 된다. 이럴 경우, 활성화된 자판은 2개가 되는 셈이다.

위 상황에서 보자면,

- Default Layer : \_BADV
- Current Layer : \_NPD

## 한글을 쓰려면 꼭 Qwerty 자판이 활성화돼야만 하나??

이에 대한 답은, '**아님!!**'
활성화는 문제가 되지 않는다. 그냥 '**Default Layer**'로 지정만 하면 된다. 활성화되지 않아도 작동에 아무 문제가 없다.

> [!info] 한글 입력을 위해선 기본 자판배열이 Qwerty 라야만 한다.
> 활성화하진 않아도 무방.
> 다만, 그 반대, 다시 말해서 Qwerty 가 활성화된 상태에서 Dvorak 이 기본층이면??

현재 이렇게 자판 계층이 정의돼 있다.

```c
enum layer_names {
  _BADV,
  _BAQT,
  _NPD,
  _FN,
};
```

Debug 를 활성화하고, 다음 코드로 시험해봤다.

```c
#ifdef CONSOLE_ENABLE
      uint8_t curr_layer1 = get_highest_layer(layer_state); // 현재 Layer
      uint8_t default_layer1 = get_highest_layer(default_layer_state); // Default Layer
      uprintf("Default_Layer: %u\n", default_layer1);
      uprintf("Current_Layer: %u\n", curr_layer1);
      if (IS_LAYER_ON(_BADV)) {
        print("Dvorak is On\n");
      } else if (IS_LAYER_ON(_BAQT)) {
        print("Qwerty is On\n");
      }
      print("######################################################################\n");
#endif

```

이후, `qmk console` 로 접속한 뒤, 한/영 전환 및 다른 자판배열로 전환하며 시험해봤다.

```
# 영문 Dvorak 상황일 때
Bun:BAlice80:1: Default_Layer: 0
Bun:BAlice80:1: Current_Layer: 0
Bun:BAlice80:1: Dvorak is On
```

당연히 Current/Default 모두 0, 즉 \_BADV 이다.

이번엔 한글 두벌식으로 전환. (Fcitx)

```
Bun:BAlice80:1: Default_Layer: 1
Bun:BAlice80:1: Current_Layer: 0
Bun:BAlice80:1: Dvorak is On
```

Default Layer 는 1, 즉 \_BAQT 로 바뀌었지만, Current_Layer 는 여전히 0 이다.
이런 배경에는 다음 코드가 있다.

```c
if (default_layer == _BADV) {
  // Dvorak 자판일 때는 영문이므로, 한글(Qwerty)자판으로 전환
  set_single_default_layer(_BAQT);
  registerd_key = KR_HAEN;
  hangul_kor = true;
} else if (default_layer == _BAQT) { // Qwerty 라면, 한글자판이므로 영문/Dvorak 으로 전환.
  set_single_default_layer(_BADV);
  if (host == OS_LINUX) { // Linux 라면 영문자판 전용 전환키를, 그외는 한영키를.
    registerd_key = KC_INT4;
    hangul_kor = false;
  } else {
    registerd_key = KR_HAEN;
    hangul_kor = true;
  }
}
```

여기서 `layer_move(_BADV)` 등은 사용하지 않았고, 그냥 `set_single_default_layer`만 사용했다.
다시 말해서, 한영전환을 해도 활성 Layer 변환은 없다. 그렇지만 한글 입력엔 전혀 문제가 없다.
시험 삼아 layer_move 함수를 써서 활성화를 해봤는데, 역시 잘 작동했다.

그렇다면, 굳이 활성화시킬 필요는 없지 않겠나?

