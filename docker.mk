all:
	@echo Docker image tasks

dev-build:
	docker build --target base -t otaviog/tensorviz:base .
	docker build -t otaviog/tensorviz:dev -f development.dockerfile .

dev-start:
	docker run --gpus all --user=`id -u`:`id -g` --env="DISPLAY"\
		-e NVIDIA_DRIVER_CAPABILITIES=all\
		-e XAUTHORITY\
		--volume="/etc/group:/etc/group:ro"\
		--volume="/etc/passwd:/etc/passwd:ro"\
		--volume="/etc/shadow:/etc/shadow:ro"\
		--volume="/etc/sudoers.d:/etc/sudoers.d:ro"\
		--volume="/tmp/.X11-unix:/tmp/.X11-unix:rw"\
		--volume=`pwd`:/workspaces/tensorviz\
		-it otaviog/tensorviz:dev /bin/bash

try-build:
	docker build --target try -t otaviog/tensorviz:try .

try-push:
	docker push otaviog/tensorviz:try
