require "playground/editor_mouse"

AssetView = AssetView or {}

function AssetView:init()
    EditorMouse:Init()

    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)
    self.camera_transform = Transform.GetTransform(self.world, self.camera_unit)

    Transform.SetPosition(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, -10))

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
        transform = Transform.GetTransform(self.world, self.actual_asset_unit)
        local pos = Transform.GetPosition(self.world, transform)
        local rot = Transform.GetRotation(self.world, transform)

        if EditorMouse.left then
            rot.X = rot.X + EditorMouse.delta_x * -0.01;
            rot.Y = rot.Y + EditorMouse.delta_y * 0.01;
        end
        EditorMouse:ResetButtons()

        Transform.SetPosition(self.world, transform, pos)
        Transform.SetRotation(self.world,transform, rot)
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
