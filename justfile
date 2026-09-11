configure:
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

build: configure
    cmake --build build -j

run: build
    ./build/soundchest

configure-release:
    cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release

build-release: configure-release
    cmake --build build-release -j

run-release: build-release
    ./build-release/soundchest

clean:
    -cmake --build build --target clean
    -cmake --build build-release --target clean

wipe:
    rm -rf build build-release
