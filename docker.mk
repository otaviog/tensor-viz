all:
	@echo Docker image tasks

base-build:
	docker build -t otaviog/tensorviz-base:latest --target base .

base-start: base-build
	docker run -it otaviog/tensorviz-base:latest bash

dev-build:
	docker build -t otaviog/tensorviz-devcontainer:latest --target devcontainer .

dev-start: dev-build
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
	docker build --target try -t otaviog/tensorviz:latest .

try-push: try-build
	docker push otaviog/tensorviz:latest
