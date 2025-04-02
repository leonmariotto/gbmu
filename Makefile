
NAME = gbmu

FILES = main.cpp \
		Cpu.cpp \
		CpuUtility.cpp \
		Mem.cpp \
		InstructionsControl.cpp \
		InstructionsJumpCalls.cpp \
		Instructions8BitLoad.cpp \
		Instructions16BitLoad.cpp \
		Instructions16BitArithmetic.cpp \
		Instructions8BitArithmetic.cpp \
		Instructions8BitShift.cpp \
		InstructionsMisc.cpp \
		FlagOp.cpp \
		Clock.cpp \
		Screen.cpp \
		UserInterface.cpp \
		Ppu.cpp \
		Joypad.cpp \
		Gameboy.cpp \
		TilePixels.cpp \
		CpuStackTrace.cpp \
		MBC.cpp \
		Sprite.cpp \
		Debugger.cpp \
		Hdma.cpp \
		Common.cpp \
		APU.cpp \
		SquareWave.cpp \
		Waveform.cpp \
		Noise.cpp


APPIMAGE = gbmu-x86_64.AppImage

IMGUI_PATH = src/imgui/

SDLINCPATH ?= /usr/local/include/SDL2

SDLFLAGS = $(shell pkg-config sdl2 --libs --static)

LIBS = libimgui.a
OBJ = $(addprefix obj/,$(FILES:.cpp=.o))
CXX = g++

LDFLAGS = -Wl,--gc-sections
CXXFLAGS = -std=gnu++14 -Wall -Wextra -O3 -g -ffunction-sections -fdata-sections
#-g3 -Og

all: $(NAME)

$(NAME): $(OBJ)
	make -C $(IMGUI_PATH)
	${CXX} ${LDFLAGS} -o $@ -I ${SDLINCPATH} ${OBJ} ${SDLFLAGS} $(LIBS)

app: $(APPIMAGE)

$(APPIMAGE): $(NAME)
	./tools/linuxdeploy-x86_64.AppImage -e gbmu -i resources/gbmu.png --create-desktop-file --appdir AppDir
	./tools/appimagetool-x86_64.AppImage AppDir
	rm -rf AppDir

obj/%.o:src/%.cpp src/*.hpp src/define.hpp src/*.tpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I ${SDLINCPATH}

clean :
	@#make -C $(IMGUI_PATH) clean
	rm -rf obj

fclean : clean
	@#make -C $(IMGUI_PATH) fclean
	rm -rf $(NAME)
	rm -rf $(APPIMAGE)

re : fclean all

.PHONY: all norme clean fclean re app
