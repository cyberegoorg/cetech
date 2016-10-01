FROM ubuntu:latest

ADD requirements.txt /tmp/requirements.txt
ADD dev_requirements.txt /tmp/dev_requirements.txt

RUN apt-get update
RUN apt-get install -y clang \
                        cmake \
                        libx11-dev \
                        libjemalloc-dev \
                        libgl1-mesa-dev \
                        zlib1g-dev \
                        python3-pip

RUN pip3 install -r /tmp/requirements.txt
RUN pip3 install -r /tmp/dev_requirements.txt