-- Rainbow wave animation with smooth transitions
time = 0

function tick(dt)
    time = time + dt
    clear()
    
    -- Use pulsing brightness for added depth
    local brightness = map(math.sin(time * 1.5), -1, 1, 150, 255)
    setBrightness(math.floor(brightness))
    
    -- Create segments of the rainbow using gradients
    local segments = 6  -- Number of main color transitions
    local segment_size = math.floor(NUM_PIXELS / segments)
    local offset = time * 100  -- Moving speed
    
    for s = 0, segments-1 do
        local start_pos = (s * segment_size) % NUM_PIXELS
        local end_pos = ((s + 1) * segment_size - 1) % NUM_PIXELS
        
        -- Calculate hues for this segment
        local hue1 = (offset + s * 360 / segments) % 360
        local hue2 = (offset + (s + 1) * 360 / segments) % 360
        
        -- Convert hues to RGB for gradient
        local r1, g1, b1 = hsv2rgb(hue1, 1, 1)
        local r2, g2, b2 = hsv2rgb(hue2, 1, 1)
        
        -- Handle wrap-around case
        if end_pos < start_pos then
            gradient(start_pos, NUM_PIXELS-1, r1, g1, b1, r2, g2, b2)
            gradient(0, end_pos, r1, g1, b1, r2, g2, b2)
        else
            gradient(start_pos, end_pos, r1, g1, b1, r2, g2, b2)
        end
    end
end
