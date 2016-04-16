require 'core/class'

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, camera_unit)
    self.world = world
    self.unit = camera_unit
    self.camera = Camera.GetCamera(world, camera_unit)
    self.transform = Transform.GetTransform(world, camera_unit)
end


function FPSCamera:update(dt, dx, dy, left, right, up, down)
    local pos = Transform.GetPosition(self.world, self.transform)
    local rot = Transform.GetRotation(self.world, self.transform)

    rot.X = rot.X + dx
    rot.Y = rot.Y + dy
    Transform.SetRotation(self.world, self.transform, rot)

    local m_world = Transform.GetWorldMatrix(self.world, self.transform)
    local z_dir = Vec3f.Normalize(Mat4f.Z(m_world))
    local x_dir = Vec3f.Normalize(Mat4f.X(m_world))
    local speed = 0.1 * dt

    if up then
        pos = pos - (z_dir * speed)
    end

    if down then
        pos = pos + (z_dir *  speed)
    end

    if left then
        pos = pos - (x_dir * speed)
    end

    if right then
        pos = pos + (x_dir * speed)
    end
    Transform.SetPosition(self.world, self.transform, pos)
end