-- Mouse API example
print("evrp mouse test")
print("mouse.BTN_LEFT =", mouse.BTN_LEFT)
print("mouse.BTN_RIGHT =", mouse.BTN_RIGHT)
print("mouse.BTN_MIDDLE =", mouse.BTN_MIDDLE)

-- Move cursor (relative)
mouse.move(50, 50)
mouse.move(-20, 10)

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
