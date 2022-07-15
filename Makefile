include variables.mk

###################
# OBS external deps
###################

fetch-obs-dependencies-Linux:
	cd obs-studio/obs-studio/CI/linux && \
	./01_install_dependencies.sh

fetch-obs-dependencies-Darwin:
	cd obs-studio/obs-studio/CI/macos && \
	./01_install_dependencies.sh

fetch-obs-dependencies:
	$(MAKE) fetch-obs-dependencies-${UNAME}


###################
# Protos generation
###################

grpc-plugin:  # @@Dependencies@@ Build gRPC plugin for the C proto compiler
	cd ${BUILD_RELEASE_DIR} && ninja grpc

client-protos: grpc-plugin
	@if [ -f "./build_release/install/grpc/bin/grpc_cpp_plugin" ] ; then \
		rm -rf ./build_release/build/s2t-obs/proto-src ; \
		mkdir -p ./build_release/build/s2t-obs/proto-src ; \
		./build_release/install/grpc/bin/protoc \
			-I=./s2t-obs/protos \
			--cpp_out=./build_release/build/s2t-obs/proto-src \
			./s2t-obs/protos/s2t.proto ; \
	else \
		echo "GRPC Plugin Not Found. Please build plugin" ; \
	fi

server-protos:
	cd triton && \
	source .env/bin/activate && \
	python -m grpc_tools.protoc \
		--proto_path=../s2t-obs/protos \
		--python_out=./src/grpc \
		--grpc_python_out=./src/grpc \
		../s2t-obs/protos/s2t.proto && \
	deactivate

#########
# S2T_OBS
#########

${S2T_OBS_BUILD_CMAKE}:
	$(MKDIR) "${BUILD_RELEASE_DIR}" && \
	echo "Configure CMake with CMAKE_BUILD_TYPE='${BUILD_TYPE}'" && \
	cd "${BUILD_RELEASE_DIR}" && \
	../csm/csm.py ../ && \
	cmake -G Ninja \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DUSE_LOGS=ON \
		-DEXTERNAL_PROJECT_NUM_JOBS=${NPROC} \
		${OBS_MACOS_CMAKE_FLAGS} \
		.

build: ${S2T_OBS_BUILD_CMAKE} # @@Build@@ Build the plugin
	cd "${BUILD_RELEASE_DIR}" && ninja

#########
# Linting
#########

install-lint-deps:
	pip3 install cpplint
	cpplint --version

lint:
	cpplint --recursive s2t-obs/src