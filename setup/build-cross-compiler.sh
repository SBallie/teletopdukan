# This is a work in progress
# Build a cross-compiler for i686

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Build binutils
cd $HOME/src
mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --