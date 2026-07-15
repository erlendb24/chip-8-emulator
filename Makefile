.PHONY: all analyze release

TARGET := chip
all:
	clang *.c -Wall -Wextra -pedantic -g -fsanitize=undefined -Iinclude -I/opt/homebrew/Cellar/raylib/5.5/include -L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib -framework Cocoa -framework IOKit -framework OpenGL -framework CoreVideo -o $(TARGET)
analyze: 
	gcc-14 *.c -Wall -Wextra -pedantic -g -fanalyzer -Iinclude -I/opt/homebrew/Cellar/raylib/5.5/include -L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib -framework Cocoa -framework IOKit -framework OpenGL -framework CoreVideo -o $(TARGET)
release:
	clang *.c -Wall -Wextra -pedantic -Iinclude -I/opt/homebrew/Cellar/raylib/5.5/include -L/opt/homebrew/Cellar/raylib/5.5/lib -framework Cocoa -framework IOKit -framework OpenGL -framework CoreVideo -lraylib -o $(TARGET)


