# Playground

Vývojové prostředí které se snáží o snadnou **modifikovatelnost** aby mohlo co nejlépe vyhovovát specifikacím projektu
pokud to bude potřeba. Proto je zvolena kombinace jazyků **Python**, **LUA**, **JS**, **HTML**.

---

# Proč tolik jazyků?

* **Python + QT** jako základ. Výborná podpora napříč OS.
* **LUA** pro komunikaci s enginem. To umožnuje mít kód nástrojů oddělený od enginu (nástroj pouze pošle script a ten
  nastaví požadované chování a pak stačí komunikovat přes `console_server`)
* **HTML + JS** pro vizualizace a různé widgety. Snadné použití a mocná vizualizace (a taky můžete mít puštěný
editor, upravovat kód a pak jenom zmáčnout RELOAD, stránka se načte a hned vydíte co dělate == rychlá iterace).

---

# Jak to spustím?

V hlavním adresáří spuste skript **playground.py**

!!! note

    Při spuštění na vás vyskočí uvodní dialog kde můžete přidat projekt který se nachází v adresáři `data`. Po
    vytvoření už stačí jenom na projekt dvakrát kliknout.

```bash
$ python playground.py
```

