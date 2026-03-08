-- Mouse API example
print("evrp mouse test")
print("mouse.BTN_LEFT =", mouse.BTN_LEFT)
print("mouse.BTN_RIGHT =", mouse.BTN_RIGHT)
print("mouse.BTN_MIDDLE =", mouse.BTN_MIDDLE)

-- Move cursor (relative)
mouse.move(50, 50)
mouse.move(-20, 10)

-- Get current position and move to target (requires X11/XWayland)
if mouse.is_cursor_available() then
  local x, y = mouse.get_position()
  if x then
    print("Current position:", x, y)
    mouse.move_to_screen(960, 540)  -- move to screen center (e.g. 1920x1080)
  end
else
  print("Cursor position unavailable (no X11 display)")
end

-- Scroll
mouse.scroll_v(-1)   -- scroll up
mouse.scroll_v(1)    -- scroll down
mouse.scroll_h(1)    -- scroll right

-- Click left button
mouse.button_click(mouse.BTN_LEFT)

-- Right click (down then up)
mouse.button_down(mouse.BTN_RIGHT)
mouse.button_up(mouse.BTN_RIGHT)

print("Mouse API done.")
