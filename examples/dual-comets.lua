-- Dual comets with color variation and smooth tails
time = 0

function tick(dt)
    time = time + dt
    clear()
    
    -- Calculate comet positions (two comets moving in opposite directions)
    local pos1 = math.floor((time * 30) % NUM_PIXELS)
    local pos2 = math.floor(NUM_PIXELS - ((time * 20) % NUM_PIXELS))
    
    -- Get colors for comet heads
    local r1, g1, b1 = hsv2rgb(time * 100 % 360, 1, 1)          -- Moving hue for first comet
    local r2, g2, b2 = hsv2rgb((time * 100 + 180) % 360, 1, 1)  -- Opposite hue for second comet
    
    -- Create gradient trails behind each comet
    local trail_length = 10
    
    -- First comet trail (going forward)
    local start1 = (pos1 - trail_length) % NUM_PIXELS
    if start1 <= pos1 then
        gradient(start1, pos1, 0, 0, 0, r1, g1, b1)
    else
        -- Handle wrap-around
        gradient(0, pos1, 0, 0, 0, r1, g1, b1)
        gradient(start1, NUM_PIXELS-1, 0, 0, 0, r1, g1, b1)
    end
    
    -- Second comet trail (going backward)
    local start2 = (pos2 + trail_length) % NUM_PIXELS
    if pos2 <= start2 then
        gradient(pos2, start2, r2, g2, b2, 0, 0, 0)
    else
        -- Handle wrap-around
        gradient(pos2, NUM_PIXELS-1, r2, g2, b2, 0, 0, 0)
        gradient(0, start2, r2, g2, b2, 0, 0, 0)
    end
    
    -- When comets cross paths, blend their colors
    local blend_distance = 5
    for i = 0, NUM_PIXELS-1 do
        local dist1 = math.min(math.abs(i - pos1), math.abs(i - pos1 + NUM_PIXELS), math.abs(i - pos1 - NUM_PIXELS))
        local dist2 = math.min(math.abs(i - pos2), math.abs(i - pos2 + NUM_PIXELS), math.abs(i - pos2 - NUM_PIXELS))
        
        if dist1 < blend_distance and dist2 < blend_distance then
            -- Both comets are influencing this pixel
            local r_old, g_old, b_old = getPixel(i)
            local blend_ratio = math.min(1.0, (dist1 + dist2) / (2 * blend_distance))
            local r_new, g_new, b_new = blend(r1, g1, b1, r2, g2, b2, blend_ratio)
            setPixel(i, r_new, g_new, b_new)
        end
    end
end
