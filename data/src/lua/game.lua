for k, v in pairs(cetech) do _G[k] = v end

require 'core/fpscamera'

Game = Game or {}

local quit_btn = Keyboard.button_index 'q'
local debug_btn = Keyboard.button_index 'f9'
local reload_btn = Keyboard.button_index 'r'
local capture_btn = Keyboard.button_index 'f10'
local screenshot_btn = Keyboard.button_index 'f11'
local log_test_btn = Keyboard.button_index 'l'
local flymode_btn = Keyboard.button_index 'lshift'

function Game:init()
    Log.info("boot.lua", "Platform %s", Application.get_platform())

    Log.info("boot.lua", "info")
    Log.warning("boot.lua", "warn")
    Log.error("boot.lua", "error")
    Log.debug("boot.lua", "debug")

    --    self.viewport = Renderer.GetViewport("default")
    self.world = World.create()

    self.unit = Unit.spawn(self.world, "unit1");
    local t = Transform.get(self.world, self.unit)
    local p = Transform.get_scale(self.world, t)
    Log.debug("lua", "%f %f %f", p.x, p.y, p.z)

    --    self.level = World.LoadLevel(self.world, "level1");
    --    self.level = World.LoadLevel(self.world, "level1",
    --                 Vec3f.make(2, 5.0, 0.0),
    --                 Quatf.Identity, Vec3f.Unit);

    --    self.level_unit = World.LevelUnit(self.world, self.level)

    --    self.unit2 = World.UnitByName(self.world, self.level, "box2")
    --    self.unit = self.unit1

    self.camera_unit = Unit.spawn(self.world, "camera");
    self.camera = 0; --Camera.GetCamera(self.world, self.camera_unit);
    self.fps_camera = FPSCamera(self.world, self.camera_unit)
    --Unit.Spawn(self.world, "unit11");

    self.debug = false
    self.capture = false
    self.switch_unit = false
end

function Game:shutdown()
    Log.info("boot.lua", "shutdown")
    World.destroy(self.world);
end

function rotator(world, node, delta_rot)
    local rot = SceneGraph.GetRotation(world, node)
    rot = rot * delta_rot
    SceneGraph.SetRotation(world, node, rot)
end

TEXTURE_CYCLE = { "texture2", "content/texture1", "content/scene/m4a1/m4_diff" }
TEXTURE_CYCLE_IT = 1

function Game:update(dt)
    if Keyboard.button_pressed(Keyboard.button_index("t")) then
        local mesh = Mesh.get(self.world, self.unit)
        local material = Mesh.get_material(self.world, mesh)

        Material.set_texture(material, "u_texColor", TEXTURE_CYCLE[(TEXTURE_CYCLE_IT % #TEXTURE_CYCLE) + 1])
        TEXTURE_CYCLE_IT = TEXTURE_CYCLE_IT + 1
    end

    if Keyboard.button_pressed(reload_btn) then
        ResourceCompilator.compile_all()
        ResourceManager.reload_all()
    end

    if Keyboard.button_pressed(quit_btn) then
        Application.quit()
    end

    if Keyboard.button_pressed(debug_btn) then
        self.debug = not self.debug;
        Renderer.set_debug(self.debug)
    end

    if Keyboard.button_pressed(capture_btn) then
        self.capture = not self.capture;

        if self.capture then
            RenderSystem.BeginCapture()
        else
            RenderSystem.EndCapture()
        end
    end

    if Keyboard.button_pressed(screenshot_btn) then
        RenderSystem.SaveScreenShot("screenshot");
    end

    if Keyboard.button_pressed(Keyboard.button_index('f8')) then
        self.switch_unit = not self.switch_unit
        if self.switch_unit then
            self.unit = self.level_unit
        else
            self.unit = self.unit1
        end
    end

    if Keyboard.button_state(log_test_btn) then
        Log.info("game", "INFO TEST")
        Log.warning("game", "WARN TEST")
        Log.error("game", "ERROR TEST")
        Log.debug("game", "DEBUG TEST")
    end

    if Keyboard.button_state(flymode_btn) then
        self.fps_camera.fly_mode = true
    else
        self.fps_camera.fly_mode = false
    end

    local dx = 0
    local dy = 0
    if Mouse.button_state(Mouse.button_index("left")) then
        local m_axis = Mouse.axis(Mouse.axis_index("relative"))
        dx, dy = m_axis.x, -m_axis.y
        --Log.debug("lua", "%f %f", dx, dy)
    end
    
    local up = Keyboard.button_state(Keyboard.button_index('w'))
    local down = Keyboard.button_state(Keyboard.button_index('s'))
    local left = Keyboard.button_state(Keyboard.button_index('a'))
    local right = Keyboard.button_state(Keyboard.button_index('d'))

    local updown = 0.0
    local leftdown = 0.0
    if up then updown = 10.0 end
    if down then updown = -10.0 end
    if left then leftdown = -10.0 end
    if right then leftdown = 10.0 end

    self.fps_camera:update(dt, dx, dy, updown, leftdown)

    if Gamepad.is_active(0) then
        if Gamepad.button_pressed(0, Gamepad.button_index("left_shoulder")) then
            Gamepad.play_rumble(0, 5.0, 1000)
        end

        local right_a = Gamepad.axis(0, Gamepad.axis_index("right"))
        local left_a = Gamepad.axis(0, Gamepad.axis_index("left"))
        -- -- Log.info("lua", "{0}, {1}, {2}", left_a.X, left_a.Y, left_a.Z)

        if Gamepad.button_state(0, Gamepad.button_index("right_shoulder")) then
            self.fps_camera.fly_mode = true
        else
            self.fps_camera.fly_mode = false
        end

        self.fps_camera:update(dt, right_a.x * -0.1, right_a.y * -0.1, left_a.y * 10, left_a.x * 10)
    end

    --
    --- -    -- local unit1 = World.UnitByName(self.world, self.level, "box1")
    ---- -- local node = SceneGraph.GetNodeByName(self.world, unit1, "n_geom_0")
    ---- -- rotator(self.world, node, Quatf.FromAxisAngle(Vec3f.UnitY, 0.05))
    --
    -- unit1 = World.UnitByName(self.world, self.level, "box2")
    -- node = SceneGraph.GetNodeByName(self.world, unit1, "n_cube")
    --
    -- rotator(self.world, node, Quatf.FromAxisAngle(Vec3f.UnitZ, 0.1))
    --
    -- World.Update(self.world, dt)
    -- -- print("%f, %f", m_axis.x, m_axis.y)
    -- -- print(dt)
end

function Game:render()
    Renderer.render_world(self.world, self.camera, self.viewport)
end

function foo(value)
    return value
end
