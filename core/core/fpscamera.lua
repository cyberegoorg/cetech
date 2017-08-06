require 'core/class'

require 'cetech/transform'
require 'cetech/camera'

local Mat4f, Vec3f, Quatf, Log = cetech.Mat4f, cetech.Vec3f, cetech.Quatf, cetech.Log

FPSCamera = class(FPSCamera)

function FPSCamera:init(world, camera_entity, fly_mode)
    self.world = world
    self.entity = camera_entity
    self.camera = Camera.get(world, camera_entity)
    self.transform = Transform.get(world, camera_entity)

    self.fly_mode = fly_mode or false
end

function FPSCamera:setFlyMode(enable)
    self.fly_mode = enable
end

function FPSCamera:update(dt, dx, dy, updown, leftright)
    local pos = Transform.get_position(self.transform)
    local rot = Transform.get_rotation(self.transform)

    local m_world = Transform.get_world_matrix(self.transform)

    local x_dir = m_world.x
    local z_dir = m_world.z
       
    -- Position
    if not self.fly_mode then
        z_dir.y = 0.0
    end

    pos = pos + (z_dir * updown * dt)
    pos = pos + (x_dir * leftright * dt)

    -- Rotation
    local rotation_around_world_up = Quatf.from_axis_angle(Vec3f.unit_y(), -dx * dt * 100)
    local rotation_around_camera_right = Quatf.from_axis_angle(x_dir, dy * dt * 100)
    local rotation = rotation_around_world_up * rotation_around_camera_right

    Transform.set_position(self.transform, pos)
    Transform.set_rotation(self.transform, rot * rotation)
end
