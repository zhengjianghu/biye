# THIS IS A GENERATED FILE: PLEASE DO NOT EDIT

.SUFFIXES:

ifdef E_CC
  CC = $(E_CC)
endif

SHELL = /bin/sh

DEFINES = \
  -DEZSP_HOST \
  -DUNIX \
  -DUNIX_HOST \
  -DBOARD_HEADER=\"$--boardHeader--$\" \
  -DCONFIGURATION_HEADER=\"$--generatedHeader--$\" \
  -DPLATFORM_HEADER=\"$--halDirFromStackFs--$/micro/unix/compiler/gcc.h\" \
$--dashDMacrosMak:"\"--$

INCLUDES = \
  -I. \
  -I$--stackDirFromProjFs--$ \
  -I$--stackDirFromProjFs--$/app/rf4ce/include \
  -I$--stackDirFromProjFs--$/app/util \
  -I$--stackDirFromProjFs--$/stack \
  -I$--halDirFromProjFs--$ \
  -I$--halDirFromProjFs--$/.. \
$--includePathsMak--$

FILES = \
$--includeFilesMak--$

LIBRARIES = \
$--includeLibFilesMak--$

CFLAGS = -ggdb -O0 -Wall
CPPFLAGS = $(DEFINES) $(INCLUDES) -MF $(DEP_PATH)/$(@F:.o=.d) -MMD -MP

ifdef NO_READLINE
  CPPFLAGS += -DNO_READLINE
else
  LDLIBS = -lncurses -lreadline
endif

BUILD_PATH = build
DEP_PATH = $(BUILD_PATH)/dep
EXE_PATH = $(BUILD_PATH)/exe
OBJ_PATH = $(BUILD_PATH)/obj

DEPENDENCIES = $(addprefix $(DEP_PATH)/, $(notdir $(FILES:.c=.d)))
EXECUTABLE = $(EXE_PATH)/$--deviceName--$
OBJECTS = $(addprefix $(OBJ_PATH)/, $(notdir $(FILES:.c=.o)))

VPATH = $(dir $(FILES))

.PHONY: all
all: $(EXECUTABLE)

.PHONY: clean
clean:
	@-rm -f $(DEPENDENCIES) $(EXECUTABLE) $(OBJECTS)

ifneq ($(MAKECMDGOALS),clean)
  -include $(DEPENDENCIES)
endif

$(EXECUTABLE): $(LIBRARIES) $(OBJECTS) | $(EXE_PATH)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_PATH)/%.o: %.c | $(DEP_PATH) $(OBJ_PATH)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD_PATH):
	@mkdir $(BUILD_PATH)

$(DEP_PATH): | $(BUILD_PATH)
	@mkdir $(DEP_PATH)

$(EXE_PATH): | $(BUILD_PATH)
	@mkdir $(EXE_PATH)

$(OBJ_PATH): | $(BUILD_PATH)
	@mkdir $(OBJ_PATH)
