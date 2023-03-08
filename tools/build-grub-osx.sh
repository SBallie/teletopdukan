
#!/bin/sh
set -e

# First we are going to make sure that you understand this is sort of experimental and we will be compiling stuff.

# by default CONTINUE will be false
CONTINUE=false

echo ""
echo "You are about to download, compile, and install stuff on your computer."
echo "Please read through the source script to know what is being done."
echo "Do you want to continue? (y/n)"
read -r response
if [[ $response =~ ^([yY][eE][sS]|[yY])$ ]]; then
  CONTINUE=true
fi

if ! $CONTINUE; then
  # Bail if response is not yes
  echo "Exiting..."
  exit
fi

# check if `brew` is installed
command -v brew >/dev/null 2>&1 || { echo >&2 "It seems you do not have \`brew\` installed. Head on over to http://brew.sh/ to install it."; exit 1; }

export PREFIX="$HOME/opt/"
export TARGET=x86_64-pc-elf
export PATH="$PREFIX/bin:$PATH"

mkdir -p $HOME/src
mkdir -p $PREFIX

# gmp mpfr libmpc
brew install gmp mpfr libmpc autoconf automake

# binutils