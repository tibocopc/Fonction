# Makefile pour le calculateur de dérivées symboliques

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
LDFLAGS = -lm

TARGET = derivative
SRC = derivative.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@echo "=== Test 1: x^2*sin(x) ==="
	@echo "x^2*sin(x)" | ./$(TARGET)
	@echo ""
	@echo "=== Test 2: x^3 ==="
	@echo "x^3" | ./$(TARGET)
	@echo ""
	@echo "=== Test 3: sin(x)*cos(x) ==="
	@echo "sin(x)*cos(x)" | ./$(TARGET)
	@echo ""
	@echo "=== Test 4: exp(x^2) ==="
	@echo "exp(x^2)" | ./$(TARGET)
	@echo ""
	@echo "=== Test 5: ln(x)/x ==="
	@echo "ln(x)/x" | ./$(TARGET)

.PHONY: all clean test
