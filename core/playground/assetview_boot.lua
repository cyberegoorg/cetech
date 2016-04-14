AssetView = AssetView or {}

function AssetView:init()
    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)

    local transform = Transform.GetTransform(self.world, self.camera_unit)
    Transform.SetPosition(self.world, transform, Vec3f.make(0.0, 0.0, -10))


    self.actual_asset_unit = nil
    self.level = nil

    --self:show_asset("camera", "unit")
    --self:show_asset("box1", "unit")
end

function AssetView:shutdown()
    World.Destroy(self.world)
end

function AssetView:update(dt)
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
