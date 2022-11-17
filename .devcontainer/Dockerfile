FROM debian:bullseye-slim

SHELL [ "/bin/bash", "-c" ]

RUN apt-get update && apt-get \
      install -y \
        wget \
        gnupg

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - >/dev/null && \
    echo "deb http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-15 main" | tee -a /etc/apt/sources.list >/dev/null && \
    echo "deb-src http://apt.llvm.org/bullseye/ llvm-toolchain-bullseye-15 main" | tee -a /etc/apt/sources.list >/dev/null && \
    apt-get update && apt-get \
      install -y \
        build-essential \
        clang-15 \
        clang-format-15 \
        clangd-15 \
        cmake \
        curl \
        gdb \
        git \
        vim && \
    rm -rf /var/lib/apt/lists*

RUN apt-get install -y \
      libboost1.74-all-dev
