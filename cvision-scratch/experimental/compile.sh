#!/bin/sh

EXIT_STATUS=no
CC_EXIST=yes
CPP_EXIST=yes

if [ ! -d "src" ]; then
  echo "src directory doesn't exist."
  EXIT_STATUS=yes
  exit 1
fi

if [ ! -d "build" ]; then
  echo "build directory doesn't exist."
  echo "creating build directory...."
  mkdir build
  EXIT_STATUS=yes
  exit 1
fi

if [ "$EXIT_STATUS" = "no" ] ; then
	
	if [ "$1" = "clean" ] ; then
	  cd build
	  echo "cleaning build directory."
	  rm -rf *
	  cd ..
	  EXIT_STATUS=yes
	  exit 1
	fi
fi

if [ "$EXIT_STATUS" = "no" ] ; then
	
	cd src

	for i in *.cpp; do
		if [ ! -f $i ]; then
			echo "C++ file(s) not found"
			echo "\n"
			CPP_EXIST=no
		fi
	done	

	for i in *.c; do
		if [ ! -f $i ]; then
			echo "C file(s) not found"
			echo "\n"
			CC_EXIST=no
		fi
	done	

	if [ "$CC_EXIST" = "yes" ] ; then

		for i in *.c; do
			echo "compiling $i"
			gcc -ggdb `pkg-config --cflags opencv` -o `basename $i .c` $i `pkg-config --libs opencv`;
			
			if [ -f `basename $i .c` ]; then
				mv `basename $i .c` ../build/
			else
				echo "$i failed"
			fi
		
		done

	fi

	if [ "$CPP_EXIST" = "yes" ] ; then

		for i in *.cpp; do
			echo "compiling $i"
			g++ -ggdb `pkg-config --cflags opencv` -o `basename $i .cpp` $i `pkg-config --libs opencv` -std=c++14;
			
			if [ -f `basename $i .cpp` ]; then
				mv `basename $i .cpp` ../build/
			else
				echo "$i failed"
			fi
			
		done
		
	fi
fi

cd ..

echo "\n"