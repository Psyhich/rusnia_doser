FROM debian:bookworm
WORKDIR /app
RUN export DEBIAN_FRONTEND=noninteractive && \
	apt update && apt install -y build-essential cmake git python3-pip && \
	python3 -m pip install conan
RUN git clone --recurse-submodules https://github.com/Psyhich/rusnia_doser.git && \
	cd rusnia_doser && \
	mkdir build && \ 
    cd build && \
	conan install ../ && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build ./ -j 8 -t rusnia_doser

FROM debian:bookworm
WORKDIR /app
COPY --from=0 /app/build/bin/rusnia_doser ./rusnia_doser
CMD ["/app/rusnia_doser", "--target", "auto", "--threads", "100"]
