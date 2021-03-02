TARGET = pa2
OBJECTS = huff.o

ALL: $(TARGET) cleanobj

$(TARGET): $(OBJECTS)
	gcc -o $@ $^

cleanobj:
	rm $(OBJECTS)

