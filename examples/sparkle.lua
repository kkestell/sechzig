-- Enhanced sparkle animation with smooth color transitions
time = 0
sparkles = {}

-- Initialize sparkle array
for i = 0, NUM_PIXELS-1 do
    sparkles[i] = 0
end

function tick(dt)
    time = time + dt
    clear()
    
    -- Create smooth base color gradient
    local segment_size = math.floor(NUM_PIXELS / 3)
    local baseOffset = time * 30
    
    for s = 0, 2 do
        local start_pos = s * segment_size
        local end_pos = ((s + 1) * segment_size - 1)
        
        -- Calculate colors for gradient
        local hue1 = (baseOffset + s * 120) % 360
        local hue2 = (baseOffset + (s + 1) * 120) % 360
        local r1, g1, b1 = hsv2rgb(hue1, 0.8, 0.3)  -- Muted colors
        local r2, g2, b2 = hsv2rgb(hue2, 0.8, 0.3)
        
        gradient(start_pos, end_pos, r1, g1, b1, r2, g2, b2)
    end
    
    -- Update and create sparkles
    for i = 0, NUM_PIXELS-1 do
        sparkles[i] = qsub8(sparkles[i], 25)  -- Fade out
    end
    
    -- Randomly add new sparkles
    if random(0, 100) < 20 then
        local pos = random(0, NUM_PIXELS-1)
        sparkles[pos] = 255
    end
    
    -- Add sparkle overlay using blend
    for i = 0, NUM_PIXELS-1 do
        if sparkles[i] > 0 then
            local r, g, b = getPixel(i)
            local r2, g2, b2 = blend(r, g, b, 255, 255, 255, sparkles[i] / 255)
            setPixel(i, r2, g2, b2)
        end
    end
    
    -- Add subtle brightness pulsing
    local brightness = map(math.sin(time * 2), -1, 1, 180, 255)
    setBrightness(math.floor(brightness))
end
