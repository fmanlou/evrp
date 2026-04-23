evrp.dry_run = false

print("evrp keyboard test")
print("keyboard.KEY_A =", keyboard.KEY_A)
print("keyboard.KEY_ENTER =", keyboard.KEY_ENTER)

keyboard.click(keyboard.KEY_H)
keyboard.click(keyboard.KEY_I)

keyboard.press(keyboard.KEY_LEFTCTRL)
keyboard.click(keyboard.KEY_C)
keyboard.release(keyboard.KEY_LEFTCTRL)

print("Keyboard API done.")
