#!/bin/bash

# TODO: rename to bootstrap or setup
banner() {
	echo "|--------------------------------------------|"
	echo "|----- Welcome to the TranbyOS bootstrap ----|"
	echo "|--------------------------------------------|"
}

banner();

osx()
{
	echo "Detected OSX!"
	if [ ! -z "$(which brew)" ]; then
		echo "Homebrew detected! Now updating..."
		brew update
		if [ -z "$(which git)" ]; then
			echo "Now installing git..."
			brew install git
		fi
		if [ "$2" == "qemu" ]; then
			if [ -z "$(which qemu-system-i386)" ]; then
				echo "Installing qemu..."
				brew install qemu
			else
				echo "QEMU already installed!"
			fi
		else
			if [ -z "$(which virtualbox)" ]; then
				echo "Now installing virtualbox..."
				brew cask install