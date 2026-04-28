#!/usr/bin/env lua

local program_im_history = {}
-- 기본값을 유지할 프로그램 목록
local programs_im_default = { albert = 1, krunner = 1 }

-- 상태를 유지하기 위한 변수 (스크립트 실행 동안 유지됨)
local last_program = "None"

local DEFAULT_IM = "keyboard-us"
local KOR_IM = "hangul"
local JAP_IM = "mozc"

local p_name = "albert"
program_im_history[p_name] = 2
program_im_history["superproductivity"] = 1
program_im_history["smplayer"] = 1

-- 기본값을 유지할 프로그램 목록
local programs_im_default = { albert = 1, krunner = 1 }

local DEFAULT_IM = "keyboard-us"
local KOR_IM = "hangul"
local JAP_IM = "mozc"

local p_name = "firefox"
local p_name = "albert"
program_im_history[p_name] = 2
current_program = "google-chrome"
current_program = "albert"

local saved_im = nil

-- if programs_im_default[current_program] ~= nil then
-- 이 정도면 되겠다..
if programs_im_default[current_program] then
	-- saved_im = DEFAULT_IM
	saved_im = "기본입력기"
else
	saved_im = program_im_history[current_program] or DEFAULT_IM
end
s = string.format("Saved IM is %s, and Current Program is %s", saved_im, current_program)

print(s)

print("program_im_history 출력")
for k, v in pairs(program_im_history) do
	print(k, v)
end
