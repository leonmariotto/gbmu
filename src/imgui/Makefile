
NAME = libimgui.a

SRC += imgui.cpp
SRC += imgui_demo.cpp
SRC += imgui_draw.cpp
SRC += imgui_impl_sdl.cpp
SRC += imgui_impl_sdlrenderer.cpp
SRC += imgui_stdlib.cpp
SRC += imgui_tables.cpp
SRC += imgui_widgets.cpp

OBJ = $(addprefix obj/,$(SRC:.cpp=.o))

CXXFLAGS = -std=gnu++14 -Wall -Wextra

all: $(NAME)

$(NAME): $(OBJ)
	ar rc ../../$(NAME) $(OBJ)

obj/%.o:%.cpp *.h
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	rm -rf obj

fclean : clean
	rm -rf ../../$(NAME)

re : fclean all

