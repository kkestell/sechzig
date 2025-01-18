#include "lua.hpp"
#include <Adafruit_NeoPixel.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#define LED_PIN 4 // NeoPixel data pin
#define NUM_PIXELS 60 // Number of NeoPixels in the ring
#define WIFI_SSID "Sechzig" // WiFi SSID
#define WIFI_PASS "" // WiFi password (leave empty for open network)
#define FRAME_TIME 16666 // Time in microseconds for each frame (60 FPS)
#define ANIMATION_DURATION 60000000 // Time in microseconds to play each animation

WebServer server(80);
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
lua_State *L = NULL;
SemaphoreHandle_t luaScriptMutex;
volatile bool scriptsUpdated = false;
String currentScript;
std::vector<String> scriptFiles;
size_t currentScriptIndex = 0;
uint64_t lastScriptChange;

static int l_qsub8(lua_State *L)
{
    int i = lua_tointeger(L, 1);
    int j = lua_tointeger(L, 2);
    lua_pushinteger(L, (i > j) ? i - j : 0);
    return 1;
}

static int l_qadd8(lua_State *L)
{
    int i = lua_tointeger(L, 1);
    int j = lua_tointeger(L, 2);
    int t = i + j;
    lua_pushinteger(L, (t > 255) ? 255 : t);
    return 1;
}

static int l_scale8(lua_State *L)
{
    uint8_t i = lua_tointeger(L, 1);
    uint8_t scale = lua_tointeger(L, 2);
    lua_pushinteger(L, (uint8_t)((uint16_t)i * (uint16_t)(scale + 1) >> 8));
    return 1;
}

static int l_random(lua_State *L)
{
    int min = lua_tointeger(L, 1);
    int max = lua_tointeger(L, 2);
    lua_pushinteger(L, random(min, max));
    return 1;
}

static int l_blend(lua_State *L)
{
    int r1 = lua_tointeger(L, 1);
    int g1 = lua_tointeger(L, 2);
    int b1 = lua_tointeger(L, 3);
    int r2 = lua_tointeger(L, 4);
    int g2 = lua_tointeger(L, 5);
    int b2 = lua_tointeger(L, 6);
    float ratio = lua_tonumber(L, 7);

    lua_pushinteger(L, r1 + (r2 - r1) * ratio);
    lua_pushinteger(L, g1 + (g2 - g1) * ratio);
    lua_pushinteger(L, b1 + (b2 - b1) * ratio);
    return 3;
}

static int l_millis(lua_State *L)
{
    lua_pushinteger(L, esp_timer_get_time() / 1000);
    return 1;
}

static int l_map(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs < 5)
    {
        Serial.println("Not enough arguments to map function");
        lua_pushnumber(L, 0);
        return 1;
    }

    float x = lua_tonumber(L, 1);
    float in_min = lua_tonumber(L, 2);
    float in_max = lua_tonumber(L, 3);
    float out_min = lua_tonumber(L, 4);
    float out_max = lua_tonumber(L, 5);

    float result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    lua_pushnumber(L, result);
    return 1;
}

static int l_fillRange(lua_State *L)
{
    int start = lua_tointeger(L, 1);
    int end = lua_tointeger(L, 2);
    int r = lua_tointeger(L, 3);
    int g = lua_tointeger(L, 4);
    int b = lua_tointeger(L, 5);

    for (int i = start; i <= end && i < NUM_PIXELS; i++)
    {
        if (i >= 0)
        {
            pixels.setPixelColor(i, r, g, b);
        }
    }
    return 0;
}

static int l_shiftPixels(lua_State *L)
{
    int amount = lua_tointeger(L, 1);
    bool wrap = lua_toboolean(L, 2);

    if (amount > 0)
    {
        for (int i = NUM_PIXELS - 1; i >= amount; i--)
        {
            uint32_t color = pixels.getPixelColor(i - amount);
            pixels.setPixelColor(i, color);
        }
        if (!wrap)
        {
            for (int i = 0; i < amount; i++)
            {
                pixels.setPixelColor(i, 0);
            }
        }
    }
    else if (amount < 0)
    {
        amount = -amount;
        for (int i = 0; i < NUM_PIXELS - amount; i++)
        {
            uint32_t color = pixels.getPixelColor(i + amount);
            pixels.setPixelColor(i, color);
        }
        if (!wrap)
        {
            for (int i = NUM_PIXELS - amount; i < NUM_PIXELS; i++)
            {
                pixels.setPixelColor(i, 0);
            }
        }
    }
    return 0;
}

static int l_setBrightness(lua_State *L)
{
    int brightness = lua_tointeger(L, 1);
    pixels.setBrightness(brightness);
    pixels.show();
    return 0;
}

