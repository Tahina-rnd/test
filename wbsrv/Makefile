NAME		= webserv
TOK_BIN		= test_tokenizer
CFG_BIN		= test_config
CFG_SRCS	= $(SRC_DIR)/config/Config.cpp $(SRC_DIR)/config/ConfigError.cpp $(SRC_DIR)/config/ConfigFileTokenizer.cpp $(SRC_DIR)/config/ConfigSyntaxValidator.cpp $(SRC_DIR)/config/LocationConfig.cpp $(SRC_DIR)/config/ServerConfig.cpp $(SRC_DIR)/config/ServerConflicts.cpp
SRC_DIR		= srcs
OBJ_DIR		= .build
INC_DIRS	= includes/network includes/http includes/config includes/cgi includes/utils
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= $(addprefix -I, $(INC_DIRS))

SRCS		= $(shell find $(SRC_DIR) -name '*.cpp')
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
	@rm -rf $(OBJ_DIR)
	@echo "$(GREEN)objects removed$(RESET)"

fclean: clean
	@rm -f $(NAME) $(TOK_BIN) $(CFG_BIN)
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

.PHONY: all clean fclean re test-tokenizer test-config