BOLD_PURPLE	=	\033[1;35m
BOLD_CYAN	=	\033[1;36m
BOLD_YELLOW	=	\033[1;33m
NO_COLOR	=	\033[0m
GRAY        =	\033[0;90m
RED         =	\033[0;91m
GREEN       =	\033[0;92m
YELLOW      =	\033[0;93m
BLUE        =	\033[0;94m
MAGENTA     =	\033[0;95m
CYAN 		=	\033[0;96m
WHITE		=	\033[0;97m
DEF_COLOR	=	\033[0;39m

ifndef color_printf
define color_printf
	@echo "$(1)\n[$(2)] $(3)$(DEF_COLOR)"
endef
endif