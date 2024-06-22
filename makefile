.PHONY: default run setup clean test

default: run

PROJECT_NAME = GPGPU
BUILD_DIR = out
RELEASE_FOLDER=${BUILD_DIR}/Release
DEBUG_FOLDER=${BUILD_DIR}/Debug
release_run:
	cmake -B ./${RELEASE_FOLDER} -S .
	cmake --build ./${RELEASE_FOLDER} --config Release
	cd src && ../${RELEASE_FOLDER}/${PROJECT_NAME}

# Futtatja a programot DEBUG flagekkel.
run:
	cmake -B ./${DEBUG_FOLDER} -S .
	cmake --build ./${DEBUG_FOLDER} --config Debug
# A "src" mappában indítja el a programot, tehát a "src" mappához relatív útvonalakkal kell hivatkozni a shaderekre, textúrákra, egyéb fájlokra. (Ezt a módszert használja a Visual Studio is.)
	cd src && ../${DEBUG_FOLDER}/${PROJECT_NAME}
	

# Újra konfigurálja a projektet
# (Amennyiben véletlenül hibásan módosítottuk a build mappát)
clean:
	rm -rf ${BUILD_DIR}
	rm -rf ./.cache

# Konfigurálja a programot és ellenőrzi, hogy a kezdő projekt működik-e
test: clean
	$(MAKE) run
