---
題目: "QMK PC 와 통신 : Raw HID"
떠오른 날: 2026-04-16
tags:
  - hardware
  - QMK
  - keyboard
  - Feker
  - IK75_Pro
  - Balice80
  - Debug
  - qmk_console
  - RAW_HID
---
# 이건 또 뭔 짓거리?
왜 여기까지 왔을까..
문제는 여기에서 비롯됐다. 뭐냐하면..
## Fcitx 입력기 전환 처리방식
Fcitx 에는 기본문자(영어..)와 기타 문자 중, 창이 바뀌었을 때도 그 상태를 유지하게 해줄 수 있는 기능이 있다. '설정 - 전역옵션 - Reset state on Focus In' 이 바로 그거다. 여기에서 '모두', '프로그램', '아니오' 중 하나를 선택할 수 있다. 모두는 무조건 새 창이 뜨면 기본값인 영어 배열로 바뀌게 되고, 프로그램은, 프로그램이 다를 경우(FF -> Wezterm 이라든가)에만 적용된다.
여기에 설정이 2개가 있는데..
- Reset state on Focus in : 새 창이 열리면 영어로 전환. ('프로그램'으로 설정)
- 입력 상태 공유(Share Input State) : 입력 상태를 공유할 대상. 역시 '프로그램'으로 설정한다.

이렇게 해주면 내가 원하는 상태가 된다. (사실 좀 불편하기도 한데..)
즉, ==창이 바뀌면 영어자판이 나오게끔 하고 싶다==.. 이게 내가 원하는 바다. 쓰다보니, 창이 바뀌어도 자판 상태가 유지되기 보단, 특히 다른 프로그램을 켰을 땐 영어 상태로 되는 게 키 입력 한번을 줄일 수 있겠다는 생각이 들었다.
## 하지만 문제가?
저렇게 하려면, Reset state on Focus In 을 '프로그램|모두'로 하면 된다. 창이 바뀔 때마다 영문자로 바뀐다. 다만, 내 환경에선 이게 문제가 된다.
QMK 에서 영문은 Dvorak 으로 정의했고, 한글 두벌식을 위해 Qwerty 를 쓰고 있는데, 만약 한글 상태에서 다른 창을 클릭하면, Fcitx 는 위 규칙에 따라 입력문자를 영문으로 전환한다. 허나.. QMK 자판은 그대로 Qwerty 이므로, 결국 영문이 Dvorak 이 아니라 Qwerty 가 되는 짜증나는 현상이 발생한다.
## 해결법은 없나?
이를 위해선, PC 에서 QMK 키보드의 層(Layer)을 바꿔줘야 한다.
즉, PC 에서 QMK 에게 명령을 내려야 하는 상황인데... 이를 위해 Raw HID 통신이 필요해진다.
아울러, 이게 '창 바뀜' 때마다 행해져야 하므로, Fcitx 에서도 이 때마다 명령을 내려줘야 한다.
즉.. 복잡해진다 이런 얘기. 다만, 꼭 Fcitx 를 쓰지 않고 Dbus 를 직접 건드리는 방법도 있다고 한다. 그냥 Fcitx 를 쓰기로 했다.
# 상황연구
## 사건(?) 순서
1. 창 초점이 바뀐다.
2. Fcitx 는 이를 감지하고 문자를 기본문자(영어)로 바꾼다.
3. 영어이므로 QMK 자판층도 Dvorak 으로 바뀌어야 한다. 따라서, 2번에서 행하는 Fcitx의 수행에, QMK 레이어도 변환해주는 명령을 덧붙인다.

말은 간단하군.
## 준비물
위 작업을 위해서 몇가지 준비물이 필요하다.
1. `fcitx5-lua` 꾸러미 : 그래야 위 '사건 순서' 3번을 실행할 수 있다.
2. udev rule : PC 에서 HID 장치에 접근할 수 있도록 새 규칙을 만들어줘야 한다.
3. `hidapitester` : PC 에서 HID 장치에 신호를 보내주는 프로그램. (수신은 모르겠네?)
4. QMK 코딩 : 위 상황을 받아줄 수 있게끔 적절히 수정해줘야 한다.

