for k, v in pairs(cetech) do _G[k] = v end

require "playground/editor_input"
require 'core/fpscamera'

AssetView = AssetView or {}

function AssetView:init()
    EditorInput:Init()

    self.viewport = 0 --Renderer.GetViewport("default")
    self.world = World.create()
    self.camera_unit = Unit.spawn(self.world, "camera")
    --self.camera = Camera.get(self.world, self.camera_unit)
    self.camera_transform = Transform.get(self.world, self.camera_unit)

    Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, -20.0))

    self.actual_asset_unit = nil
    self.level = nil

    --self:show_asset("camera", "unit")
    -- self:show_asset("box1", "unit")
end

function AssetView:shutdown()
    World.destroy(self.world)
end

function AssetView:update(dt)
    if self.actual_asset_unit then
        if EditorInput.mouse.left then
            local transform = Transform.get(self.world, self.actual_asset_unit)
            local rot = Transform.get_rotation(self.world, transform)
            local m_world = Transform.get_world_matrix(self.world, transform)
            local x_dir = m_world.x

            -- Rotation
            local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), EditorInput.mouse.dx * -0.1)
            local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, EditorInput.mouse.dy * 0.1)
            local rotation = rotation_around_world_up * rotation_around_camera_right
            Transform.set_rotation(self.world, transform, rot * rotation)
        end

        if EditorInput.mouse.right then
            local pos = Transform.get_position(self.world, self.camera_transform)

            pos.z = pos.z + EditorInput.mouse.dy * -0.9
            Transform.set_position(self.world, self.camera_transform, pos)
        end

        EditorInput:ResetButtons()
    end

    --World.update(self.world, dt)
end

function AssetView:render()
    Renderer.render_world(self.world, self.camera, self.viewport)
end

function AssetView:show_asset(asset, type)
    self:destroy_actual_asset()

    if type == 'unit' then
        self.actual_asset_unit = Unit.spawn(self.world, asset)

        if Transform.has(self.world, self.actual_asset_unit) then
            local transform = Transform.get(self.world, self.actual_asset_unit)
            Transform.set_position(self.world, transform, Vec3f.make(0.0, 0.0, 0.0))
        end
    end
end

function AssetView:destroy_actual_asset()
    if self.actual_asset_unit then
        Unit.destroy(self.world, self.actual_asset_unit)
    end
end


function init()
    AssetView:init()
end

function update(dt)
    AssetView:update(dt)
end

function shutdown()
    AssetView:shutdown()
end

function render()
    AssetView:render()
end
