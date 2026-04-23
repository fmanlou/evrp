print("evrp mouse test")
print("mouse.BTN_LEFT =", mouse.BTN_LEFT)
print("mouse.BTN_RIGHT =", mouse.BTN_RIGHT)
print("mouse.BTN_MIDDLE =", mouse.BTN_MIDDLE)

mouse.move(50, 50)
mouse.move(-20, 10)

if mouse.is_cursor_available() then
  local x, y = mouse.get_position()
  if x then
    print("Current position:", x, y)
    mouse.move_to_screen(960, 540)
  end
else
  print("Cursor position unavailable (no X11 display)")
end

mouse.scroll_v(-1)
mouse.scroll_v(1)
mouse.scroll_h(1)

mouse.button_click(mouse.BTN_LEFT)

mouse.button_down(mouse.BTN_RIGHT)
mouse.button_up(mouse.BTN_RIGHT)

print("Mouse API done.")
