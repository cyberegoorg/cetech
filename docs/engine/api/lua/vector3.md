# Popis

Třísložkový vektor.

---

# Metody

## Vector3

```lua
function Vector3(x, y, z)
```

Vytvoří nový vektor `[x, y, z]`

---

## new

```lua
function Vector3.new(x, y, z)
```

Vytvoří nový vektor `[x, y, z]`

---

## zero

```lua
function Vector3.zero()
```

Vrátí nulový vektor `[0, 0, 0]`

---

## unit

```lua
function Vector3.unit()
```

Vrátí vektor `[1, 1, 1]`

---

## up

```lua
function Vector3.up()
```

Vrátí vektor `[0, 1, 0]`

---

## down

```lua
function Vector3.down()
```

Vrátí vektor `[0, -1, 0]`

---

## right

```lua
function Vector3.right()
```

Vrátí vektor `[-1, 0, 0]`


---

## left

```lua
function Vector3.left()
```

Vrátí vektor `[1, 0, 0]`

---

## forwards

```lua
function Vector3.forwards()
```

Vrátí vektor `[0, 0, -1]`

---

## backwards

```lua
function Vector3.backwards()
```

Vrátí vektor `[0, 0, 1]`

---

## normalized

```lua
function Vector3.normalized(vector)
```

Vráti normalizovaný `vector`.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector3 | Vektor

---

## is_normalized

```lua
function Vector3.is_normalized(vector)
```

Vráti `true` pokud je `vector` normalizovaný.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector3 | Vektor

---

## dot

```lua
function Vector3.dot(v1, v2)
```

Vráti skalární součin `(v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z)`.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector3 | Vektor
`v2`     | Vector3 | Vektor

---

## cross

```lua
function Vector3.cross(v1, v2)
```

Vráti cross

```math
[(v1.y * v2.z) - (v1.z * v2.y),
 (v1.z * v2.x) - (v1.x * v2.z),
 (v1.x * v2.y) - (v1.y * v2.x)]
```

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector3 | Vektor
`v2`     | Vector3 | Vektor

---

## len

```lua
function Vector3.len(vector)
```

Vráti délku vektoru.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector3 | Vektor

---

## len_inv

```lua
function Vector3.len_inv(vector)
```

Vráti 1 / len.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector3 | Vektor

---

## len_sq

```lua
function Vector3.len_sq(vector)
```

Vráti len ^ 2

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector3 | Vektor

---

## distance

```lua
function Vector3.distance(v1, v2)
```

Vráti vzdálenost dvou vektorů resp. bodů.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector3 | Vektor
`v2`     | Vector3 | Vektor

---

## distance_sq

```lua
function Vector3.distance_sq(v1, v2)
```

Vráti distance ^ 2.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector3 | Vektor
`v2`     | Vector3 | Vektor

---