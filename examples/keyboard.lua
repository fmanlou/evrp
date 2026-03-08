-- evrp keyboard API example
-- Run: evrp -l examples/keyboard.lua (may require sudo for /dev/input/event*)
--
-- evrp.press(code)  - press key
-- evrp.release(code) - release key
-- evrp.click(code) - press then release
-- evrp.dry_run = true - simulate without device (no keyboard needed)
--
-- Key constants: evrp.KEY_A, evrp.KEY_ENTER, evrp.KEY_SPACE, etc.

-- dry_run = true: simulate only. dry_run = false: send keys (needs keyboard + sudo).
evrp.dry_run = false

print("evrp keyboard test")
print("evrp.KEY_A =", evrp.KEY_A)
print("evrp.KEY_ENTER =", evrp.KEY_ENTER)

-- Example: type "hi" (requires keyboard device)
evrp.click(evrp.KEY_H)
evrp.click(evrp.KEY_I)

-- Or press/release manually:
evrp.press(evrp.KEY_LEFTCTRL)
evrp.click(evrp.KEY_C)  -- Ctrl+C
evrp.release(evrp.KEY_LEFTCTRL)

print("Keyboard API done.")
