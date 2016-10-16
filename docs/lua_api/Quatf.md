# cetech.Quatf

Quaternion

!!! important

    Quatf is temporary value that is valid only one frame

# Example

```lua
```

# Methods
* Constructor
    * [from_axis_angle( axis, angle ) : Quatf](#from_axis_angle-axis-angle-quatf)
    * [from_euler( heading, attitude, bank ) : Quatf](#from_euler-heading-attitude-bank-quatf)
* Basic
    * [to_mat44f( q ) : Mat44f](#to_mat44f-q-mat44f)
    * [to_euler_angle( q ) : Vec3f](#to_euler_angle-q-vec3f)
    * [length( q ) : number](#length-q-number)
    * [length_squared( q ) : number](#length_squared-q-number)
    * [normalized( q ) : Quatf](#normalized-q-quatf)
    
# Operators
* [+( q1, q2 ): Quatf](#q1-q2-Quatf)
* [-( q1, q2 ): Quatf](#-q1-q2-Quatf)
* [*( q, q ): Quatf](#q-q-Quatf)
* [/( q, s ): Quatf](#q-s-Quatf_1)
* [-( q ): Quatf](#-q-Quatf)

------------------------------------------------------------------------------------------------------------------------

## from_axis_angle( axis, angle ) : Quatf

Create new temporary quaternion from axis and angle.

### Returns
* Quatf

### Arguments
* `axis`  : **Vec3f**   - Axis
* `angle` : **number** - Angle

------------------------------------------------------------------------------------------------------------------------

## from_euler( heading, attitude, bank ) : Quatf

Create new temporary quaternion from euler angles.

### Returns
* Quatf

### Arguments
* `heading`  : **number** - Heading angle
* `attitude` : **number** - Attitude angle
* `bank`     : **number** - Bank angle

------------------------------------------------------------------------------------------------------------------------

## to_mat44f( q ) : Mat44f

Convert quaternion to Mat44f.

### Returns
* Mat44f

### Arguments
* `q` : **Quatf** - Quaternion

------------------------------------------------------------------------------------------------------------------------

## to_euler_angle( q ) : Vec3f

Convert quaternion to euler angles.

### Returns
* Vec3f

### Arguments
* `q` : **Quatf** - Quaternion

------------------------------------------------------------------------------------------------------------------------

## length( q ) : number

Return  length.

### Returns
* Vector length

### Arguments
* `q` : **Quatf** - Quaternon

------------------------------------------------------------------------------------------------------------------------

## length_squared( q ) : number

Return length^2.

### Returns
* Length^2

### Arguments
* `q` : **Quatf** - Quaternon


------------------------------------------------------------------------------------------------------------------------

## normalized( q ) : Quatf

Return normalized vector.

### Returns
* Nomralized vector

### Arguments
* `q` : **Quatf** - Quaternon

------------------------------------------------------------------------------------------------------------------------

## +( q1, q2 ) : Quatf

Quaternion addition

### Returns
* Final quaternion

### Arguments
* `q1` : **Quatf**  - Q1
* `q2` : **Quatf**  - Q2

------------------------------------------------------------------------------------------------------------------------

## -( q1, q2 ) : Quatf

Quaternion substitution

### Returns
* Final quaternion

### Arguments
* `q1` : **Quatf**  - Q1
* `q2` : **Quatf**  - Q2

------------------------------------------------------------------------------------------------------------------------

## *( q1, q1 ) : Quatf

Multiple quaternions

### Returns
* Final quaternion

### Arguments
* `q1` : **Quatf** - Q1
* `q2` : **Quatf** - Q2

------------------------------------------------------------------------------------------------------------------------

## /( q, s ) : Quatf

Divide quaternon by scalar

### Returns
* Final quaternion

### Arguments
* `q` : **Quatf**  - Quaternion
* `s` : **number** - Scalar

------------------------------------------------------------------------------------------------------------------------

## -( q ) : Quatf

Return inverse quaternion

### Returns
* Final quaternion

### Arguments
* `q` : **Quatf**  - Quaternion
