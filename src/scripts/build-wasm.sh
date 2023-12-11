emcc -o index.html main.c -Os -Wall ./lib/libraylib-wasm.a -I. -Iinclude -L. -Llib -s USE_GLFW=3 -s ASYNCIFY --shell-file ./shell.html -DPLATFORM_WEB
mv index.* bin