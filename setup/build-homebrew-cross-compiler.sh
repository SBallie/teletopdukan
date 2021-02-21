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
		if [ -z 