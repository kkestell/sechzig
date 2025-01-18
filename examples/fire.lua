-- Fire animation with smooth color transitions
heat = {}

-- Initialize heat array
for i = 0, NUM_PIXELS-1 do
    heat[i] = 0
end

function tick(dt)
    -- Step 1. Cool down every cell a little
    for i = 0, NUM_PIXELS-1 do
        local cooling = random(0, math.floor((55 * 10) / NUM_PIXELS) + 2)
        heat[i] = qsub8(heat[i], cooling)
    end

    -- Step 2. Heat from each cell drifts 'up' (towards index 0)
    for i = NUM_PIXELS-1, 2, -1 do
        heat[i] = math.floor((heat[i-1] + heat[i-2] + heat[i-2]) / 3)
    end

    -- Step 3. Randomly ignite new sparks near the bottom
    if random(0, 255) < 60 then
        local y = random(0, 7)
        heat[y] = qadd8(heat[y], random(160, 255))
    end

    -- Step 4. Convert heat to LED colors
    for i = 0, NUM_PIXELS-1 do
        local t192 = scale8(heat[i], 191)
        local heatramp = (t192 & 0x3F) << 2  -- Using native Lua 5.4 bitwise operators
        
        if t192 > 128 then
            setPixel(i, 255, 255, heatramp)
        elseif t192 > 64 then
            setPixel(i, 255, heatramp, 0)
        else
            setPixel(i, heatramp, 0, 0)
        end
    end
end
