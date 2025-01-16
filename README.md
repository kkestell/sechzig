# Sechzig

Lua-scripted NeoPixel LED animations.

> Please keep still and only watchen astaunished the blinkenlights.

## How it Works

The ESP32 is connected to a 60-LED NeoPixel ring. The ESP32 runs a Lua interpreter that executes animation scripts to control the LEDs. It cycles through the animations, looping each animation for 10 seconds before moving on to the next one.

The ESP32 creates a public Wi-Fi access point that hosts a web server. Users can connect to the access point and create and modify their own Lua scripts to create custom animations. The scripts are stored on a LittleFS file system on the ESP32's internal flash memory.

The NeoPixel ring is epoxied to a piece of hardboard that is painted flat black and mounted in a poplar wood frame. The frame fits into the back of a cheap picture frame. The frame's glass is covered with a layer of tinted film for added effect.

## Writing Your Own Animations

First, connect to the "Sechzig" Wi-Fi network. Visit [http://192.168.4.1](http://192.168.4.1) in your web browser to access the web interface.

### Structure of an Animation Script

Each animation script must define a function called `tick(dt)`. This function is called repeatedly to update the animation on each frame.

* `dt` (float): The time in seconds since the last frame.

### Example Script

```lua
function tick(dt)
  gradient(0, NUM_PIXELS-1, 255, 0, 0, 0, 0, 255)
end
```

## Lua Helper Functions

### Pixel Control

#### `setPixel(pixel, r, g, b)`

Sets the color of a specific pixel.

* `pixel` (int): Index of the pixel (0 to NUM_PIXELS-1).
* `r`, `g`, `b` (int): Red, green, and blue values (0-255).

#### `getPixel(pixel)`

Gets the RGB values of a specific pixel.

* `pixel` (int): Index of the pixel (0 to NUM_PIXELS-1).
* **Returns**: `r`, `g`, `b` (int): Red, green, and blue values.

#### `fillRange(start, end, r, g, b)`

Fills a range of pixels with a specific color.

* `start`, `end` (int): Start and end indices of the range.
* `r`, `g`, `b` (int): Red, green, and blue values (0-255).

#### `clear()`

Clears all pixels (sets them to off).

#### `setBrightness(brightness)`

Sets the global brightness for all pixels.

* `brightness` (int): Brightness level (0-255).

#### `shiftPixels(amount, wrap)`

Shifts all pixels by a specified amount.

* `amount` (int): Number of pixels to shift (positive or negative).
* `wrap` (bool): Whether to wrap pixels around the edges.

### Color Manipulation

#### `blend(r1, g1, b1, r2, g2, b2, ratio)`

Blends two colors together.

* `r1, g1, b1` (int): First color.
* `r2, g2, b2` (int): Second color.
* `ratio` (float): Blend ratio (0.0 = full first color, 1.0 = full second color).
* **Returns**: `r`, `g`, `b` (int): Blended color.

#### `hsv2rgb(h, s, v)`

Converts HSV color to RGB.

* `h` (float): Hue (0-360).
* `s` (float): Saturation (0.0-1.0).
* `v` (float): Value (0.0-1.0).
* **Returns**: `r`, `g`, `b` (int): RGB values.

#### `gradient(start, end, r1, g1, b1, r2, g2, b2)`

Creates a gradient between two colors across a range of pixels.

* `start`, `end` (int): Start and end indices of the range.
* `r1, `g1, `b1` (int): Start color.
* `r2, `g2, `b2` (int): End color.

### Math and Utilities

#### `qsub8(i, j)`

Subtracts two integers, clamping the result to 0.

* `i`, `j` (int): Input integers.
* **Returns**: Clamped result (int).

#### `qadd8(i, j)`

Adds two integers, clamping the result to 255.

* `i`, `j` (int): Input integers.
* **Returns**: Clamped result (int).

#### `scale8(i, scale)`

Scales an integer by a factor (0-255).

* `i` (int): Input integer.
* `scale` (int): Scale factor.
* **Returns**: Scaled result (int).

#### `map(x, in_min, in_max, out_min, out_max)`

Maps a number from one range to another.

* `x` (float): Input value.
* `in_min`, `in_max` (float): Input range.
* `out_min`, `out_max` (float): Output range.
* **Returns**: Mapped value (float).

#### `random(min, max)`

Generates a random integer in a specified range.

* `min`, `max` (int): Range.
* **Returns**: Random integer.

#### `millis()`

Gets the number of milliseconds since the program started.

* **Returns**: Milliseconds (int).
