# cetech.Vec3f

3D vector.

!!! important

    Vec3f is temporary value that is valid only one frame

# Example

```lua
```

# Methods
* Basic
    * [make( x, y, z ) : Vec3f](#make-x-y-zVec3f)
    * [unit_x() : Vec3f](#unit_x-Vec3f)
    * [unit_y() : Vec3f](#unit_y-Vec3f)
    * [unit_z() : Vec3f](#unit_z-Vec3f)
    * [length(v) : number](#length-v-number)
    * [length_squared( v ) : number](#length_squared-v-number)
    * [normalized( v ) : Vec3f](#normalized-v-Vec3f)
    * [dot( v1, v2 ) : number](#dot-v1-v2-number)
    * [lerp( from, to, time ) : Vec3f](#lerp-from-to-time-Vec3f)

# Operators
* [+( v1, v2 ): Vec3f](#v1-v2-Vec3f)
* [-( v1, v2 ): Vec3f](#-v1-v2-Vec3f)
* [*( v, s ): Vec3f](#v-s-Vec3f)
* [/( v, s ): Vec3f](#v-s-Vec3f_1)
* [-( v ): Vec3f](#-v-Vec3f)

------------------------------------------------------------------------------------------------------------------------

## make( x, y, z ) : Vec3f

Create new temporary vector.

### Returns
* Vec3f

### Arguments
* `x` : **number** - X value
* `y` : **number** - Y value
* `z` : **number** - Z value

------------------------------------------------------------------------------------------------------------------------

## unit_x() : Vec3f

Create new vector (1.0, 0.0).

### Returns
* Vec3f

------------------------------------------------------------------------------------------------------------------------

## unit_y() : Vec3f

Create new vector (0.0, 1.0).

### Returns
* Vec3f

------------------------------------------------------------------------------------------------------------------------

## unit_z() : Vec3f

Create new vector (0.0, 0.0, 1.0).

### Returns
* Vec3f

------------------------------------------------------------------------------------------------------------------------

## length( v ) : number

Return vector length.

### Returns
* Vector length

### Arguments
* `v` : **Vec3f** - Vector

------------------------------------------------------------------------------------------------------------------------

## length_squared( v ) : number

Return vector length^2.

### Returns
* Vector length^2

### Arguments
* `v` : **Vec3f** - Vector


------------------------------------------------------------------------------------------------------------------------

## normalized( v ) : Vec3f

Return normalized vector.

### Returns
* Nomralized vector

### Arguments
* `v` : **Vec3f** - Vector

------------------------------------------------------------------------------------------------------------------------

## dot( v1, v2 ) : number

Return vector dot product.

### Returns
* Dot product

### Arguments
* `v1` : **Vec3f** - Vector
* `v2` : **Vec3f** - Vector

------------------------------------------------------------------------------------------------------------------------

## cross( v1, v2 ) : Vec3f

Return vector cross product.

### Returns
* Dot product

### Arguments
* `v1` : **Vec3f** - Vector
* `v2` : **Vec3f** - Vector

------------------------------------------------------------------------------------------------------------------------

## lerp( from, to, time ) : Vec3f

Vector linear interpolation.

### Returns
* Final vector

### Arguments
* `from` : **Vec3f**  - From
* `to`   : **Vec3f**  - To
* `time` : **number** - Time

------------------------------------------------------------------------------------------------------------------------

## +( v1, v2 ) : Vec3f

Vector addition

### Returns
* Final vector

### Arguments
* `v1` : **Vec3f**  - V1
* `v2` : **Vec3f**  - V2

------------------------------------------------------------------------------------------------------------------------

## -( v1, v2 ) : Vec3f

Vector substitution

### Returns
* Final vector

### Arguments
* `v1` : **Vec3f**  - V1
* `v2` : **Vec3f**  - V2

------------------------------------------------------------------------------------------------------------------------

## *( v, s ) : Vec3f

Multiple vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec3f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## /( v, s ) : Vec3f

Divide vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec3f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## -( v ) : Vec3f

Return inverse vector

### Returns
* Final vector

### Arguments
* `v` : **Vec3f**  - V1
