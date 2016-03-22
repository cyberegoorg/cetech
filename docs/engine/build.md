# A co závislosti?

V hlávním adresáři se nachází python skript [externals.py](#externals.py) který vše obstará.

## externals.py

### Nejčastejší použití

Připravý všechny závislosti pro aktuální [platformu](#podporovane-platformy) v konfiguraci [develop](#develop)

```bash
$ python externals.py
```

# Jak to zkompilovat?

Engine pro vlastní kompilaci používa skripty vygenerované pomocí programu [GENie][genie] (jde o fork premake4.4
vice [zde][genie_why]). A díky tomu je možne generovat makefile a nebo projekty pro visual studio a navíc pro různé
platformy. Aby se celý proces zjednodušil existuje v hlavním adresáři python skript [make.py](#make.py) který se
stará o celou srandu kolem kompilace.

## make.py

Skript který se nachází v hlavním adresáři projektu a zjednodušuje a sjednocuje proces kompilace na ruzných platformách.

### Nejčastejší způsob kompilace

Pro začátek stačí zkompilovat engine pro aktuální [platformu](#podporovane-platformy) v konfiguraci [develop](#develop) jednoduše takto:

```bash
$ python make.py
```

### Použití
```bash
python make.py [AKCE]
```

Akce    | Popis
--------|----------------------------------------------------------------------------
` `     | Pokud není nic zadáno provede se **build**.
`clean` | Smaže složku kterou vytvořil **build**. Dost často řesí spoustu problému =D

### Podporované platformy

Engine se snaží podporovat všechny platformy jako knihovna [bgfx][bgfx], ale zatím je na začátku vývoje tak je podpora
dosti omezená (ok... zatím to beží jenom na linuxu takže se dá hovořit o uniplatformnosti ale kupodivu ji dělá dobře).

Platforma  | Popis
-----------|------------------------------
`linux64`  | Linux na architektuře x86-64


### Build konfigurace
Konfigurace | Optimalizace kompilatoru | Kod pro tvorbu | Debug
------------|--------------------------|----------------|------
`develop`   | Ano                      | Ano            | Ne
`debug`     | Ne                       | Ano            | Ano
`release`   | Ano                      | Ne             | Ne

#### Develop
Build  obsahuje vše potřebné pro komunikaci s editorem, tvorbou herního obsahu a její kompilaci.
Jedná se o nejčastejí využívanou konfiguraci během vývoje.

#### Debug
Build má zapnutou kontrolu chyb jako např. asserty. Jsou vypnuté optimalizace kompilátoru a proto je tento build výrazně
pomalejší než `develop`. Tuto konfiguraci nejčastěji využijete pokud upravujete kód enginu.

#### Release
Tento build je primárně určený pro finální verzi hry nebo aplikace. Má odstaněnou veškerou funkcionalitu pro
vývoj a editor jako např. Console server, Kompilaci resource souboru. A patří mezi nejrychlejší konfigurace.

---

[genie]: https://github.com/bkaradzic/genie
[genie_why]: https://github.com/bkaradzic/genie#why-fork
[bgfx]: https://github.com/bkaradzic/bgfx
