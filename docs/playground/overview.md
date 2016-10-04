# Playground

[![Prototyp](../img/prototyp.png)](../img/prototyp.png)

Development environment that tries to ease **modifiable** so that it can best comply with the specifications of the project
if needed. Therefore, the selected combination of languages **Python** **LUA**, **JS** **HTML**.

# Why so many languages?

* **Python + QT** is base. Good support for multiplatform.
* **LUA** for comunication with engine. This allows code to be separated from engine tools (tool only sends the script and the sets the desired behavior, and then you can just communicate through `console_server`)

* **HTML + JS** for vizualization and widgets. Easy to use and powerful visualization (and you too can have runing
editor, edit the code, and then just when to pull reload the page loads and just see what makes == rapid iteration).

# How does it run?

In the main directory, run the script **playground.py**

```bash
$ python3 playground/playground.py
```

!!! note

    When you start to pop up a dialog displays where you can add a project which is located in the `examples/develop` dir. After
    enough to create a project just double-click.


