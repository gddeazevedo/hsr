FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY asmjit /app/asmjit

# Compila asmjit como biblioteca estática
RUN cmake -S /app/asmjit -B /app/asmjit/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DASMJIT_STATIC=ON \
    && cmake --build /app/asmjit/build --parallel \
    && cmake --install /app/asmjit/build --prefix /usr/local

COPY . /app

CMD ["/bin/bash"]