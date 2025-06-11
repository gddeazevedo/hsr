IMAGE=hsr
FILE=main.cc
PWD=$(shell pwd)

all: run

build:
	docker build -t $(IMAGE) .

bash:
	docker run -it --rm -v $(PWD):/app $(IMAGE) bash

run:
	docker run --rm -v $(PWD):/app $(IMAGE) \
		bash -c "g++ $(FILE) -o main /usr/local/lib/libasmjit.a && ./main"

clean:
	rm -f main
