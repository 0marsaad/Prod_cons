# CC =g++
# CFLAGS =-Wall -g
# LDFLAGS =-lrt -lm -lpthread
# COMMODITIES =GOLD SILVER COPPER
# all: producer consumer
# producer: producer.c shared.h
#     gcc -Wall -g -o producer producer.c -lrt -lm -lpthread
# consumer: consumer.c shared.h
#     gcc -Wall -g -o consumer consumer.c -lrt -lm -lpthread
# run_GOLD: producer
#     ./producer GOLD 1800.0 10.0 200 40 &
# run_SILVER: producer
#     ./producer SILVER 100.0 5.0 100 20 &
# run_COPPER: producer
#     ./producer COPPER 50.0 2.5 50 10 &
# run_consumer: consumer
#     ./consumer 40
# run_all: producer consumer run_producers run_consumer
# clean:
#     rm -f producer consumer
