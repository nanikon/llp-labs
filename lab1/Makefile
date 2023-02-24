BUILDDIR=build
SRCDIR=src
CC=g++

.PHONY: clean, all, unit_test, stress_test, test

clean:
	rm -rf $(BUILDDIR)
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/*/%.cpp
	$(CC) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/*/*/%.cpp
	$(CC) -c $< -o $@

all: $(BUILDDIR)/file.o $(BUILDDIR)/unit_test.o $(BUILDDIR)/main.o 
	$(CC) -o $(BUILDDIR)/main $^

unit_test: all
	$(BUILDDIR)/main u

stress_test: all
	$(BUILDDIR)/main s

test: unit_test stress_test