-- evrp keyboard API example
-- Run: evrp -p examples/keyboard.lua (may require sudo for /dev/input/event*)
--
-- keyboard.press(code)  - press key
-- keyboard.release(code) - release key
-- keyboard.click(code) - press then release
-- evrp.dry_run = true - simulate without device (no keyboard needed)
--
-- Key constants: keyboard.KEY_A, keyboard.KEY_ENTER, etc.

-- dry_run = true: simulate only. dry_run = false: send keys (needs keyboard + sudo).
evrp.dry_run = false

print("evrp keyboard test")
print("keyboard.KEY_A =", keyboard.KEY_A)
print("keyboard.KEY_ENTER =", keyboard.KEY_ENTER)

-- Example: type "hi" (requires keyboard device)
keyboard.click(keyboard.KEY_H)
keyboard.click(keyboard.KEY_I)

-- Or press/release manually:
keyboard.press(keyboard.KEY_LEFTCTRL)
keyboard.click(keyboard.KEY_C)  -- Ctrl+C
keyboard.release(keyboard.KEY_LEFTCTRL)

print("Keyboard API done.")
