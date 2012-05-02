# Makefile for the Meilhaus Java extension classes

SOURCE_DIR := sources
OUTPUT_DIR := classes

JAVA_HOME := /usr/lib/jvm/java
JAVA := $(JAVA_HOME)/bin/java
JAVAC := $(JAVA_HOME)/bin/javac

JFLAGS := -sourcepath $(SOURCE_DIR) -d $(OUTPUT_DIR)
JVMFLAGS := 

JVM := $(JAVA) $(JVM_FLAGS)
FIND := /usr/bin/find

all_javas := $(OUTPUT_DIR)/all.javas

.PHONY: compile
compile: $(all_javas)
	$(JAVAC) $(JFLAGS) @$<

.INTERMEDIATE: $(all_javas)
$(all_javas):
	$(FIND) $(SOURCE_DIR) -name '*.java' > $@

