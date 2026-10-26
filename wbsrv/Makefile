NAME		= webserv
TOK_BIN		= test_tokenizer
CFG_BIN		= test_config
HTTP_BIN	= test_http

SRC_DIR		= srcs
OBJ_DIR		= .build
INC_DIRS	= includes/network includes/http includes/config includes/cgi includes/utils
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= $(addprefix -I, $(INC_DIRS))

CFG_SRCS	= $(SRC_DIR)/config/Config.cpp \
			  $(SRC_DIR)/config/ConfigError.cpp \
			  $(SRC_DIR)/config/ConfigFileTokenizer.cpp \
			  $(SRC_DIR)/config/ConfigSyntaxValidator.cpp \
			  $(SRC_DIR)/config/LocationConfig.cpp \
			  $(SRC_DIR)/config/ServerConfig.cpp \
			  $(SRC_DIR)/config/ServerConflicts.cpp

HTTP_SRCS	= $(SRC_DIR)/http/Mime.cpp \
			  $(SRC_DIR)/http/Request.cpp \
			  $(SRC_DIR)/http/Response.cpp

CGI_SRCS	= $(SRC_DIR)/cgi/CgiHandler.cpp

NET_SRCS	= $(SRC_DIR)/network/Socket.cpp \
			  $(SRC_DIR)/network/Server.cpp \
			  $(SRC_DIR)/network/Connection.cpp

SRCS		= $(SRC_DIR)/main.cpp \
			  $(SRC_DIR)/utils/utils.cpp \
			  $(CFG_SRCS) \
			  $(NET_SRCS) \
			  $(HTTP_SRCS) \
			  $(CGI_SRCS)

OBJS		= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
DEPS		= $(OBJS:.o=.d)

RESET		= \033[0m
GREEN		= \033[32m
YELLOW		= \033[33m

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled successfully$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@
	@echo "$(YELLOW)compiled:$(RESET) $<"

-include $(DEPS)

clean:
	@find $(OBJ_DIR) -type f -delete 2>/dev/null || true; @rm -rf $(OBJ_DIR) 2>/dev/null || true
	@echo "$(GREEN)objects removed$(RESET)"

fclean: clean
	@rm -f $(NAME) $(TOK_BIN) $(CFG_BIN) $(HTTP_BIN)
	@echo "$(GREEN)$(NAME) removed$(RESET)"

re: fclean all

# Binaire de test du tokenizer (hors webserv : main() séparé).
test-tokenizer:
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC_DIR)/config/ConfigFileTokenizer.cpp test/test_tokenizer.cpp -o $(TOK_BIN)
	@echo "$(GREEN)$(TOK_BIN) compiled successfully$(RESET)"

# Binaire de preuve du stockage/héritage (main() séparé).
test-config:
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(CFG_SRCS) test/test_config.cpp -o $(CFG_BIN)
	@echo "$(GREEN)$(CFG_BIN) compiled successfully$(RESET)"

# Binaire de test unitaire HTTP (Request, Response, Mime).
test-http:
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(CFG_SRCS) $(HTTP_SRCS) test/test_http.cpp -o $(HTTP_BIN)
	@echo "$(GREEN)$(HTTP_BIN) compiled successfully$(RESET)"

.PHONY: all clean fclean re test-tokenizer test-config test-http