자.. 해보자.
# Fcitx 설정
## 전역설정
Fcitx 전역설정에서, ==Reset state on Focus in== 과, ==입력 상태 공유(Share Input State)== 을 모두 '프로그램'으로 설정한다.
## fcitx5-lua
`fcitx5-lua`를 설치하고, `~.local/share/fcitx5/lua/imeapi/extensions/to_dvorak.lua` 등을 만든다. 이 파일의 내용은 이런 식.
```lua
#!/usr/bin/env lua

local fcitx = require("fcitx")

function on_focus_in()
	os.execute("$HOME/bin/qmk_change_to_dvorak.sh")
end

fcitx.watchEvent(fcitx.EventType.FocusIn, "on_focus_in")
```

창 바뀜을 감지하고, 그 때마다 명령을 수행한다. 그 명령은 또 다른 실행스크립트 `qmk_change_to_dvorak.sh`에 들어있다.

> [!caution] 한가지 문제!
>그런데 한가지 문제가 있다.
>'프로그램'으로 해놓으면, 어떤 프로그램들은(Electron 계열/Firefox 등) 탭 전환만 해도 fcitx-lua 의 `on_focus_in` 함수가 실행된다. 따라서 원치않는 결과가 나오게 되는데...
>문제는, 탭전환을 했을 때 Fcitx 는 그대로 원래 자판(한글)을 유지하고 있지만, `on_focus_in`은 수행된다는데 있다.
>간단한 if 로 해결될까 했었으나... 잘 안되네.

Fcitx 는 일단 이걸로 끝. 
# hidapitester 설치
위에 있는 `qmk_change_to_dvorak.sh` 은 아래와 같다.
```sh
#!/usr/bin/env sh
hidapitester --vidpid feed:1109 --usagePage 0xFF60 --open --length 32 --send-output 1,2 --close
```
위 방법을 알아내느라 소비한 시간은... 반나절 이상? 아.. 힘들었네.

