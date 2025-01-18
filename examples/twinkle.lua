local fadeAmount = 4         -- Amount by which pixels fade per frame
local twinkleChance = 2      -- Percentage chance of a twinkle per frame
local delayMs = 20           -- Frame delay in milliseconds

-- Called once per frame
function tick(dt)
  -- Gradually fade all pixels
  for i = 0, NUM_PIXELS - 1 do
    local r, g, b = getPixel(i)

    -- Subtract fade amount from each color component, clamping at 0
    r = (r > fadeAmount) and (r - fadeAmount) or 0
    g = (g > fadeAmount) and (g - fadeAmount) or 0
    b = (b > fadeAmount) and (b - fadeAmount) or 0

    setPixel(i, r, g, b)
  end

  -- Randomly add twinkles
  if random(0, 10) < twinkleChance then
    local idx = random(0, NUM_PIXELS - 1)
    local r = random(128, 255)
    local g = random(128, 255)
    local b = random(128, 255)
    setPixel(idx, r, g, b)
  end

  -- Introduce frame delay
  delay(delayMs / 1000) -- Convert milliseconds to seconds for Lua
end
