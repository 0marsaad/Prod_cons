# Makefile for Producer/Consumer with shared memory + semaphores

# Compiler settings
CXX       := g++
CXXFLAGS  := -Wall -g   # add -O2 or any other flags as needed
TARGETS   := producer consumer

# Default target - build both executables
all: $(TARGETS)

# Build producer
producer: producer.o
	$(CXX) $(CXXFLAGS) -o producer producer.o

# Build consumer
consumer: consumer.o
	$(CXX) $(CXXFLAGS) -o consumer consumer.o

# Object file for producer
producer.o: producer.cpp shared.h
	$(CXX) $(CXXFLAGS) -c producer.cpp

# Object file for consumer
consumer.o: consumer.cpp shared.h
	$(CXX) $(CXXFLAGS) -c consumer.cpp

# Run target (example usage)
# Adjust arguments or commands as needed
run_p: producer
	
	@echo "Running multiple producers..."
	./producer GOLD 1800 5 200 40 &
	./producer SILVER 22 1 300 40 &
	./producer CRUDEOIL 80 3 500 40 &

	@echo "Producers are now running"

run_c: consumer

	@echo "Running multiple consumers..."
	./consumer 40
	echo "Consumers are now running"

# Clean up build artifacts and any leftover executables
clean:
	rm -f *.o $(TARGETS)

.PHONY: all clean run

kill:
	killall producer consumer
	
