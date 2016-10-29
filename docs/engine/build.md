# Build

## Linux

### Prerequisites

* Install python and pip >= 3.4
* Install QT5 + PyQT
* Install requirements.txt (```$ pip3 install -r scripts/requirements.txt```)
* Optional install dev_requirements.txt (```$ pip3 install -r scripts/dev_requirements.txt```)

### Build

```bash
$ python3 scripts/build.py
```

### Run

#### Engine develop build

```bash
$ ./bin/linux64/cetech_develop -compile -continue -src ./examples/develop/src -build ./examples/develop/build
```

#### Playground

```bash
$ python3 playground/playground.py
```
* add folder `examples/develop`