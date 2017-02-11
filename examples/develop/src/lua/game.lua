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

    --    self.unit = Unit.
    -- -- (self.world, "unit1");
    --    local t = Transform.get(self.world, self.unit)
    --    local p = Transform.get_scale(self.world, t)
    --    Log.debug("lua", "%f %f %f", p.x, p.y, p.z)

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
    --Unit.spawn(self.world, "unit11");

    self.debug = false
    self.capture = false
    self.switch_unit = false

    self.level = Level.load_level(self.world, "level1")
    self.unit = Level.unit_by_id(self.level, "55643423443313252");
    --Unit.destroy(self.world, self.unit)
end

function Game:shutdown()
    Log.info("boot.lua", "shutdown")
    World.destroy(self.world);
end

function rotator(world, unit, node_name, delta_rot)
    local node = SceneGraph.node_by_name(world, unit, node_name)
    local rot = SceneGraph.get_rotation(world, node)
    rot = rot * delta_rot
    SceneGraph.set_rotation(world, node, rot)
end

function transform_rotator(world, unit, delta_rot)
    local lt = Transform.get(world, unit)
    local rot = Transform.get_rotation(world, lt)
    rot = rot * delta_rot

    Transform.set_rotation(world, lt, rot)
end

TEXTURE_CYCLE = { "content/scene/m4a1/m4_diff", "content/uv_checker", "content/texture1" }
TEXTURE_CYCLE_IT = 1

L = 2
function Game:update(dt)
        local mesh = Mesh.get(self.world, self.unit)
        local material = Mesh.get_material(self.world, mesh)

--    L = L + dt * 0.1
--    if (L >= 1.0) then L = 0; end

    Material.set_vec4f(material, "u_vec4", Vec4f.make(L, L, L, 1.0))

    if Keyboard.button_pressed(Keyboard.button_index("t")) then
        Material.set_texture(material, "u_texColor", TEXTURE_CYCLE[(TEXTURE_CYCLE_IT % #TEXTURE_CYCLE) + 1])


        TEXTURE_CYCLE_IT = TEXTURE_CYCLE_IT + 1
    end

    local level_unit = Level.unit(self.level)
    --    transform_rotator(self.world, level_unit, Quatf.from_axis_angle(Vec3f.unit_y(), 0.02))
    rotator(self.world, Level.unit_by_id(self.level, "55643433135454252"), "n_cube", Quatf.from_axis_angle(Vec3f.unit_x(), 0.05))
    --transform_rotator(self.world, self.unit, Quatf.from_axis_angle(Vec3f.unit_z(), 0.08))

    if Keyboard.button_pressed(reload_btn) then
        ResourceManager.compile_all()
        ResourceManager.reload_all()
        Plugin.reload_all()
    end

    if Keyboard.button_pressed(quit_btn) then
        Application.quit()
    end

    if Keyboard.button_pressed(debug_btn) then
        self.debug = not self.debug;
    end
        Renderer.set_debug(self.debug)

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
        dx, dy = m_axis.x, m_axis.y
        if dx ~= 0 or dy ~= 0 then
            --  Log.debug("lua", "%f %f", dx, dy)
        end

        m_axis = Mouse.axis(Mouse.axis_index("absolute"))
        local x, y = m_axis.x, m_axis.y
        --Log.debug("lua", "abs: %f %f", x, y)
    end

    local up = Keyboard.button_state(Keyboard.button_index('w'))
    local down = Keyboard.button_state(Keyboard.button_index('s'))
    local left = Keyboard.button_state(Keyboard.button_index('a'))
    local right = Keyboard.button_state(Keyboard.button_index('d'))

    local updown = 0.0
    local leftdown = 0.0
    if up then updown = 20.0 end
    if down then updown = -20.0 end
    if left then leftdown = -20.0 end
    if right then leftdown = 20.0 end

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

        self.fps_camera:update(dt, right_a.x * 1.0, right_a.y * 1.0, left_a.y * 20, left_a.x * 20)
    end

    World.update(self.world, dt)
end

function Game:render()
    Renderer.render_world(self.world, self.camera, self.viewport)
end

function foo(value)
    return value
end
