# cetech.Vec2f

2D vector.

!!! important

    Vec2f is temporary value that is valid only one frame

# Example

```lua
```

# Methods
* Basic
    * [make( x, y ) : Vec2f](#make-x-y-vec2f)
    * [unit_x() : Vec2f](#unit_x-vec2f)
    * [unit_y() : Vec2f](#unit_y-vec2f)
    * [length(v) : number](#length-v-number)
    * [length_squared( v ) : number](#length_squared-v-number)
    * [normalized( v ) : Vec2f](#normalized-v-Vec2f)
    * [dot( v1, v2 ) : number](#dot-v1-v2-number)
    * [lerp( from, to, time ) : Vec2f](#lerp-from-to-time-Vec2f)

# Operators
* [+( v1, v2 ): Vec2f](#v1-v2-Vec2f)
* [-( v1, v2 ): Vec2f](#-v1-v2-Vec2f)
* [*( v, s ): Vec2f](#v-s-Vec2f)
* [/( v, s ): Vec2f](#v-s-Vec2f_1)
* [-( v ): Vec2f](#-v-Vec2f)

------------------------------------------------------------------------------------------------------------------------

## make( x, y ) : Vec2f

Create new temporary vector.

### Returns
* Vec2f

### Arguments
* `x` : **number** - X value
* `y` : **number** - Y value

------------------------------------------------------------------------------------------------------------------------

## unit_x() : Vec2f

Create new vector (1.0, 0.0).

### Returns
* Vec2f

------------------------------------------------------------------------------------------------------------------------

## unit_y() : Vec2f

Create new vector (0.0, 1.0).

### Returns
* Vec2f

------------------------------------------------------------------------------------------------------------------------

## length( v ) : number

Return vector length.

### Returns
* Vector length

### Arguments
* `v` : **Vec2f** - Vector

------------------------------------------------------------------------------------------------------------------------

## length_squared( v ) : number

Return vector length^2.

### Returns
* Vector length^2

### Arguments
* `v` : **Vec2f** - Vector


------------------------------------------------------------------------------------------------------------------------

## normalized( v ) : Vec2f

Return normalized vector.

### Returns
* Nomralized vector

### Arguments
* `v` : **Vec2f** - Vector

------------------------------------------------------------------------------------------------------------------------

## dot( v1, v2 ) : number

Return vector dot product.

### Returns
* Dot product

### Arguments
* `v1` : **Vec2f** - Vector
* `v2` : **Vec2f** - Vector

------------------------------------------------------------------------------------------------------------------------

## lerp( from, to, time ) : Vec2f

Vector linear interpolation.

### Returns
* Final vector

### Arguments
* `from` : **Vec2f**  - From
* `to`   : **Vec2f**  - To
* `time` : **number** - Time

------------------------------------------------------------------------------------------------------------------------

## +( v1, v2 ) : Vec2f

Vector addition

### Returns
* Final vector

### Arguments
* `v1` : **Vec2f**  - V1
* `v2` : **Vec2f**  - V2

------------------------------------------------------------------------------------------------------------------------

## -( v1, v2 ) : Vec2f

Vector substitution

### Returns
* Final vector

### Arguments
* `v1` : **Vec2f**  - V1
* `v2` : **Vec2f**  - V2

------------------------------------------------------------------------------------------------------------------------

## *( v, s ) : Vec2f

Multiple vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec2f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## /( v, s ) : Vec2f

Divide vector by scalar

### Returns
* Final vector

### Arguments
* `v` : **Vec2f**  - V1
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## -( v ) : Vec2f

Return inverse vector

### Returns
* Final vector

### Arguments
* `v` : **Vec2f**  - V1
