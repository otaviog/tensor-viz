# Contributing to TensorViz

## Setup Local Developer Mode

The `environment.yml` contains the dependencies. Installing command line:


**Create environment - Option 1**: Using a local `conda` installation

```
tensorviz$ conda env create -f environment.yml
```

**Create environment - Option 2**: Using a local `conda` installation

```shell
tensorviz$ aria2c https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
tensorviz$ bash Miniconda3-latest-Linux-x86_64.sh -b -p env
tensorviz$ env/bin/activate
tensorviz$ conda env update --file environment.yml
```

**Install in setup mode**

```shell
(env) tensorviz$ python setup.py develop
```

## Setup Docker Developer Mode (VSCode)

In `Remote-Containers: Open Folder In Container`, select the `development.dockerfile`. 
On the `.devcontainer/devcontainer.json` add some useful configurations:

```json
...
	"extensions": [
		"ms-vscode.cpptools",
		"ms-python.python",
		"ms-python.vscode-pylance"
	],
	"build": { "target": "devcontainer" },
	"runArgs": [
		"--gpus", "all",
		"-e", "XAUTHORITY",
		"-e", "DISPLAY=${env:DISPLAY}",
		"-v", "/tmp/.X11-unix:/tmp/.X11-unix:rw"
	],

	"postStartCommand": "pip3 install -e /workspaces/tensorviz",
...
```

