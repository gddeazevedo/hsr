IMAGE=hsr
FILE=main.cc
PWD=$(shell pwd)

build:
	docker build -t $(IMAGE) .

bash:
	docker run -it --rm -v "$(PWD)":/app $(IMAGE) bash

run:
	docker run --rm -v "$(PWD)":/app $(IMAGE) \
		bash -c "g++ $(FILE) -o out/main /usr/local/lib/libasmjit.a && ./out/main"

run-demo:
	docker run --rm -v "$(PWD)":/app -w /app $(IMAGE) \
		bash -c "g++ demos/$(file) -o out/demo_exec /usr/local/lib/libasmjit.a && ./out/demo_exec"
