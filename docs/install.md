# Naklonování repozitáře

!!! important

    Je potřeba mít naistalovaný a funční [git][git_setup]

Pokud *git* funguje tak už stačí jenom naklonovat repo `git clone git@github.com:cyberegoorg/cetech.git`.

# Kompilace

!!! important

    Je potřeba mít naistalovaný a funční [python3.5][python3] a nainstalovat závislosti `pip install -r requirements.txt`

Před prvním spuštěním je potřeba [zkompilovat engine][engine_build] a připravit [playground](#priprava-playground).

# Příprava Playground

!!! important

    Je potřeba naistalovat knihovnu [PyQT5][pyqt] .

!!! important

    Je potřeba naistalovat knihovnu `pyenet` která se nachází v adresáři `3rdparty/pyenet`.

    ```$ python setup.py install```


[pyqt]: https://riverbankcomputing.com/software/pyqt/download5
[playground]: playground/overview.md
[engine]: engine/overview.md
[git_setup]: https://help.github.com/articles/set-up-git/
[python3]: https://www.python.org/downloads/
[engine_build]: engine/build.md
