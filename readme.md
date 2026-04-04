# BAlice80

Balice80 QMK Configuration.

# 리눅스(KDE)/fcitx5 用

- KDE가 아닌, 다른 DE 에서도 단축키 설정이 된다면 쓸 수 있다.
- MS Windows 에서도 되지만, 살짝 문제가 있을 수 있다.
- macOS 에선 아직 시험 전이지만, 특별한 설정이 필요할 듯?
- 원활한 한영전환을 위해선 KDE 설정이 필요하다.

# PgUp/PgDn 설정

우측 키보드에 세로로 키가 3개밖에 없기 때문에, Home/End, PgUp/PgDn 을 모두 구현할 수가 없다.
따라서, 사용 빈도가 높은 Home/End 키를 상/하단에 넣고 중간에 PgUp/Dn 을 넣었다.
단, PgUp/Dn 은 Tap Dance 로, 한번 누르면 PgUp, 두번 누르면 PgDn 을 수행하게 했다.

- PgUp 은 그대로, Alt+PgUp 을 PgDn 에 할당한 코드도 있다. (26.04.04 현재 未使用)

# Dvorak/Qwerty 설정

- 키보드 자체에 드보락(\_BADV) 자판을 구현했으므로, OS 에선 그냥 일반(Qwerty)로 설정한다.
- 다만, 하드웨어 드보락 자판에선 한글(Fcitx)을 구현할 수가 없으므로, 한글을 쓰려면 Qwerty 로 바꿔야 한다.
- 자판 전환키는 `Shift+Space` 로 설정했는데, 이게 원활히 작동하기 위해선 fcitx 에서 별도 설정을 해줘야 한다.

# Fcitx 설정 (韓英日 자판 사용 가능)

- 한영전환키는 `Shift+Space` 이지만, 실제 전환키는 '한영'키이다.
- 사실, 한영키도 실제 전환키는 아니고, KDE 에서 단축키 등록을 통해 한글입력으로 직접 이동하게끔 했다.

```
fcitx5-remote -s hangul             # '한글(KR_HAEN)' 키에 단축키 할당
fcitx5-remote -s keyboard-us        # 'KC_INT4' 키에 단축키 할당
fcitx5-remote -s mozc               # 'KC_INT2' 키에 단축키 할당
```

- 위와 같이 설정한 뒤, `fcitx5` 전역 설정에서 한영 전환키등은 모두 삭제한다. Toggle Input Method 는 아예 안쓰는 이상한 키 조합을 만들어놔도 된다.
