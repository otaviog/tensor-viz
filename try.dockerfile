FROM nvidia/cudagl:10.2-devel-ubuntu18.04
MAINTAINER otaviog

RUN apt update && apt -yq install python3 aria2

WORKDIR /
RUN aria2c https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN bash Miniconda3-latest-Linux-x86_64.sh -b -p /miniconda3
ENV PATH="/miniconda3/bin/:${PATH}"

ADD environment.yml /
RUN conda env update -n base --file environment.yml

ADD . /tensorviz
WORKDIR /tensorviz
RUN python setup.py install -j2

RUN mkdir /exec
WORKDIR /exec

ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display
