all:
	@echo Prepares CI docker image

try-1-build:
	docker build -t tensorviz:try -f try.dockerfile .

try-2-shell:
	docker run --gpus all -it tensorviz:try /bin/bash

try-3-tag:
	docker tag tensorviz:try otaviog/tensorviz:try

try-4-push:
	docker push otaviog/tensorviz:try

# Continouous integration

ci-1-build:
	docker build -t tensorviz:ci -f ci.dockerfile .

ci-2-tag:
	docker tag tensorviz:ci otaviog/tensorviz:ci

ci-3-shell:
	docker run -it otaviog/tensorviz:ci /bin/bash

ci-4-push:
	docker push otaviog/tensorviz:ci

ci-5-pull:
	docker pull otaviog/tensorviz:ci