static int l_gradient(lua_State *L)
{
    int start = lua_tointeger(L, 1);
    int end = lua_tointeger(L, 2);
    int r1 = lua_tointeger(L, 3);
    int g1 = lua_tointeger(L, 4);
    int b1 = lua_tointeger(L, 5);
    int r2 = lua_tointeger(L, 6);
    int g2 = lua_tointeger(L, 7);
    int b2 = lua_tointeger(L, 8);

    int length = end - start + 1;
    for (int i = 0; i < length; i++)
    {
        float ratio = (float)i / (length - 1);
        int r = r1 + (r2 - r1) * ratio;
        int g = g1 + (g2 - g1) * ratio;
        int b = b1 + (b2 - b1) * ratio;
        if (start + i >= 0 && start + i < NUM_PIXELS)
        {
            pixels.setPixelColor(start + i, r, g, b);
        }
    }
    return 0;
}

static int l_getPixel(lua_State *L)
{
    int pixel = lua_tointeger(L, 1);
    if (pixel >= 0 && pixel < NUM_PIXELS)
    {
        uint32_t color = pixels.getPixelColor(pixel);
        lua_pushinteger(L, (color >> 16) & 0xFF);
        lua_pushinteger(L, (color >> 8) & 0xFF);
        lua_pushinteger(L, color & 0xFF);
        return 3;
    }
    return 0;
}

static int l_debug(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);
    Serial.println(msg);
    return 0;
}

static int l_hsv2rgb(lua_State *L)
{
    float h = fmod(lua_tonumber(L, 1), 360);
    float s = lua_tonumber(L, 2);
    float v = lua_tonumber(L, 3);

    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;
    float r = 0, g = 0, b = 0;

    if (h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    lua_pushinteger(L, (r + m) * 255);
    lua_pushinteger(L, (g + m) * 255);
    lua_pushinteger(L, (b + m) * 255);
    return 3;
}

static int l_clear(lua_State *L)
{
    pixels.clear();
    return 0;
}

static int l_setPixel(lua_State *L)
{
    int pixel = lua_tointeger(L, 1);
    int r = lua_tointeger(L, 2);
    int g = lua_tointeger(L, 3);
    int b = lua_tointeger(L, 4);
    if (pixel >= 0 && pixel < NUM_PIXELS)
    {
        pixels.setPixelColor(pixel, r, g, b);
    }
    return 0;
}

void refreshScriptList()
{
    Serial.println("Refreshing script list...");
    scriptFiles.clear();
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        String fileName = file.name();
        if (fileName.endsWith(".lua"))
        {
            scriptFiles.push_back(fileName);
        }
        file = root.openNextFile();
    }
    root.close();
}

