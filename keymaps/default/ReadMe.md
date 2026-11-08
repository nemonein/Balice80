## 읽어봐..

다음은 계속 고려할 점들..
PgUp/Dn :
최적의 방법을 찾아보기.. 26.04.03 현재는 PgUp/Alt+PgUp 으로..
대충 살펴본 결과 ALt+PgUp 은 안쓰는 단축키같긴 한데..
제일 좋은 방법은 레이어겠고..
아니면 Dial 로???
Tap Dance?? 근데 Tap Dance 면 연속해서 눌러야할 땐 어찌..?

Dial 활용:
현재는 Alt+Tab, Shift+Alt+Tab 등을 이용하고 있는데.. 어떤게 좋을지.

---

2026-10-23
일단, 다른 기능은 건드리지 않았고, 키보드로 smplayer/mpv 를 옮겨다닐 수 있게 하는 기능을 추가하려 한다. 그를 위해 mpv 브랜치를 만든다.
--> 이건 다 해결하여, main 으로 합류.

2026-11-07
Include 경로 표시.
compile_commands.json 을, 이 디렉토리(QMK) 최상위에 배치.
그러면 nvim keymap.c 에서 경고가 많이 줄어들기도 하고,
`ENCODER_MAP_ENABLE` 관련 항목도 흐려지지 않고 제대로 표시된다.

```c
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
... 저 json 파일이 없으면 여기가 흐리게 표시된다.
```

만드는 법.

### QMK compilation database

```sh
qmk compile --compiledb -kb <keyboard> -km <keymap>
```

이러면, QMK Root 에 `compile_commands.json`이 생성된다.
작업대 쪽으로도 복사해주면 된다.
