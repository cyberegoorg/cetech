Application
===========

**get_frame_id** (): int
> Return frame id.

**get_delta_time ()**: float
> Return delta time.

**quit** ()
> Quit application.


Package
=======

**create** (package_name) : stringid64
> Create package

**load** (package_id)
> Load package

**unload** (package_id)
> Unload pacakge

**is_loaded** (package_id)
> Is package loaded

**flush** (package_id)
> Wait for package


Keyboard
========

**button_index** (button_name) : int
> Return button index

**button_name** (button_index) : string
> Return button name

**button_state** (button_index) : bool
> Return button state 

**pressed** (button_index) : bool
> Is button pressed in this frame?

**released** (button_index) : bool
> Is button released in this frame?


Mouse
=====

**button_index** (button_name) : int
> Return button index

**button_name** (button_index) : string
> Return button name

**button_state** (button_index) : bool
> Return button state 

**pressed** (button_index) : bool
> Is button pressed in this frame?

**released** (button_index) : bool
> Is button released in this frame?

**axis** (button_index) : Vector2
> Return mouse position.
