
#! /bin/sh

OSNAME="tranbyos"

BUILD_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJ_DIR="${BUILD_DIR}/.."
SRC="${PROJ_DIR}/src"
OBJ_DIR="${PROJ_DIR}/obj"
BIN_DIR="${PROJ_DIR}/bin"
ASM_DIR="${SRC}/asm"
INC_DIR="${SRC}/include"

echo "$OSNAME $BUILD_DIR $SRC $OBJ_DIR $BIN_DIR $ASM_DIR $INC_DIR"
echo "grub.cfg auto-create disabled."

# TODO: init grub.cfg if doesn't exist
#OSNAME=tranbyos
#BUILD_DIR=tools
# echo "${OSNAME}:\n\ttitle ${OSNAME}\n\troot (fd0)\n\tset gfxmode=auto\n\tkernel /boot/${OSNAME}.bin\n" > "${BUILD_DIR}/grub.cfg"

##############################################
# Copy Into Grub Image
has_command() { 
	cmd = $1 || ""
	if command -v $1 >/dev/null 2>&1; then
		echo "has command."
	else
		echo >&2 "I require $1 but it's not installed. Aborting."; exit 1; 
	fi
}

copy_grub()
{
	if has_command "hdiutil"; then
		echo "WE HAVE hdiutil, use it"
	elif has_command "uname"; then
		echo "WE HAVE uname, use it"
	fi