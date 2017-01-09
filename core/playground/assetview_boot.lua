for k, v in pairs(cetech) do _G[k] = v end

require "playground/editor_input"
require 'core/fpscamera'

ASSET_CREATOR = {
    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    -- UNIT
    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    unit = function(self, asset_name, type)
        self.actual_asset_unit = Unit.spawn(self.world, asset_name)

        if Transform.has(self.world, self.actual_asset_unit) then
            local transform = Transform.get(self.world, self.actual_asset_unit)
            Transform.set_position(self.world, transform, Vec3f.make(0.0, 0.0, 0.0))
        end
    end,

    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    -- MATERIAL
    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    material = function(self, asset_name, type)
        Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 20.0))

        self.actual_asset_unit = Unit.spawn(self.world, "playground/cube")

        local mesh = Mesh.get(self.world, self.actual_asset_unit)
        Mesh.set_material(self.world, mesh, asset_name)
    end,

    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    -- TEXTURE
    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    texture = function(self, asset_name, type)
        Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 20.0))

        self.actual_asset_unit = Unit.spawn(self.world, "playground/cube")

        local mesh = Mesh.get(self.world, self.actual_asset_unit)
        local material = Mesh.get_material(self.world, mesh)
        Material.set_texture(material, "u_texColor", asset_name)
    end,

    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    -- LEVEL
    -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
    level = function(self, asset_name, type)
        self.level = Level.load_level(self.world, asset_name)
    end
}

AssetView = AssetView or {}

function AssetView:init()
    EditorInput:Init()

    self.viewport = 0 --Renderer.GetViewport("default")
    self.world = World.create()
    self.camera_unit = Unit.spawn(self.world, "playground/camera")

    self.camera_transform = Transform.get(self.world, self.camera_unit)

    Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 20.0))

    self.actual_asset_unit = nil
    self.level = nil
end

function AssetView:shutdown()
    World.destroy(self.world)
end

function AssetView:update(dt)
    if self.actual_asset_unit then
        if EditorInput.mouse.left then
            -- Transform
            local transform = Transform.get(self.world, self.actual_asset_unit) -- self.camera_transform
            -- local transform = self.camera_transform
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
    Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 20.0))

    self:destroy_actual_asset()
    if ASSET_CREATOR[type] ~= nil then
        ASSET_CREATOR[type](self, asset, type)
    end
end

function AssetView:destroy_actual_asset()
    if self.level then
        Level.destroy(self.world, self.level)
        self.level = nil

    elseif self.actual_asset_unit then
        Unit.destroy(self.world, self.actual_asset_unit)
        self.actual_asset_unit = nil
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
