about:
	@echo "Project maintaining tasks"

pylint:
	python -m pylint tenviz

pep8:
	python -m autopep8 --recursive --in-place tenviz

doc-create:
	rm -f doc/source/rflow.*.rst
	sphinx-apidoc -o doc/source tenviz
	make -C doc/ html

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

local-ci-test:
	gitlab-runner exec docker test --docker-pull-policy=never
