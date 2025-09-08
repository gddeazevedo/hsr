FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake gdb \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /hsr

COPY asmjit /hsr/asmjit

# Compila asmjit como biblioteca estática
RUN cmake -S /hsr/asmjit -B /hsr/asmjit/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DASMJIT_STATIC=ON \
    && cmake --build /hsr/asmjit/build --parallel \
    && cmake --install /hsr/asmjit/build --prefix /usr/local

COPY . .

CMD ["/bin/bash"]
