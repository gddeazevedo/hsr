IMAGE=hsr
CONTAINER_NAME=hsr
FILE=main.cc
PWD=$(shell pwd)

build:
	docker build -t $(IMAGE) .

start-bash:
	docker run -it --rm -v "$(PWD)":/hsr --name $(CONTAINER_NAME) $(IMAGE)

run:
	docker run --rm -v "$(PWD)":/hsr $(IMAGE) \
		bash -c "g++ $(FILE) -o out/main /usr/local/lib/libasmjit.a && ./out/main"

run-demo:
	docker run --rm -v "$(PWD)":/hsr -w /hsr $(IMAGE) \
		bash -c "g++ demos/$(file) -o out/demo_exec /usr/local/lib/libasmjit.a && ./out/demo_exec"
