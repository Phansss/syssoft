# if you created a subfolder for every exercise this target will
# build and execute the main.c file in the ex3 folder
ex2:
	mkdir -p build
	gcc -Wall -Werror -o ./build/ex2 exercise2/main.c
	./build/ex2

ex3:
	mkdir -p build
	gcc -Wall -Werror -o ./build/ex3 exercise3/main.c
	./build/ex3


# the main file in the ex3 folder will be ziped and is then ready to
# be submitted to labtools.groept.be
zip:
	cd ex3
	zip ../lab2_ex3.zip main.c
	cd ..
