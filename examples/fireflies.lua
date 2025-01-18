local maxFireflies = 5
local spawnChance = 3        -- Percentage out of 100
local tailLength  = 2        -- Glow extends this many pixels on each side

-- Each entry: { position = int, brightness = int, phase = int }
local fireflies = {}

-- Initialize the firefly states
for i = 1, maxFireflies do
  fireflies[i] = {
    position   = 0,
    brightness = 0,
    phase      = 0
  }
end

-- Called once per frame
function tick(dt)
  -- Clear the ring before drawing new frame
  clear()

  -- For each firefly
  for i = 1, maxFireflies do
    local f = fireflies[i]

    -- If brightness == 0, we can attempt to spawn a new firefly
    if f.brightness == 0 then
      if random(0, 100) < spawnChance then
        f.position   = random(0, NUM_PIXELS)   -- pick a new random pixel
        f.brightness = 255                     -- firefly starts at full brightness
        f.phase      = 0
      end
    end

    -- If this firefly is active
    if f.brightness > 0 then
      -- Compute the sine-based brightness factor
      local angle = f.phase * math.pi / 128
      local current_brightness = f.brightness * (math.sin(angle) + 1) / 2

      -- Light up the firefly and its tail
      for j = -tailLength, tailLength do
        local pos = (f.position + j + NUM_PIXELS) % NUM_PIXELS

        -- Retrieve existing color at that pixel
        local r, g, b = getPixel(pos)

        -- Compute how bright this segment should be
        local tailFactor  = (tailLength - math.abs(j)) / tailLength
        if tailFactor < 0 then
          tailFactor = 0
        end

        -- Add a scaled portion of brightness to red and green
        local c = math.floor(current_brightness * tailFactor + 0.5)
        local new_r = qadd8(r, c)
        local new_g = qadd8(g, c)
        local new_b = 0 -- forcing blue to 0 to match the original Fireflies code

        setPixel(pos, new_r, new_g, new_b)
      end

      -- Advance firefly’s phase. Once phase surpasses 254, reset brightness to 0
      f.phase = f.phase + 0.5
      if f.phase >= 255 then
        f.brightness = 0
      end
    end
  end
end