실행하면 이런 결과가 나온다.
```sh
$ hidapitester --vidpid feed:1109 --usagePage 0xFF60 --open --length 32 --send-output 1,2   --close
Opening device, vid/pid:0xFEED/0x1109, usagePage/usage: FF60/0
Device opened
Writing output report of 32-bytes...wrote 32 bytes:
 01 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Closing Device
```
위에서 말한 '반나절' 안에, 왜 length 가 32라야만 하는지, feed:1109 가 뭔지 등등을 알아낸 시간도 포함돼 있다.
## 왜 hidapitester ?
[Raw HID](https://docs.qmk.fm/features/rawhid)와 통신하는 방법은 몇가지가 있다. 쉽게 쓸 수 있는게 [Python](https://pypi.org/project/hid/)이긴 한데... 기타 다른 CLI 도구들도 있고, 그 중 하나가 hidapitester 다. 이건 [검색하다](https://github.com/qmk/qmk_firmware/issues/17245#issuecomment-1214205827) 찾아냈다. 또는, Python [pyhidapi](https://www.reddit.com/r/olkb/comments/x00jyo/cannot_send_or_receive_data_with_raw_hid_feature/)로도 결과를 얻을 순 있는데.. `hidapitester`가 간편하다. [AUR](https://aur.archlinux.org/packages/hidapitester-git)로 설치가능하다.

이 외에도, 구글 AI 가 알려준 [qmk-hid](https://github.com/luckasRanarison/qmk-hid/) 이라든가, [qmk_hid](https://github.com/FrameworkComputer/qmk_hid)라는 것도 있다. 둘 다 나한텐 혼란만 줬다. `qmk-hid`는 Cargo 로 간단히 설치할 수 있고, `qmk_hid` 는 AUR 에 있다.
# udev rule 설정
이 내용은 [QMK 문서](https://docs.qmk.fm/features/rawhid)엔 안나와있다. 구글 검색에서 알아냈다.
## vid, pid
PC 와 키보드가 통신하려면 PC 에서 키보드로 데이터를 보낼 수 있어야 한다. 즉, HID Device 에 접근 권한이 있어야 한다. 일반 사용자는 이게 불가능하므로, 가능하도록 만들어준다. 이를 위해선 두가지 정보가 필요하다. 모두 `lsusb`로 간단하게 알아낼 수 있다.
```sh
$ lsusb
...
Bus 001 Device 008: ID feed:1109 Bun BAlice80
...
```
Vendor ID 는 feed (영단어가 아니고 16진수다), Product ID 는 1109 다. 이 정보를 추가해준다.
## 90-qmk.rules (파일명은 적당히..)
`/etc/udev/rules.d/95-qmk-balice80.rules`등에 다음 내용을 넣는다.
```udev
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="feed", ATTRS{idProduct}=="1109", MODE="0666", TAG+="uaccess"
```
## 확인
`ls /dev/hidraw*`를 보고, 일반 사용자에게 쓰기 권한이 부여되어있는지 본다.

일단은 PC 쪽 준비 완료. hidapitester 명령문은 아래에서 다시.
# QMK 수정
다음 두 파일에서 작업을 해줘야 한다. `config.h`는 선택.
- rules.mk
- keymap.c
- config.h

이 파일들 모두, `keymaps/default` 디렉토리에 있다.
## [rules.mk](https://docs.qmk.fm/features/rawhid#usage)
```c
# PC 와 통신 설정
RAW_ENABLE = yes

# Debugging Enable
CONSOLE_ENABLE = yes
```

`CONSOLE_ENABLE` 은 안해줘도 되지만, 작동하는지 확인하려면 해주는게 좋겠다.
## (선택) [config.h](https://docs.qmk.fm/features/rawhid#basic-configuration)
여긴 안해도 된다. 이게 왜 필요한지는 저 아래에서 `hidapitester` 사용할 때 다시.
```c
#define RAW_EPSIZE 32
#define RAW_USAGE_PAGE 0xFF60
#define RAW_USAGE_ID 0x61
```
위 값들은 기본값이고, 설정 안해도 그냥 기본값으로 잘 작동한다. 굳이 건드릴 필요는 없다.
## keymap.c
다음 함수를 넣는다.
```c
// PC 와 통신 가능하게.
void raw_hid_receive(uint8_t *data, uint8_t length) {
  uprintf("Data[0] is %u\n", data[0]);
  if (data[0] == 0x01) {
    layer_move(_BADV);
    hangul_kor = false;
    print("I received some stuff\n");
  }
}
```
`print` 와 `uprintf` 는, `CONSOLE_ENABLE`이 돼 있을 때만 유효한 출력 명령이다. 이렇게 해주고, PC 터미널에서 `qmk console` 해놓으면 결과값이 출력된다.

`raw_hid_receive`는, PC 에서 데이터를 받았을 자동 실행되는 callback 함수다. 단, 이 데이터가 정확하게 송신되어야만 값을 받는다.
### 언제 송신?
위에서 말한 fcitx lua 를 사용해서, 창 초점이 변경되었을 때 키보드로 데이터를 보낸다. 키보드에선 신호를 받고, 그 값이 1(0x01)이면, 자판층을 `_BADV`로 바꾸고, `hangul_kor` 를 false 로 변경한다. 확인을 위해 print 문을 사용했다.

이게 끝. 구글에선 `#include "raw_hid.h"` 를 넣어야 한다고도 했으나, 불필요했다. 구글 말고, 다른 어떤 문서에서도 이 구문은 찾을 수가 없었다.

***
# 정리 시간..
자... 이제 수시간동안 모니터와 싸웠던 결과를 정리해보도록 하자.
내가 찾은 문서 및 AI(구글) 결과를 종합하면, vid=feed, pid=1109 만 넣으면 데이터가 전송돼야 한다.
그런데.. 그게 잘 안되더라 이런 얘기.
예를 들자면, 이게 잘 작동했어야 한다.
```sh
echo -ne "aoeu" | qmk-hid -v feed -p 1109
```
그런데, `qmk-hid`는 저기에 10진수를 넣어야 하더라. 다른 것들 다 16진수 넣는데 얘만 또 왜..
그래서 이런 식으로 해줘야 했고..
```sh
echo -ne "aoeu" | qmk-hid -v 65261 -p 4361
```
이건 작동은 했다. 헌데, `qmk-hid`는 송신 이후 수신 메시지까지 기다리고 있다. 난 송신만 하면 되는데..
이거 말고 `qmk_hid`는 뭐가 문젠지 잘 되질 않았다.
위에 언급한 Python 코드도 역시나 한참을 헤맨 후에 작동했다.
## 뭐가 문제였을까?
### usagePage 와 usage
HID 기기엔 vid/pid 말고, usagePage 와 usage 라는 또 다른 값이 있다. 이걸 정확히 넣어주지 않으면 제대로 작동하지 않는 듯 하다.
이 값이 [[#(선택) [config.h](https //docs.qmk.fm/features/rawhid basic-configuration)|config.h]]에서 말한 바로 그 값이다. 기본값은 FF60/0061.
확인할 땐 `hidapitester --list-detail`로 할 수 있다.
```sh
$ hidapitester --vidpid feed:1109 --list-detail
FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0x0001
  usage:         0x0006
  serial_number: 48FF7006498888491322226700000000 
  interface:     0 
  path: /dev/hidraw1

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0xFF60
  usage:         0x0061
  serial_number: 48FF7006498888491322226700000000 
  interface:     1 
  path: /dev/hidraw2

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0x0001
  usage:         0x0002
  serial_number: 48FF7006498888491322226700000000 
  interface:     2 
  path: /dev/hidraw4

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0x0001
  usage:         0x0080
  serial_number: 48FF7006498888491322226700000000 
  interface:     2 
  path: /dev/hidraw4

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0x000C
  usage:         0x0001
  serial_number: 48FF7006498888491322226700000000 
  interface:     2 
  path: /dev/hidraw4

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0x0001
  usage:         0x0006
  serial_number: 48FF7006498888491322226700000000 
  interface:     2 
  path: /dev/hidraw4

FEED/1109: Bun - BAlice80
  vendorId:      0xFEED
  productId:     0x1109
  usagePage:     0xFF31
  usage:         0x0074
  serial_number: 48FF7006498888491322226700000000 
  interface:     3 
  path: /dev/hidraw5
```

왜 이렇게 많이 나오는지 모르겠으나, 이 중 필요한 건 FF60/0061값을 갖는 hidraw2 뿐이다.
따라서, ==최종 명령==은 이렇게 줘야 한다.

```sh
$ hidapitester --vidpid feed:1109 --usagePage 0xFF60 --open --length 32 --send-output 1,2 --close
Opening device, vid/pid:0xFEED/0x1109, usagePage/usage: FF60/0
Device opened
Writing output report of 32-bytes...wrote 32 bytes:
 01 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Closing device
```

길이, length 도 32를 줘야지, 다른 값을 주면 QMK 키보드가 받질 못한다.
위와 같이 정확하게 전송하면, QMK 키보드에 수신에 성공하게 되고, 설정해놓은 CONSOLE_ENABLE 에 따라 지정한 문구가 출력된다. 물론, 코드(layer_move)도 제대로 작동했다.
`send-output 1,2`의 결과로, `01 02` 가 출력됐고, 나머지 30바이트는 00 으로 채워졌다.
`send-output` 으로는 숫자만 보낼 수 있는 듯? 어차피 나는 뭔가 값을 보내기만 하면 되니까... Python 으론 문자를 보낼 수 있었다. Python 코드는 Balice80 디렉토리에 넣어놨다.

# 최종 스크립트
`~/bin/qmk_change_to_dvorak.sh`에 위 명령을 넣는다.
# 정리!
1. Fcitx 설정 적절히.
2. fcitx-lua 에 `on_focus_in` 을 포함한 lua 스크립트 설정. 'hidapitester' 호출
3. QMK 소스 변경 / 컴파일 / 플래시

> [!attention] 문제는 역시..
> `on_focus_in` 이므로, 같은 프로그램인지 아닌지 구분을 하진 않는다. 따라서, 같은 프로그램내 탭 이동을 해도 포커스 변경이 이뤄진다... 이거 바꿀 순 없으려나??