void handleRoot()
{
    String html = "<html><body><h2>Pixel Ring</h2>";
    html += "<form action='/create' method='post'>";
    html += "New script name: <input type='text' name='filename'>.lua ";
    html += "<input type='submit' value='Create'></form><hr>";

    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        String fileName = file.name();
        if (fileName.endsWith(".lua"))
        {
            html += "<div style='margin: 5px 0;'>";
            html += "<a href='/edit?file=" + fileName + "'>" + fileName + "</a> ";
            html += "<form action='/delete' method='post' style='display: inline;'>";
            html += "<input type='hidden' name='filename' value='" + fileName + "'>";
            html += "<input type='submit' value='Delete'></form></div>";
        }
        file = root.openNextFile();
    }
    root.close();
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleCreate()
{
    String fileName = server.arg("filename");
    if (fileName.length() > 0)
    {
        if (!fileName.endsWith(".lua"))
        {
            fileName += ".lua";
        }

        File file = LittleFS.open("/" + fileName, "w");
        if (file)
        {
            file.println("function tick(dt)");
            file.println("end");
            file.close();

            xSemaphoreTake(luaScriptMutex, portMAX_DELAY);
            scriptsUpdated = true;
            xSemaphoreGive(luaScriptMutex);
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleEdit()
{
    String fileName = server.arg("file");

    File file = LittleFS.open("/" + fileName, "r");
    if (!file)
    {
        server.send(404, "text/plain", "File not found");
        return;
    }

    String content = "";
    while (file.available())
    {
        content += (char)file.read();
    }
    file.close();

    String html = "<html><body>";
    html += "<h2>Editing " + fileName + "</h2>";
    html += "<form action='/save' method='post'>";
    html += "<textarea name='script' rows='20' cols='80'>";
    html += content;
    html += "</textarea><br>";
    html += "<input type='hidden' name='filename' value='" + fileName + "'>";
    html += "<input type='submit' value='Save'></form>";
    html += "<p><a href='/'>Back to script list</a></p>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleDelete()
{
    if (server.hasArg("filename"))
    {
        String fileName = server.arg("filename");
        LittleFS.remove("/" + fileName);
        xSemaphoreTake(luaScriptMutex, portMAX_DELAY);
        scriptsUpdated = true;
        xSemaphoreGive(luaScriptMutex);
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleSave()
{
    if (server.hasArg("script") && server.hasArg("filename"))
    {
        String script = server.arg("script");
        String fileName = server.arg("filename");

        File file = LittleFS.open("/" + fileName, "w");
        if (file)
        {
            file.print(script);
            file.close();
            xSemaphoreTake(luaScriptMutex, portMAX_DELAY);
            scriptsUpdated = true;
            xSemaphoreGive(luaScriptMutex);
        }
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

void animationTask(void *parameter)
{
    uint64_t lastFrameTime = esp_timer_get_time();
    float dt = 0;

    while (true)
    {
        uint64_t now = esp_timer_get_time();
        dt = (now - lastFrameTime) / 1000000.0f;

        if (scriptsUpdated)
        {
            xSemaphoreTake(luaScriptMutex, portMAX_DELAY);
            refreshScriptList();
            scriptsUpdated = false;
            xSemaphoreGive(luaScriptMutex);
        }

        if (scriptFiles.size() > 0 && (now - lastScriptChange >= ANIMATION_DURATION))
        {
            pixels.clear();
            pixels.show();

            currentScriptIndex = (currentScriptIndex + 1) % scriptFiles.size();
            lastScriptChange = now;

            File file = LittleFS.open("/" + scriptFiles[currentScriptIndex], "r");
            if (file)
            {
                currentScript = file.readString();
                file.close();

                if (L != NULL)
                {
                    lua_close(L);
                }

                L = luaL_newstate();
                if (L != NULL)
                {
                    luaL_openlibs(L);
                    lua_register(L, "setPixel", l_setPixel);
                    lua_register(L, "qsub8", l_qsub8);
                    lua_register(L, "qadd8", l_qadd8);
                    lua_register(L, "scale8", l_scale8);
                    lua_register(L, "random", l_random);
                    lua_register(L, "hsv2rgb", l_hsv2rgb);
                    lua_register(L, "clear", l_clear);
                    lua_register(L, "blend", l_blend);
                    lua_register(L, "millis", l_millis);
                    lua_register(L, "map", l_map);
                    lua_register(L, "fillRange", l_fillRange);
                    lua_register(L, "shiftPixels", l_shiftPixels);
                    lua_register(L, "setBrightness", l_setBrightness);
                    lua_register(L, "gradient", l_gradient);
                    lua_register(L, "debug", l_debug);
                    lua_register(L, "getPixel", l_getPixel);

                    lua_pushinteger(L, NUM_PIXELS);
                    lua_setglobal(L, "NUM_PIXELS");

                    if (luaL_dostring(L, currentScript.c_str()) != LUA_OK)
                    {
                        Serial.println("Lua error:");
                        Serial.println(lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                }
            }
        }

        if (L != NULL)
        {
            lua_getglobal(L, "tick");
            lua_pushnumber(L, dt);

            if (lua_pcall(L, 1, 0, 0) != LUA_OK)
            {
                Serial.println("Lua error in tick:");
                Serial.println(lua_tostring(L, -1));
                lua_pop(L, 1);
            }

            pixels.show();
        }

        lastFrameTime = now;
        uint64_t elapsed = esp_timer_get_time() - now;
        if (elapsed < FRAME_TIME)
        {
            delayMicroseconds(FRAME_TIME - elapsed);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS Mount Failed");
        return;
    }

    pixels.begin();
    pixels.setBrightness(50);
    pixels.clear();
    pixels.show();

    luaScriptMutex = xSemaphoreCreateMutex();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/create", HTTP_POST, handleCreate);
    server.on("/edit", HTTP_GET, handleEdit);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/delete", HTTP_POST, handleDelete);
    server.begin();

    refreshScriptList();

    lastScriptChange = esp_timer_get_time() - ANIMATION_DURATION;

    xTaskCreatePinnedToCore(animationTask, "Animation", 10000, NULL, 1, NULL, 1);

    if (scriptFiles.size() == 0)
    {
        File file = LittleFS.open("/blinkenlights.lua", "w");
        if (file)
        {
            file.print(R"(
-- Blinkenlights animation
local lastUpdate = 0
pixels = {}
for i = 0, NUM_PIXELS-1 do
    pixels[i] = false
end

function tick(dt)
    if millis() - lastUpdate < 500 then
        return
    end

    lastUpdate = millis()

    for i = 0, NUM_PIXELS-1 do
        setPixel(i, 0, 0, 0)
    end

    for i = 0, NUM_PIXELS-1 do
        pixels[i] = false
    end

    local lit_count = 0
    while lit_count < 20 do
        local pos = random(0, NUM_PIXELS-1)
        if not pixels[pos] then
            setPixel(pos, 255, 0, 0)
            pixels[pos] = true
            lit_count = lit_count + 1
        end
    end
end
)");
            file.close();
            refreshScriptList();
        }
    }
}

void loop()
{
    server.handleClient();
}
