#!/usr/bin/env lua

-- 2026.04.21
-- 이전 프로그램과 현재 프로그램이 다를 때만 작동하게끔.
-- 또, 이전 프로그램 입력기 상태를 기억했다가 그 입력기를 다시 켜주게끔 설정.

local fcitx = require("fcitx")

-- 프로그램별 마지막 입력기를 저장할 테이블
local program_im_history = {}
-- 상태를 유지하기 위한 변수 (스크립트 실행 동안 유지됨)
local last_program = "None"

local DEFAULT_IM = "keyboard-us"
local KOR_IM = "hangul"
local JAP_IM = "mozc"

function on_focus_out()
	local p_name = fcitx.currentProgram() or "Unknown"
	program_im_history[p_name] = fcitx.currentInputMethod()
	last_program = p_name
end

function on_focus_in()
	local current_program = fcitx.currentProgram() or "Unknown"
	if current_program ~= last_program then
		local saved_im = program_im_history[current_program] or DEFAULT_IM
		fcitx.setCurrentInputMethod(saved_im)
		local log_msg =
			string.format("[%s] Focus Changed: %s -> %s", os.date("%Y-%m-%d %H:%M:%S"), last_program, current_program)

		-- 입력기별 QMK 값 매핑 테이블
		local im_to_qmk_map = {
			[DEFAULT_IM] = 1,
			[KOR_IM] = 2,
			[JAP_IM] = 3,
		}

		local toQMK = im_to_qmk_map[saved_im] or 1

		-- 로그 파일에 기록 (백그라운드 실행)
		log_msg = log_msg .. "\n" .. saved_im
		local cmd_log = string.format('echo "%s" >> /tmp/fcitx-focus-test-comparing-prgs.txt &', log_msg)
		local cmd_qmk = string.format("%s/bin/qmk_change_to_dvorak.sh %s", os.getenv("HOME"), toQMK)
		os.execute(cmd_log)
		os.execute(cmd_qmk)
	end
end

fcitx.watchEvent(fcitx.EventType.FocusOut, "on_focus_out")
fcitx.watchEvent(fcitx.EventType.FocusIn, "on_focus_in")
