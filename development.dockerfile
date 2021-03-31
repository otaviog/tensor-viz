FROM otaviog/tensorviz:base
LABEL maintaner=otavio.b.gomes@gmail.com

RUN pip install -U pip
ADD requirements-dev.txt .
RUN pip install -r requirements-dev.txt

RUN apt update && DEBIAN_FRONTEND=noninteractive apt -yq install sudo git byobu

ARG USERNAME=tensorviz
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create the user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME

SHELL ["/bin/bash", "-c"]

ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display
