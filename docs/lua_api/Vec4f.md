# cetech.Vec4f

4D vector.

!!! important

    Vec4f is temporary value that is valid only one frame

# Example

```lua
```

# Methods
* Basic
    * [make( x, y, z ) : Vec4f](#make-x-y-zVec4f)
    * [unit_x() : Vec4f](#unit_x-Vec4f)
    * [unit_y() : Vec4f](#unit_y-Vec4f)
    * [unit_z() : Vec4f](#unit_z-Vec4f)
    * [length(v) : number](#length-v-number)
    * [length_squared( v ) : number](#length_squared-v-number)
    * [normalized( v ) : Vec4f](#normalized-v-Vec4f)
    * [dot( v1, v2 ) : number](#dot-v1-v2-number)
    * [lerp( from, to, time ) : Vec4f](#lerp-from-to-time-Vec4f)

# Operators
* [+( v1, v2 ): Vec4f](#v1-v2-Vec4f)
* [-( v1, v2 ): Vec4f](#-v1-v2-Vec4f)
* [*( v, s ): Vec4f](#v-s-Vec4f)
* [/( v, s ): Vec4f](#v-s-Vec4f_1)
* [-( v ): Vec4f](#-v-Vec4f)

------------------------------------------------------------------------------------------------------------------------

## make( x, y, z ) : Vec4f

Create new temporary vector.

### Returns
* Vec4f

### Arguments
* `x` : **number** - X value
* `y` : **number** - Y value
* `z` : **number** - Z value

------------------------------------------------------------------------------------------------------------------------

## unit_x() : Vec4f

Create new vector (1.0, 0.0).

### Returns
* Vec4f

------------------------------------------------------------------------------------------------------------------------

## unit_y() : Vec4f

Create new vector (0.0, 1.0).

### Returns
* Vec4f

------------------------------------------------------------------------------------------------------------------------

## unit_z() : Vec4f

Create new vector (0.0, 0.0, 1.0).

### Returns
* Vec4f

------------------------------------------------------------------------------------------------------------------------

## unit_w() : Vec4f

Create new vector (0.0, 0.0, 0.0, 1.0).

### Returns
* Vec4f

------------------------------------------------------------------------------------------------------------------------

## length( v ) : number

Return vector length.

### Returns
* Vector length

### Arguments
* `v` : **Vec4f** - Vector

------------------------------------------------------------------------------------------------------------------------

## length_squared( v ) : number

Return vector length^2.

### Returns
* Vector length^2

### Arguments
* `v` : **Vec4f** - Vector


------------------------------------------------------------------------------------------------------------------------

## normalized( v ) : Vec4f

Return normalized vector.

### Returns
* Nomralized vector

### Arguments
* `v` : **Vec4f** - Vector

------------------------------------------------------------------------------------------------------------------------

## dot( v1, v2 ) : number

Return vector dot product.

### Returns
* Dot product

### Arguments
* `v1` : **Vec4f** - Vector
* `v2` : **Vec4f** - Vector

------------------------------------------------------------------------------------------------------------------------

## lerp( from, to, time ) : Vec4f

Vector linear interpolation.

### Returns
* Final vector

### Arguments
* `from` : **Vec4f**  - From
* `to`   : **Vec4f**  - To
* `time` : **number** - Time

------------------------------------------------------------------------------------------------------------------------

## +( v1, v2 ) : Vec4f

Vector addition

### Returns
* Final vector

### Arguments
* `v1` : **Vec4f**  - V1
* `v2` : **Vec4f**  - V2

------------------------------------------------------------------------------------------------------------------------

## -( v1, v2 ) : Vec4f

Vector substitution

### Returns
* Final vector

### Arguments
* `v1` : **Vec4f**  - V1
* `v2` : **Vec4f**  - V2

------------------------------------------------------------------------------------------------------------------------

## *( v, s ) : Vec4f

Multiple vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec4f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## /( v, s ) : Vec4f

Divide vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec4f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## -( v ) : Vec4f

Return inverse vector

### Returns
* Final vector

### Arguments
* `v` : **Vec4f**  - V1
