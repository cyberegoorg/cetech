# cetech.Mat44f

Matrix 4x4

!!! important

    Matrix 4x4 is temporary value that is valid only one frame

# Example

```lua
```

# Methods
* Basic
    * [make( x, y, z, w ) : Mat44f](#make-x-y-z-w-mat44f)
    * [perspective_fov( fov, aspect_ratio, near, far ) : Mat44f](#perspective_fov-fov-aspect_ratio-near-far-mat44f)
    * [identity() : Mat44f](#identity-mat44f)
    * [translate( t ) : Mat44f](#translate-t-mat44f)
    * [scale( s ) : Mat44f](#scale-s-mat44f)
    * [rotate_x( x ) : Mat44f](#rotate_x-x-mat44f)
    * [rotate_y( y ) : Mat44f](#rotate_y-y-mat44f)
    * [rotate_z( z ) : Mat44f](#rotate_z-z-mat44f)
    * [rotate_xy( x, y ) : Mat44f](#rotate_xy-x-y-mat44f)
    * [rotate_xyz( x, y, z ) : Mat44f](#rotate_xyz-x-y-z-mat44f)
    * [rotate_zyx( z, y, x ) : Mat44f](#rotate_zyx-z-y-x-mat44f)
    * [transpose( m ) : Mat44f](#transpose-m-mat44f)

# Operators
* [*( m1, m2 ): Mat44f](#m1-m2-mat44f)
* [-( m ): Mat44f](#-v-mat44f)

------------------------------------------------------------------------------------------------------------------------

## make( x, y, z, w ) : Mat44f

Create new temporary matrix.

### Returns
* Mat44f

### Arguments
* `x` : **Vec4f** - X axis
* `y` : **Vec4f** - Y axis
* `z` : **Vec4f** - Z axis
* `w` : **Vec4f** - W axis

------------------------------------------------------------------------------------------------------------------------

## perspective_fov( fov, aspect_ratio, near, far ) : Mat44f

Create new perspective matrix.

### Returns
* Mat44f

### Arguments
* `fov`         : **number** - FOV
* `aspec_ratio` : **number** - Aspect ratio
* `near`        : **number** - Near
* `far`         : **number** - Far

------------------------------------------------------------------------------------------------------------------------

## identity( ) : Mat44f

Create identity matrix.

### Returns
* Mat44f

------------------------------------------------------------------------------------------------------------------------

## translate( t ) : Mat44f

Create new translate matrix.

### Returns
* Mat44f

### Arguments
* `t` : **Vec3f** - Translation

------------------------------------------------------------------------------------------------------------------------

## scale( s ) : Mat44f

Create new scale matrix.

### Returns
* Mat44f

### Arguments
* `s` : **Vec3f** - Scale

------------------------------------------------------------------------------------------------------------------------

## rotate_x( x ) : Mat44f

Create new rotation matrix by x axis.

### Returns
* Mat44f

### Arguments
* `x` : **number** - X angle

------------------------------------------------------------------------------------------------------------------------

## rotate_y( y ) : Mat44f

Create new rotation matrix by y axis.

### Returns
* Mat44f

### Arguments
* `y` : **number** - Y angle

------------------------------------------------------------------------------------------------------------------------

## rotate_z( z ) : Mat44f

Create new rotation matrix by z axis.

### Returns
* Mat44f

### Arguments
* `z` : **number** - Z angle

------------------------------------------------------------------------------------------------------------------------

## rotate_xy( x, y ) : Mat44f

Create new rotation matrix by x and y axis.

### Returns
* Mat44f

### Arguments
* `x` : **number** - X angle
* `y` : **number** - Y angle

------------------------------------------------------------------------------------------------------------------------

## rotate_xyz( x, y, z ) : Mat44f

Create new rotation matrix by x, y and z axis.

### Returns
* Mat44f

### Arguments
* `x` : **number** - X angle
* `y` : **number** - Y angle
* `z` : **number** - Z angle

------------------------------------------------------------------------------------------------------------------------

## rotate_zyx( z, y, x ) : Mat44f

Create new rotation matrix by z, y and x axis.

### Returns
* Mat44f

### Arguments
* `z` : **number** - Z angle
* `y` : **number** - Y angle
* `x` : **number** - X angle

------------------------------------------------------------------------------------------------------------------------

## *( m1, m2 ) : Mat44f

Multiple two matrix

### Returns
* Final matrix

### Arguments
* `m1` : **Mat44f**  - M1
* `m2` : **Mat44f**  - M2

------------------------------------------------------------------------------------------------------------------------

## -( m ) : Mat44f

Return inverse matrix

### Returns
* Final matrix

### Arguments
* `m` : **Mat44f**  - Mat44f
