.PHONY: default run setup clean test

default: run

PROJECT_NAME = "GPGPU"
release_run:
	cd build && time cmake --build . --config Release
	cd src && time ../build/${PROJECT_NAME}

# Futtatja a programot DEBUG flagekkel.
run:
	cd build && time cmake --build . --config Debug
	# A "src" mappában indítja el a programot, tehát a "src" mappához relatív útvonalakkal kell hivatkozni a shaderekre, textúrákra, egyéb fájlokra. (Ezt a módszert használja a Visual Studio is.)
	cd src && time ../build/${PROJECT_NAME}

# Konfigurálja a projektet
setup:
	cmake -B build -S .

# Újra konfigurálja a projektet
# (Amennyiben véletlenül hibásan módosítottuk a build mappát)
clean:
	rm -rf ./build
	rm -rf ./cache
	$(MAKE) setup

# Konfigurálja a programot és ellenőrzi, hogy a kezdő projekt működik-e
test: clean
	$(MAKE) run
