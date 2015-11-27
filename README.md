# CETech

CETech is Data-Driven [game engine](https://en.wikipedia.org/wiki/Game_engine) and toolbox for game and
interactive application developing.

---

* [Authors](AUTHORS.md)
* [License](LICENSE.md)
* [History](HISTORY.md)
* [Release notes](http://cyberegoorg.github.io/cetech/about/release-notes)
* [Documentation (temporarily in czech language](http://cyberegoorg.github.io/cetech)

---

## Dependency

* Qt5
* Python 3.5
  * PyQt5
  * yaml

## Suported platforms

* Linux

## Compile

* install python3.5
* install [PyQt5][pyqt]
* install requirements.txt

    ```bash
    $ pip install -r requirements.txt
    ```

* Download and compile externals.

    ```bash
    $ python external.py
    ```

* Install pyenet (folder `3rparty/pyenet`)

    ```bash
    $ python setup.py install
    ```

* Compile engine

    ```bash
    $ python make.py
    ```

## Run playground

```bash
$ python playground.py
```

* Click on `add exist`.
* Set name `SAMPLE`.
* Click on `...` and select repo folder `data`.
* Click on `ok`.
* Double click on `SAMPLE`.
* Enjoy playground bugs.

[pyqt]: https://riverbankcomputing.com/software/pyqt/download5
