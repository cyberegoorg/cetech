# Popis

Dvousložkoví vektor.

---

# Metody

## Vector2

```lua
function Vector2(x, y)
```

Vytvoří nový vektor `[x, y]`

---

## new

```lua
function Vector2.new(x, y)
```

Vytvoří nový vektor `[x, y]`

---

## zero

```lua
function Vector2.zero()
```

Vrátí nulový vektor `[0, 0]`

---

## unit

```lua
function Vector2.unit()
```

Vrátí vektor `[1, 1]`

---

## up

```lua
function Vector2.up()
```

Vrátí vektor `[0, 1]`

---

## down

```lua
function Vector2.down()
```

Vrátí vektor `[0, -1]`

---

## right

```lua
function Vector2.right()
```

Vrátí vektor `[-1, 0]`


---

## left

```lua
function Vector2.left()
```

Vrátí vektor `[1, 0]`

---

## normalized

```lua
function Vector2.normalized(vector)
```

Vráti normalizovaný `vector`.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector2 | Vektor

---

## is_normalized

```lua
function Vector2.is_normalized(vector)
```

Vráti `true` pokud je `vector` normalizovaný.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector2 | Vektor

---

## dot

```lua
function Vector2.dot(v1, v2)
```

Vráti skalární součin `(v1.x * v2.x) + (v1.y * v2.y)`.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector2 | Vektor
`v2`     | Vector2 | Vektor

---

## cross

```lua
function Vector2.cross(v1, v2)
```

Vráti cross `(v1.x * v2.y) - (v1.y * v2.x)`.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector2 | Vektor
`v2`     | Vector2 | Vektor

---

## len

```lua
function Vector2.len(vector)
```

Vráti délku vektoru.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector2 | Vektor

---

## len_inv

```lua
function Vector2.len_inv(vector)
```

Vráti 1 / len.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector2 | Vektor

---

## len_sq

```lua
function Vector2.len_sq(vector)
```

Vráti len ^ 2

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`vector` | Vector2 | Vektor

---

## distance

```lua
function Vector2.distance(v1, v2)
```

Vráti vzdálenost dvou vektorů resp. bodů.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector2 | Vektor
`v2`     | Vector2 | Vektor

---

## distance_sq

```lua
function Vector2.distance_sq(v1, v2)
```

Vráti distance ^ 2.

### Parametry

Parametr | Typ     | Popis
---------|---------|--------
`v1`     | Vector2 | Vektor
`v2`     | Vector2 | Vektor

---