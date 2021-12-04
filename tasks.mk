about:
	@echo "Project maintaining tasks"

pylint:
	python -m pylint tenviz

pep8:
	python -m autopep8 --recursive --in-place tenviz

unit-test:
	python3 -m unittest discover

doc-create:
	rm -f doc/source/rflow.*.rst
	sphinx-apidoc -o doc/source tenviz
	cd doc && sphinx-build -b html source/ html

doc-open:
	sensible-browser doc/build/html/index.html

clang-format:
	clang-format -i **/*.cpp **/*.hpp

clang-tidy:
	find . -name "*.cpp" ! -name "CMake*.cpp" | xargs clang-tidy -p . -extra-arg "-fsized-deallocation"

cpp-doc-create:
	doxygen Doxyfile

cpp-doc-open:
	sensible-browser doc/cpp/html/index.html

ci-build-images:
	gitlab-runner exec shell build:images --docker-pull-policy=never --docker-gpus all

ci-linter:
	gitlab-runner exec docker linter --docker-pull-policy=never --docker-gpus all

ci-unit-test:
	gitlab-runner exec docker unit-test --docker-pull-policy=never --docker-gpus all

ci-pages:
	gitlab-runner exec docker pages --docker-pull-policy=never --docker-gpus all
