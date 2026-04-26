#pragma once

#include <string>
#include <vector>

#include "evrp/device/api/types.h"
#include "evrp/sdk/evdev.h"

bool isTouchpad(const char *devPath);
bool isTouchscreen(const char *devPath);
bool isMouse(const char *devPath);
bool isKeyboard(const char *devPath);

bool isMouseFromCapabilities(const Capabilities &caps);
Event makeEvent(unsigned short type, unsigned short code, int value);

std::string findFirstTouchpad();
std::string findFirstTouchscreen();
std::string findFirstMouse();
std::string findFirstKeyboard();

std::string findDevicePath(evrp::device::api::DeviceKind kind);

std::vector<std::string> findAllDevicePaths(
    evrp::device::api::DeviceKind kind);
