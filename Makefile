CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -MMD -MP -g -O0
NAME = webserv
SRC_DIR = srcs
OBJ_DIR = obj
INC_DIR = includes
INCLUDES = -I$(INC_DIR)

SOURCES =   main.cpp \
			config/ConfigParser.cpp \
			config/LocationConfig.cpp \
			config/ServerConfig.cpp \
			utils/Logger.cpp \
			utils/findLocation.cpp \
			sockets/BindingSocket.cpp \
			sockets/ConnectingSocket.cpp \
			sockets/ListeningSocket.cpp \
			sockets/SimpleSocket.cpp \
			servers/ServerManager.cpp \
			clients/Client.cpp \
			http/HttpRequest.cpp \
			http/HttpResponse.cpp \
			cgi/CgiHandler.cpp

SRCS = $(addprefix $(SRC_DIR)/, $(SOURCES))
OBJS = $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))
DEPS = $(OBJS:.o=.d)

HEADER_FILES = 	utils/Logger.hpp \
				utils/findLocation.hpp \
				exceptions/WebservException.hpp \
				exceptions/ParserException.hpp \
				exceptions/SocketException.hpp \
				exceptions/HttpException.hpp \
				sockets/BindingSocket.hpp \
				sockets/ConnectingSocket.hpp \
				sockets/ListeningSocket.hpp \
				sockets/SimpleSocket.hpp \
				sockets/Socket.hpp \
				servers/ServerManager.hpp \
				clients/Client.hpp \
				http/HttpRequest.hpp \
				http/HttpResponse.hpp \
				http/HttpRequest.hpp\
				cgi/CgiHandler.hpp

HEADERS = $(addprefix $(INC_DIR)/, $(HEADER_FILES))

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking program..."
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean:
	@echo "Cleaning object files..."
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "Cleaning all..."
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re