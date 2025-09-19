.PHONY: docs format lint clean tests
test: format lint

docs:
	tox -e docs

format:
	tox -e format

lint:
	tox -e lint

clean:
	rm -rf .mypy_cache .pytest_cache .ruff_cache
	rm -rf *.egg-info
	rm -rf .tox dist site
	rm -rf coverage.xml .coverage

tests:
	python -m pytest --doctest-modules tests --cov --cov-config=pyproject.toml --cov-report=xml

docker_build:
	docker build -t pymp4v2-builder .

docker_run:
	docker run -it --rm pymp4v2-builder