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
				brew cask install virtualbox
			else
				echo "Virtualbox already installed!"
			fi
		fi
	else
		echo "Homebrew does not appear to be installed! Would you like me to install it?"
		printf "(Y/n): "
		read -r installit
		if [ "$installit" == "Y" ]; then
			ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
		else
			echo "Will not install, now exiting..."
			exit
		fi
	fi
	echo "Cloning Redox repo"
	git clone -b "$1" --recursive https://github.com/redox-os/redox.git
	echo "Running Redox setup script..."
	sh redox/setup/osx-homebrew.sh
	echo "Running rust install script"
	sh redox/setup/bina