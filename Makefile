all:	
	#cat main.c.in | m5 > main.c 2>&1
	emcc main.c \
		-std=c11 \
		-s WASM=1 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -O3 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 \
		--preload-file texture.png --use-preload-plugins -o index.js	