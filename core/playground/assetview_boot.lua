require "playground/editor_input"
require 'core/fpscamera'

AssetView = AssetView or {}

function AssetView:init()
    EditorInput:Init()

    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)
    self.camera_transform = Transform.GetTransform(self.world, self.camera_unit)

    Transform.SetPosition(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 20))

    self.actual_asset_unit = nil
    self.level = nil

    --self:show_asset("camera", "unit")
    --self:show_asset("box1", "unit")
end

function AssetView:shutdown()
    World.Destroy(self.world)
end

function AssetView:update(dt)
    if self.actual_asset_unit then
        if EditorInput.mouse.left then
            local transform = Transform.GetTransform(self.world,  self.actual_asset_unit)
            local rot = Transform.GetRotation(self.world, transform)
            rot.X = rot.X + EditorInput.mouse.dx * 0.01
            rot.Y = rot.Y + EditorInput.mouse.dy * 0.01
            Transform.SetRotation(self.world, transform, rot)
        end

        if EditorInput.mouse.right then
            local pos = Transform.GetPosition(self.world,  self.camera_transform)
            pos.Z = pos.Z + EditorInput.mouse.dy * -0.9
            Transform.SetPosition(self.world, self.camera_transform, pos)
        end

        EditorInput:ResetButtons()
    end

    World.Update(self.world, dt)
end

function AssetView:render()
    Renderer.RenderWorld(self.world, self.camera)
end

function AssetView:show_asset(asset, type)
    self:destroy_actual_asset()

    if type == 'unit' then
        self.actual_asset_unit = Unit.Spawn(self.world, asset)

        if Transform.HasTransform(self.world, self.actual_asset_unit) then
            local transform = Transform.GetTransform(self.world, self.actual_asset_unit)
            Transform.SetPosition(self.world, transform, Vec3f.make(0.0, 0.0, 0.0))
        end
    end
end

function AssetView:destroy_actual_asset()
    if self.actual_asset_unit then
        Unit.Destroy(self.world, self.actual_asset_unit)
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
